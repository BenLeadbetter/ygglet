#pragma once

#include <ygglet/engine/connection_id.hpp>
#include <ygglet/engine/node_id.hpp>

#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {

struct Connection
{
    using Id = ConnectionId;
    struct Port
    {
        NodeId node{};
        std::uint32_t port{};
    };
    Port in;
    Port out;
    Id id;
};

bool operator==(const Connection::Port& lhs, const Connection::Port& rhs);
bool operator==(const Connection& lhs, const Connection& rhs);

} // namespace ygglet::engine
