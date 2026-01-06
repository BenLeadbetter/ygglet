#pragma once

#include <boost/uuid/uuid.hpp>

#include <variant>

namespace ygglet::engine {

struct Connection
{
    struct Node
    {
        boost::uuids::uuid id{};
        boost::uuids::uuid port{};
    };
    struct Endpoint
    {
        std::uint32_t index{};
    };
    std::variant<Node, Endpoint> in;
    std::variant<Node, Endpoint> out;
};

} // namespace ygglet::engine
