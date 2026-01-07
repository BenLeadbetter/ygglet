#include <ygglet/engine/detail/render_graph.hpp>

namespace ygglet::engine::detail {

RenderGraph::Graph& RenderGraph::current()
{
    return buffers[index.load(std::memory_order_relaxed)];
}

RenderGraph::Graph& RenderGraph::inactive()
{
    return buffers[1 - index.load(std::memory_order_relaxed)];
}

void RenderGraph::publish()
{
    index.store(1 - index.load(std::memory_order_relaxed), std::memory_order_release);
}

} // namespace ygglet::engine::detail
