#pragma once

#include <ygglet/engine/connection.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {
struct Node;
}

namespace ygglet::engine::detail {

struct ControlGraph
{
    using Endpoints = std::vector<boost::uuids::uuid>;
    using Nodes = boost::container::flat_map<boost::uuids::uuid, std::unique_ptr<Node>>;
    using Connections = boost::container::flat_map<boost::uuids::uuid, Connection>;
    Endpoints inputs;
    Endpoints outputs;
    Nodes nodes;
    Connections connections;
};

} // namespace ygglet::engine::detail
