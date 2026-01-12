#pragma once

#include <ygglet/engine/node_id.hpp>

#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {

struct Connection
{
    struct Port
    {
        NodeId node{};
        std::uint32_t port{};
    };
    Port in;
    Port out;
};

bool operator==(const Connection::Port& lhs, const Connection::Port& rhs);
bool operator==(const Connection& lhs, const Connection& rhs);

} // namespace ygglet::engine
