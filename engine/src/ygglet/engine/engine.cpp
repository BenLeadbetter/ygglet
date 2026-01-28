#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/detail/connections.hpp>
#include <ygglet/engine/detail/endpoint.hpp>
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
    for (auto i = std::size_t{}; i != inputs.size(); ++i)
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
    for (auto i = std::size_t{}; i != outputs.size(); ++i)
    {
        if (graph.outputs[i])
        {
            *graph.outputs[i] = outputs[i];
        }
    }

    // process nodes
    for (auto i = std::size_t{}; i != graph.nodes.size(); ++i)
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

    std::vector<detail::ControlGraph::Graph::vertex_descriptor> order;
    boost::topological_sort(m_control.graph, std::back_inserter(order));

    boost::container::flat_set<Node::Id> active;
    boost::depth_first_visit(
        boost::make_reverse_graph(m_control.graph), m_control.descriptors[m_control.output->id()],
        ActiveVisitor{.active = active},
        boost::make_vector_property_map<boost::default_color_type>(boost::get(boost::vertex_index, m_control.graph)));


    // reset the inactive render graph

    auto& graph = m_render.inactive();
    graph.nodes.clear();
    graph.inputs.resize(m_control.input->outputs());
    graph.outputs.resize(m_control.output->inputs());
    graph.silence = std::vector<float>(std::size_t{m_blockSize}, 0.0f);
    graph.epoch = 0;

    // build render graph

    for (auto& node : order | boost::adaptors::reversed | boost::adaptors::transformed([&](auto descriptor) -> Node& {
                          return *nodes().find(m_control.graph[descriptor]);
                      }) | boost::adaptors::filtered([&](auto& node) {
                          return active.find(node.id()) != active.end();
                      }))
    {
        auto render = detail::RenderGraph::Node{
            .node = &node,
            .inputs = {},
        };
        render.inputs.resize(node.inputs());

        // connect buffers

        for (const auto& edge :
             boost::make_iterator_range(boost::in_edges(m_control.descriptors[node.id()], m_control.graph)) |
                 boost::adaptors::transformed([this](auto descriptor) {
                     return m_control.graph[descriptor];
                 }))
        {
            auto& from = *nodes().find(edge.out.node);
            if (render.node == m_control.output && &from == m_control.input)
            {
                // passthrough
                graph.inputs[edge.out.port] = edge.out.port;
            }
            else if (render.node == m_control.output)
            {
                // external output
                graph.outputs[edge.in.port] = &from.m_buffers.buffers[edge.out.port];
            }
            else if (&from == m_control.input)
            {
                // external input
                graph.inputs[edge.out.port] = &render.inputs[edge.out.port];
            }
            else
            {
                // internal
                render.inputs[edge.in.port] = from.m_buffers.buffers[edge.out.port];
            }
        }

        graph.nodes.push_back(std::move(render));
    }

    // silence any inputs not connected

    for (auto& node : graph.nodes)
    {
        for (auto& input : node.inputs | boost::adaptors::filtered([](auto input) {
                               return input.empty();
                           }))
        {
            input = graph.silence;
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
