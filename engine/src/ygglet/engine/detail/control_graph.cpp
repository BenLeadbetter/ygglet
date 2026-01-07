#include <ygglet/engine/detail/control_graph.hpp>

namespace ygglet::engine::detail {

ControlGraph::ControlGraph(std::size_t inputs, std::size_t outputs)
{
    auto i = std::make_unique<Endpoint>(0, inputs);
    auto o = std::make_unique<Endpoint>(outputs, 0);
    input = i.get();
    output = o.get();
    nodes.insert({input->id(), std::move(i)});
    nodes.insert({output->id(), std::move(o)});
}

} // namespace ygglet::engine::detail
