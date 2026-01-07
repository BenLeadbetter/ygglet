#include <ygglet/engine/node.hpp>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/uuid/generators.hpp>

namespace ygglet::engine {

Node::Node(std::uint32_t inputs, std::uint32_t outputs)
: m_id(boost::uuids::random_generator{}())
, m_inputs(inputs)
, m_outputs(outputs)
{
    boost::uuids::random_generator uuidgen{};
}

Node::~Node() = default;

boost::uuids::uuid Node::id() const
{
    return m_id;
}

std::uint32_t Node::inputs() const
{
    return m_inputs;
}

std::uint32_t Node::outputs() const
{
    return m_outputs;
}

std::span<std::span<float>> Node::buffers()
{
    return m_buffers.buffers;
}

} // namespace ygglet::engine
