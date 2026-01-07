#pragma once

#include <ygglet/engine/node.hpp>

namespace ygglet::engine::detail {

struct Endpoint : Node
{
    using Node::Node;
    void process(std::span<std::span<const float>>) override {}
};

} // namespace ygglet::engine::detail
