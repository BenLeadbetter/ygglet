#include <ygglet/processor/kernal.hpp>
#include <ygglet/processor/logger.hpp>
#include <ygglet/processor/node.hpp>

#include <boost/assert.hpp>

namespace ygglet::processor {

Node::Node(std::shared_ptr<Kernal> kernal)
: engine::Node(kernal->inputs.size(), kernal->outputs.size())
, m_kernal(std::move(kernal))
, m_performer(m_kernal->engine.createPerformer())
{
}

void Node::process(std::span<std::span<const float>> inputs)
{
    BOOST_ASSERT_MSG(m_performer, "Expected a valid performer instance");

    if (inputs.size() != m_kernal->inputs.size())
    {
        Logger::error("Incorrect number of input buffers");
        for (auto output : buffers())
        {
            std::ranges::fill(output, 0.0f);
        }
        return;
    }

    for (size_t i = 0; i < inputs.size(); ++i)
    {
        auto result =
            m_performer.setInputFrames(m_kernal->inputs[i], inputs[i].data(), static_cast<uint32_t>(inputs[i].size()));
        if (result != cmaj::Result::Ok)
        {
            Logger::error("setInputFrames failed with result: {}", static_cast<int>(result));
        }
    }

    auto advanceResult = m_performer.advance();
    if (advanceResult != cmaj::Result::Ok)
    {
        Logger::error("advance() failed with result: {}", static_cast<int>(advanceResult));
    }

    BOOST_ASSERT_MSG(buffers().size() == m_kernal->outputs.size(), "Incorrect number of output buffers");

    for (size_t i = 0; i < buffers().size(); ++i)
    {
        auto result = m_performer.copyOutputFrames(m_kernal->outputs[i], buffers()[i].data(),
                                                   static_cast<uint32_t>(buffers()[i].size()));
        if (result != cmaj::Result::Ok)
        {
            Logger::error("copyOutputFrames failed with result: {}", static_cast<int>(result));
        }
    }
}

} // namespace ygglet::processor
