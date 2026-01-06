#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/detail/connections.hpp>
#include <ygglet/engine/detail/endpoints.hpp>
#include <ygglet/engine/detail/nodes.hpp>
#include <ygglet/engine/engine.hpp>
#include <ygglet/engine/node.hpp>
#include <ygglet/engine/visitor.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace ygglet::engine {

namespace detail {

RenderGraph::Graph& RenderGraph::current()
{
    return buffers[index.load(std::memory_order_relaxed)];
}

RenderGraph::Graph& RenderGraph::inactive()
{
    return buffers[1 - index.load(std::memory_order_relaxed)];
}

void RenderGraph::publish()
{
    index.store(1 - index.load(std::memory_order_relaxed), std::memory_order_release);
}

} // namespace detail

Engine::Engine(double sampleRate, std::uint32_t blockSize)
: m_sampleRate(sampleRate)
, m_blockSize(blockSize)
{
}

Engine::~Engine() = default;

void Engine::process(std::span<std::span<const float>> inputs, std::span<std::span<float>> outputs)
{
    auto& graph = m_render.current();

    if (inputs.size() != graph.inputs.size() || outputs.size() != graph.outputs.size())
    {
        return;
    }

    // assign external input buffers
    for (auto i = 0; i != inputs.size(); ++i)
    {
        if (graph.inputs[i])
        {
            *graph.inputs[i] = inputs[i];
        }
    }

    // assign external output buffers
    for (auto i = 0; i != outputs.size(); ++i)
    {
        if (graph.outputs[i])
        {
            *graph.outputs[i] = outputs[i];
        }
    }

    // process nodes
    for (auto i = 0; i != graph.nodes.size(); ++i)
    {
        graph.nodes[i].node->process(graph.nodes[i].inputs);
    }
}

