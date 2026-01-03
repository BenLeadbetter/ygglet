#include <ygglet/processor/graph.hpp>
#include <ygglet/processor/node.hpp>

namespace ygglet::processor {

Graph::Graph() = default;

Graph::~Graph() = default;

void Graph::compile()
{
}

void Graph::cleanup()
{
}

void Graph::insert(std::unique_ptr<Node>)
{
}

bool Graph::connect(boost::uuids::uuid in, boost::uuids::uuid out)
{
    return false;
}

} // namespace ygglet::processor
