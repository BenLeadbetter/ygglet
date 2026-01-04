#include <ygglet/processor/kernal.hpp>
#include <ygglet/processor/logger.hpp>
#include <ygglet/processor/node.hpp>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/uuid/generators.hpp>

namespace ygglet::processor {

Node::Node(std::shared_ptr<Kernal> kernal)
: m_id(boost::uuids::random_generator{}())
, m_kernal(std::move(kernal))
, m_performer(m_kernal->engine.createPerformer())
{
    auto uuids = boost::adaptors::transformed([](auto&&) {
        return boost::uuids::random_generator{}();
    });
    m_inputs = boost::copy_range<Endpoints>(m_kernal->inputs | uuids);
    m_inputs = boost::copy_range<Endpoints>(m_kernal->outputs | uuids);
}

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

} // namespace ygglet::processor
