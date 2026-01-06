#include <ygglet/engine/node.hpp>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/uuid/generators.hpp>

namespace ygglet::engine {

Node::Node(std::size_t inputs, std::size_t outputs)
: m_id(boost::uuids::random_generator{}())
{
    boost::uuids::random_generator uuidgen{};

    m_inputs.resize(inputs);
    for (auto& input : m_inputs)
    {
        input = uuidgen();
    }

    m_outputs.resize(outputs);
    for (auto& output : m_outputs)
    {
        output = uuidgen();
    }
}

Node::~Node() = default;

boost::uuids::uuid Node::id() const
{
    return m_id;
}

std::span<const boost::uuids::uuid> Node::inputs() const
{
    return m_inputs;
}

std::span<const boost::uuids::uuid> Node::outputs() const
{
    return m_outputs;
}

std::span<std::span<float>> Node::buffers()
{
    return m_buffers.buffers;
}

} // namespace ygglet::engine
