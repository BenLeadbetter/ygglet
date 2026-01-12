#pragma once

#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/node_id.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {
struct Node;
} // namespace ygglet::engine

namespace ygglet::engine::detail {

struct Endpoint;

struct ControlGraph
{
    ControlGraph(std::size_t inputs, std::size_t outputs);

    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, NodeId, Connection>;
    using Connections = boost::container::flat_map<Connection::Id, Connection>;
    using Descriptors = boost::container::flat_map<NodeId, Graph::vertex_descriptor>;
    using Nodes = boost::container::flat_map<NodeId, std::unique_ptr<Node>>;

    Connections connections;
    Descriptors descriptors;
    Endpoint* input{};
    Endpoint* output{};
    Graph graph;
    Nodes nodes;
};

} // namespace ygglet::engine::detail
