#include <ygglet/processor/connection.hpp>
#include <ygglet/processor/graph.hpp>
#include <ygglet/processor/graph/connections.hpp>
#include <ygglet/processor/graph/endpoints.hpp>
#include <ygglet/processor/graph/nodes.hpp>
#include <ygglet/processor/node.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/adaptor/map.hpp>

namespace ygglet::processor {

namespace graph::detail {

std::span<const boost::uuids::uuid> Inputs::endpoints(const Node& node)
{
    return node.inputs();
}

std::span<const boost::uuids::uuid> Outputs::endpoints(const Node& node)
{
    return node.inputs();
}

} // namespace graph::detail

Graph::Graph(double sampleRate, std::uint32_t blockSize)
: m_sampleRate(sampleRate)
, m_blockSize(blockSize)
{
}

Graph::~Graph() = default;

void Graph::process(std::span<std::span<float>> inputs, std::span<std::span<float>> outputs)
{
}

void Graph::compile()
{
    auto graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::uuids::uuid,
                                       std::pair<boost::uuids::uuid, boost::uuids::uuid>>{};

    for (const auto& node : nodes())
    {
        boost::add_vertex(node.id(), graph);
    }

    for (const auto& connection : connections())
    {
    }
}

graph::Nodes<Graph> Graph::nodes()
{
    return graph::Nodes<Graph>(*this);
}

graph::Nodes<const Graph> Graph::nodes() const
{
    return graph::Nodes<const Graph>(*this);
}

graph::Connections<Graph> Graph::connections()
{
    return graph::Connections<Graph>(*this);
}

graph::Connections<const Graph> Graph::connections() const
{
    return graph::Connections<const Graph>(*this);
}

graph::Endpoints<const Graph, graph::detail::Inputs> Graph::inputs() const
{
    return graph::Endpoints<const Graph, graph::detail::Inputs>(*this);
}

graph::Endpoints<const Graph, graph::detail::Outputs> Graph::outputs() const
{
    return graph::Endpoints<const Graph, graph::detail::Outputs>(*this);
}

} // namespace ygglet::processor
