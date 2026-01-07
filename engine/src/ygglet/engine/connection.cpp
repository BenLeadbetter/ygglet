#include <ygglet/engine/connection.hpp>

namespace ygglet::engine {

bool operator==(const Connection::Port& lhs, const Connection::Port& rhs)
{
    return lhs.port == rhs.port && lhs.node == rhs.node;
}

bool operator==(const Connection& lhs, const Connection& rhs)
{
    return lhs.in == rhs.in && lhs.out == rhs.out;
}

} // namespace ygglet::engine
