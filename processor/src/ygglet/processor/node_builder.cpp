#include <ygglet/processor/node.hpp>
#include <ygglet/processor/node_builder.hpp>

#include <boost/assert.hpp>

namespace ygglet::processor {

tl::expected<Node, cmaj::DiagnosticMessageList> NodeBuilder::build()
{
    BOOST_ASSERT(m_source);
    auto node = Node{};
    BOOST_ASSERT(false); // TODO:
    return node;
}

NodeBuilder& NodeBuilder::source(std::string source)
{
    m_source = std::move(source);
    return *this;
}

NodeBuilder& NodeBuilder::filename(std::string filename)
{
    m_filename = std::move(filename);
    return *this;
}

NodeBuilder& NodeBuilder::sampleRate(double rate)
{
    m_sampleRate = rate;
    return *this;
}

NodeBuilder& NodeBuilder::blockSize(std::uint32_t size)
{
    m_blockSize = size;
    return *this;
}

NodeBuilder& NodeBuilder::frames(std::uint32_t frames)
{
    m_frames = frames;
    return *this;
}

} // namespace ygglet::processor
