#pragma once

#include <vector>

namespace ygglet::processor {

struct Node;

struct RenderGraph
{
    void process();

private:
    using Nodes = std::vector<Node*>;
    Nodes m_order;
};

} // namespace ygglet::processor