void Engine::compile()
{
    using IntermediateGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::uuids::uuid,
                                                    std::pair<Connection::Node, Connection::Node>>;
    auto graph = IntermediateGraph{};
    boost::container::flat_map<boost::uuids::uuid, IntermediateGraph::vertex_descriptor> descriptors;

    for (auto& node : nodes())
    {
        if (node.buffers().empty() && !node.outputs().empty())
        {
            // allocate buffers
            node.m_buffers.storage.resize(node.outputs().size());
            for (auto& buffer : node.m_buffers.storage)
            {
                buffer = std::vector<float>(m_blockSize, 0.0f);
                node.m_buffers.buffers.push_back(buffer);
            }
        }

        auto descriptor = boost::add_vertex(node.id(), graph);
        descriptors.insert({node.id(), descriptor});
    }

    for (const auto& [in, out] : connections() | boost::adaptors::filtered([](const auto& connection) {
                                     return std::holds_alternative<Connection::Node>(connection.in) &&
                                            std::holds_alternative<Connection::Node>(connection.out);
                                 }) | boost::adaptors::transformed([](const auto& connection) {
                                     return std::make_pair(std::get<Connection::Node>(connection.in),
                                                           std::get<Connection::Node>(connection.out));
                                 }))
    {
        boost::add_edge(descriptors.find(out.id)->second, descriptors.find(out.id)->second, {in, out}, graph);
    }

    std::vector<IntermediateGraph::vertex_descriptor> order;
    boost::container::flat_map<boost::uuids::uuid, detail::RenderGraph::Node*> renderNodes;
    boost::topological_sort(graph, std::back_inserter(order));

    auto& renderGraph = m_render.inactive();

    // TODO: is this safe ?
    // How do I know the audio thread isn't still processing this one?
    renderGraph.nodes.clear();
    renderGraph.inputs.resize(m_control.inputs.size());
    renderGraph.outputs.resize(m_control.outputs.size());

    renderGraph.silence = std::vector<float>(std::size_t{m_blockSize}, 0.0f);

    for (auto descriptor : order)
    {
        auto& node = *nodes().find(graph[descriptor]);
        auto renderNode = detail::RenderGraph::Node{&node};
        renderNode.inputs.resize(node.inputs().size());

        // connect inter-node buffers

        for (const auto& edge : boost::make_iterator_range(boost::in_edges(descriptor, graph)) |
                                    boost::adaptors::transformed([&graph](auto descriptor) {
                                        return graph[descriptor];
                                    }))
        {
            const auto buffer = [&]() -> std::span<const float> {
                const auto& from = *nodes().find(edge.second.id);
                const auto index =
                    std::distance(from.outputs().begin(), std::ranges::find(from.outputs(), edge.second.port));
                BOOST_ASSERT(index != from.outputs().size());
                return node.m_buffers.buffers[index];
            }();
            const auto index =
                std::distance(node.outputs().begin(), std::ranges::find(node.outputs(), edge.first.port));
            BOOST_ASSERT(index != node.outputs().size());
            renderNode.inputs[index] = buffer;
        }

        renderGraph.nodes.push_back(std::move(renderNode));
        renderNodes.insert({node.id(), &renderGraph.nodes.back()});
    }

    // connect external input buffers

    for (const auto& connection : connections() | boost::adaptors::filtered([](const auto& connection) {
                                      return std::holds_alternative<Connection::Endpoint>(connection.in);
                                  }))
    {
        const auto in = std::get<Connection::Endpoint>(connection.in);
        std::visit(Visitor{
                       [&](const Connection::Node& out) {
                           auto& node = *m_control.nodes.find(out.id)->second;
                           const auto index =
                               std::distance(node.inputs().begin(), std::ranges::find(node.inputs(), out.port));
                           BOOST_ASSERT(index != node.inputs().size());
                           renderGraph.inputs[in.index] = &renderNodes.find(node.id())->second->inputs[index];
                       },
                       [&](const Connection::Endpoint& out) {
                           // TODO: audio was passed through
                           BOOST_ASSERT(false);
                       },
                   },
                   connection.out);
    }

    // connect external output buffers

    for (const auto& connection : connections() | boost::adaptors::filtered([](const auto& connection) {
                                      return std::holds_alternative<Connection::Endpoint>(connection.out) &&
                                             !std::holds_alternative<Connection::Endpoint>(connection.in);
                                  }))
    {
        const auto out = std::get<Connection::Endpoint>(connection.out);
        const auto in = std::get<Connection::Node>(connection.in);
        auto& node = *m_control.nodes.find(in.id)->second;
        const auto index = std::distance(node.outputs().begin(), std::ranges::find(node.outputs(), in.port));
        BOOST_ASSERT(index != node.outputs().size());
        renderGraph.outputs[out.index] = &nodes().find(node.id())->buffers()[index];
    }

    // silence any inputs not connected

    for (auto& node : renderGraph.nodes)
    {
        for (auto& input : node.inputs | boost::adaptors::filtered([](auto input) {
                               return input.empty();
                           }))
        {
            input = renderGraph.silence;
        }
    }

    m_render.publish();
}

detail::Nodes<Engine> Engine::nodes()
{
    return detail::Nodes<Engine>(*this);
}

detail::Nodes<const Engine> Engine::nodes() const
{
    return detail::Nodes<const Engine>(*this);
}

detail::Connections<Engine> Engine::connections()
{
    return detail::Connections<Engine>(*this);
}

detail::Connections<const Engine> Engine::connections() const
{
    return detail::Connections<const Engine>(*this);
}

detail::Inputs<const Engine> Engine::inputs() const
{
    return detail::Inputs<const Engine>(*this);
}

detail::Inputs<Engine> Engine::inputs()
{
    return detail::Inputs<Engine>(*this);
}

detail::Outputs<const Engine> Engine::outputs() const
{
    return detail::Outputs<const Engine>(*this);
}

detail::Outputs<Engine> Engine::outputs()
{
    return detail::Outputs<Engine>(*this);
}

} // namespace ygglet::engine
