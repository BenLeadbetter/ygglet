#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/connections.hpp>
#include <ygglet/engine/endpoints.hpp>
#include <ygglet/engine/engine.hpp>
#include <ygglet/engine/node.hpp>
#include <ygglet/engine/nodes.hpp>
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
    auto& graph = m_graph.render.current();

    if (inputs.size() != graph.inputs.size() || outputs.size() != graph.outputs.size())
    {
        return;
    }

    for (auto i = 0; i != inputs.size(); ++i)
    {
        if (graph.inputs[i])
        {
            *graph.inputs[i] = inputs[i];
        }
    }

    for (auto i = 0; i != outputs.size(); ++i)
    {
        if (graph.inputs[i])
        {
            *graph.outputs[i] = outputs[i];
        }
    }

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

    for (const auto& node : nodes())
    {
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

    auto& renderGraph = m_graph.render.inactive();

    // TODO: is this safe ?
    // How do I know the audio thread isn't still processing this one?
    renderGraph.nodes.clear();

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
                           auto& node = *m_graph.control.nodes.find(out.id)->second;
                           const auto index =
                               std::distance(node.inputs().begin(), std::ranges::find(node.outputs(), out.port));
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
        auto& node = *m_graph.control.nodes.find(in.id)->second;
        const auto index = std::distance(node.inputs().begin(), std::ranges::find(node.outputs(), in.port));
        BOOST_ASSERT(index != node.inputs().size());
        renderGraph.outputs[out.index] = &renderNodes.find(node.id())->second->inputs[index];
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

    m_graph.render.publish();
}

Nodes<Engine> Engine::nodes()
{
    return Nodes<Engine>(*this);
}

Nodes<const Engine> Engine::nodes() const
{
    return Nodes<const Engine>(*this);
}

Connections<Engine> Engine::connections()
{
    return Connections<Engine>(*this);
}

Connections<const Engine> Engine::connections() const
{
    return Connections<const Engine>(*this);
}

Inputs<const Engine> Engine::inputs() const
{
    return Inputs<const Engine>(*this);
}

Outputs<const Engine> Engine::outputs() const
{
    return Outputs<const Engine>(*this);
}

std::vector<std::vector<float>>& Engine::buffers(Node& node)
{
    return node.m_buffers.storage;
}

const std::vector<std::vector<float>>& Engine::buffers(const Node& node)
{
    return node.m_buffers.storage;
}

} // namespace ygglet::engine
