#pragma once

#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {

struct Connection
{
    struct
    {
        boost::uuids::uuid node{};
        std::uint32_t port{};
    } in;
    struct
    {
        boost::uuids::uuid node{};
        std::uint32_t port{};
    } out;
};

} // namespace ygglet::engine
