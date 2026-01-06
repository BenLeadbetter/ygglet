#pragma once

namespace ygglet::engine {

template <class... Ts> struct Visitor : Ts...
{
    using Ts::operator()...;
};

} // namespace ygglet::engine
