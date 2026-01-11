#pragma once

#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/detail/endpoint.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {
struct Node;
}

namespace ygglet::engine::detail {

struct ControlGraph
{
    ControlGraph(std::size_t inputs, std::size_t outputs);

    using Graph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::uuids::uuid, Connection>;
    using Connections = boost::container::flat_map<boost::uuids::uuid, Connection>;
    using Descriptors = boost::container::flat_map<boost::uuids::uuid, Graph::vertex_descriptor>;
    using Nodes = boost::container::flat_map<boost::uuids::uuid, std::unique_ptr<Node>>;

    Connections connections;
    Descriptors descriptors;
    Endpoint* input{};
    Endpoint* output{};
    Graph graph;
    Nodes nodes;
};

} // namespace ygglet::engine::detail
