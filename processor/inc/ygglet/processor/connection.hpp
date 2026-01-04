#pragma once

#include <boost/uuid/uuid.hpp>

namespace ygglet::processor {

struct Connection
{
    struct
    {
        boost::uuids::uuid node{};
        boost::uuids::uuid port{};
    } in;
    struct
    {
        boost::uuids::uuid node{};
        boost::uuids::uuid port{};
    } out;
};

} // namespace ygglet::processor
