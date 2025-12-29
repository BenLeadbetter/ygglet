#include <ygglet/audio/logger.h>
#include <ygglet/audio/module.h>
#include <ygglet/audio/processor.h>

#include <cmajor/API/cmaj_DiagnosticMessages.h>
#include <cmajor/API/cmaj_Endpoints.h>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>

#include <algorithm>
#include <cstring>

namespace ygglet::audio {

namespace {

void logDiagnostics(const auto& messages)
{
    for (const auto& message : messages.messages)
    {
        using Type = cmaj::DiagnosticMessage::Type;
        switch (message.type)
        {
            case Type::error:
            case Type::internalCompilerError:
                Logger::error("[{}] {}", message.getCategory(), message.getFullDescription());
                break;
            case Type::warning:
                Logger::warn("[{}] {}", message.getCategory(), message.getFullDescription(),
                             message.getAnnotatedSourceLine());
                break;
            case Type::note:
                Logger::info("[{}] {}", message.getCategory(), message.getFullDescription(),
                             message.getAnnotatedSourceLine());
                break;
            default:
                BOOST_ASSERT_MSG(false, "Unhandled message type");
                break;
        }
        if (auto location = message.getAnnotatedSourceLine(); !location.empty())
        {
            Logger::info("{}", location);
        }
    }
};

} // namespace

tl::expected<Processor, Processor::MakeError> Processor::make()
{
    auto processor = Processor{};
    processor.m_module = Module::aquire();
    processor.m_engine = cmaj::Engine::create();

    if (!processor.m_engine)
    {
        return tl::unexpected{MakeError::FailedToInitializeCMajorEngine};
    }

    return processor;
}

Processor::Processor() = default;

Processor::~Processor() = default;

Processor::Processor(Processor&&) noexcept = default;

Processor& Processor::operator=(Processor&&) noexcept = default;

tl::expected<std::monostate, Processor::LoadError> Processor::load(std::string_view source, std::string_view filename)
{
    BOOST_ASSERT_MSG(m_engine, "Processor in invalid state. The engine should never be null.");

    cmaj::Program program;

    {
        cmaj::DiagnosticMessageList messages;
        const auto result = program.parse(messages, filename.data(), source.data());
        logDiagnostics(messages);
        if (!result)
        {
            return tl::unexpected{std::move(messages)};
        }
    }

    {
        cmaj::BuildSettings buildSettings;
        buildSettings.setFrequency(m_sampleRate);
        buildSettings.setMaxBlockSize(m_blockSize);
        m_engine.setBuildSettings(buildSettings);
    }

    {
        cmaj::DiagnosticMessageList messages;
        const auto result = m_engine.load(messages, program, nullptr, nullptr);
        logDiagnostics(messages);
        if (!result)
        {
            return tl::unexpected{std::move(messages)};
        }
    }

    // Cache endpoint handles BEFORE linking (as per documentation)
    cacheEndpoints();

    {
        cmaj::DiagnosticMessageList messages;
        const auto result = m_engine.link(messages);
        logDiagnostics(messages);
        if (!result)
        {
            return tl::unexpected{std::move(messages)};
        }
    }

    m_performer = m_engine.createPerformer();
    if (!m_performer)
    {
        return tl::unexpected{FailedToCreatePerformer{}};
    }

    return {};
}

void Processor::setSampleRate(double sampleRate)
{
    m_sampleRate = sampleRate;
    // Note: Changing sample rate requires reloading the program
}

void Processor::setBlockSize(uint32_t blockSize)
{
    m_blockSize = blockSize;
    if (m_performer)
    {
        auto result = m_performer.setBlockSize(blockSize);
        if (result != cmaj::Result::Ok)
        {
            Logger::error("Failed to set performer block size. Result: {}", static_cast<int>(result));
        }
    }
}

void Processor::process(std::span<std::span<float>> inputs, std::span<std::span<float>> outputs)
{
    if (!m_performer)
    {
        for (auto output : outputs)
        {
            std::memset(output.data(), 0, output.size() * sizeof(float));
        }
        return;
    }

    for (size_t i = 0; i < std::min(inputs.size(), m_inputEndpoints.size()); ++i)
    {
        auto result =
            m_performer.setInputFrames(m_inputEndpoints[i], inputs[i].data(), static_cast<uint32_t>(inputs[i].size()));
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

    for (size_t i = 0; i < std::min(outputs.size(), m_outputEndpoints.size()); ++i)
    {
        auto result = m_performer.copyOutputFrames(m_outputEndpoints[i], outputs[i].data(),
                                                   static_cast<uint32_t>(outputs[i].size()));
        if (result != cmaj::Result::Ok)
        {
            Logger::error("copyOutputFrames failed with result: {}", static_cast<int>(result));
        }
    }
}

void Processor::reset()
{
    if (m_performer)
    {
        m_performer.reset();
    }
}

size_t Processor::getNumInputs() const
{
    return m_inputEndpoints.size();
}

size_t Processor::getNumOutputs() const
{
    return m_outputEndpoints.size();
}

void Processor::cacheEndpoints()
{
    m_inputEndpoints.clear();
    m_outputEndpoints.clear();

    auto inputs = m_engine.getInputEndpoints();
    for (auto& endpoint : inputs.endpoints)
    {
        if (endpoint.isStream() && endpoint.getNumAudioChannels() > 0)
        {
            auto idString = endpoint.endpointID.toString();
            auto handle = m_engine.getEndpointHandle(idString.c_str());
            m_inputEndpoints.push_back(handle);
        }
    }

    auto outputs = m_engine.getOutputEndpoints();
    for (auto& endpoint : outputs.endpoints)
    {
        if (endpoint.isStream() && endpoint.getNumAudioChannels() > 0)
        {
            auto idString = endpoint.endpointID.toString();
            auto handle = m_engine.getEndpointHandle(idString.c_str());
            m_outputEndpoints.push_back(handle);
        }
    }
}

} // namespace ygglet::audio
