#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/detail/connections.hpp>
#include <ygglet/engine/detail/nodes.hpp>
#include <ygglet/engine/engine.hpp>
#include <ygglet/engine/node.hpp>
#include <ygglet/engine/visitor.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace ygglet::engine {

Engine::Engine(std::size_t inputs, std::size_t outputs, double sampleRate, std::uint32_t blockSize)
: m_sampleRate(sampleRate)
, m_blockSize(blockSize)
, m_control(inputs, outputs)
{
}

Engine::~Engine() = default;

void Engine::process(std::span<std::span<const float>> inputs, std::span<std::span<float>> outputs)
{
    auto& graph = m_render.current();

    if (graph.nodes.empty())
    {
        return;
    }

    BOOST_ASSERT(inputs.size() == graph.inputs.size());
    BOOST_ASSERT(outputs.size() == graph.outputs.size());

    // assign external input buffers
    for (auto i = 0; i != inputs.size(); ++i)
    {
        std::visit(Visitor{
                       [&](std::span<const float>* buffer) {
                           if (buffer)
                           {
                               *buffer = inputs[i];
                           }
                       },
                       [&](std::uint32_t passthrough) {
                           std::memcpy(outputs[passthrough].data(), inputs[i].data(), inputs[i].size() * sizeof(float));
                       },
                   },
                   graph.inputs[i]);
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

    graph.epoch++;
}

namespace {

template <typename T> struct ActiveVisitor : boost::default_dfs_visitor
{
    template <typename V, typename G> void discover_vertex(V u, const G& graph) { active.insert(graph[u]); }
    T& active;
};

} // namespace

void Engine::compile()
{
    BOOST_ASSERT(ready());

    using IntermediateGraph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::uuids::uuid, Connection>;
    using Descriptors = boost::container::flat_map<boost::uuids::uuid, IntermediateGraph::vertex_descriptor>;

    Descriptors descriptors;
    const auto graph = [&]() {
        IntermediateGraph graph{};

        for (auto& node : nodes())
        {
            auto descriptor = boost::add_vertex(node.id(), graph);
            descriptors.insert({node.id(), descriptor});
        }

        for (const auto& connection : connections())
        {
            boost::add_edge(descriptors.find(connection.out.node)->second, descriptors.find(connection.in.node)->second,
                            connection, graph);
        }

        return graph;
    }();

    // allocate buffers

    for (auto& node : nodes() | boost::adaptors::filtered([&](auto& node) {
                          return node.buffers().empty() && node.outputs() != 0 && &node != m_control.input &&
                                 &node != m_control.output;
                      }))
    {
        node.m_buffers.storage.resize(node.outputs());
        for (auto& buffer : node.m_buffers.storage)
        {
            buffer = std::vector<float>(m_blockSize, 0.0f);
            node.m_buffers.buffers.push_back(buffer);
        }
    }

    std::vector<IntermediateGraph::vertex_descriptor> order;
    boost::topological_sort(graph, std::back_inserter(order));

    boost::container::flat_set<boost::uuids::uuid> active;
    boost::depth_first_visit(
        boost::make_reverse_graph(graph), descriptors[m_control.output->id()], ActiveVisitor{.active = active},
        boost::make_vector_property_map<boost::default_color_type>(boost::get(boost::vertex_index, graph)));


    // reset the inactive render graph
    auto& renderGraph = m_render.inactive();
    renderGraph.nodes.clear();
    renderGraph.inputs.resize(m_control.input->outputs());
    renderGraph.outputs.resize(m_control.output->inputs());
    renderGraph.silence = std::vector<float>(std::size_t{m_blockSize}, 0.0f);
    renderGraph.epoch = 0;

    using RenderNodes = boost::container::flat_map<boost::uuids::uuid, detail::RenderGraph::Node*>;
    RenderNodes renderNodes;

    // build render graph

    for (auto& node : order | boost::adaptors::reversed | boost::adaptors::transformed([&](auto descriptor) -> Node& {
                          return *nodes().find(graph[descriptor]);
                      }) | boost::adaptors::filtered([&](auto& node) {
                          return active.find(node.id()) != active.end();
                      }))
    {
        auto renderNode = detail::RenderGraph::Node{&node};
        renderNode.inputs.resize(node.inputs());

        // connect buffers

        for (const auto& edge : boost::make_iterator_range(boost::in_edges(descriptors[node.id()], graph)) |
                                    boost::adaptors::transformed([&graph](auto descriptor) {
                                        return graph[descriptor];
                                    }))
        {
            auto& from = *nodes().find(edge.out.node);
            if (renderNode.node == m_control.output && &from == m_control.input)
            {
                // passthrough
                renderGraph.inputs[edge.out.port] = detail::RenderGraph::Graph::Passthrough{edge.out.port};
            }
            else if (renderNode.node == m_control.output)
            {
                // external output
                renderGraph.outputs[edge.in.port] = &from.m_buffers.buffers[edge.out.port];
            }
            else if (&from == m_control.input)
            {
                // external input
                renderGraph.inputs[edge.out.port] = &renderNode.inputs[edge.out.port];
            }
            else
            {
                // internal
                renderNode.inputs[edge.in.port] = from.m_buffers.buffers[edge.out.port];
            }
        }

        renderGraph.nodes.push_back(std::move(renderNode));
        renderNodes.insert({node.id(), &renderGraph.nodes.back()});
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

bool Engine::ready()
{
    auto& current = m_render.current();
    return current.nodes.empty() || current.epoch > 0;
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

} // namespace ygglet::engine
