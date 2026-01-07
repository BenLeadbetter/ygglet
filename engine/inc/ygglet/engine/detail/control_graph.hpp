#pragma once

#include <ygglet/engine/connection.hpp>
#include <ygglet/engine/detail/endpoint.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {
struct Node;
}

namespace ygglet::engine::detail {

struct ControlGraph
{
    ControlGraph(std::size_t inputs, std::size_t outputs);
    using Nodes = boost::container::flat_map<boost::uuids::uuid, std::unique_ptr<Node>>;
    using Connections = boost::container::flat_map<boost::uuids::uuid, Connection>;
    Endpoint* input{};
    Endpoint* output{};
    Nodes nodes;
    Connections connections;
};

} // namespace ygglet::engine::detail
