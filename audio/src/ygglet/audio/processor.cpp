#include <ygglet/audio/processor.h>

#include <cmajor/COM/cmaj_Library.h>
#include <cmajor/API/cmaj_DiagnosticMessages.h>
#include <cmajor/API/cmaj_Endpoints.h>

#include <algorithm>
#include <cstring>

namespace ygglet::audio {

namespace {
    struct LibraryInitializer {
        LibraryInitializer() {
            if constexpr (cmaj::Library::isUsingDLL) {
                // Try to find the DLL in the library search paths
                // The name is platform-specific (libCmajPerformer.dylib on macOS, etc.)
                std::string dllName = cmaj::Library::getDLLName();

                // On macOS with conan, the library should be in the RPATH
                // We can just pass the name and let the system find it
                if (!cmaj::Library::initialise(dllName)) {
                    // If that doesn't work, it might need an absolute path
                    // which would be set up by CMake
                }
            }
        }
        ~LibraryInitializer() {
            cmaj::Library::shutdown();
        }
    };

    static LibraryInitializer libraryInit;
}

tl::expected<Processor, Processor::MakeError> Processor::make() {
    auto processor = Processor{};
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
    assert(m_engine);

    cmaj::Program program;
    cmaj::DiagnosticMessageList messages;

    if (!program.parse(messages, filename.data(), source.data()))
    {
        return tl::unexpected{std::move(messages)};
    }

    cmaj::BuildSettings buildSettings;
    buildSettings.setFrequency(m_sampleRate);
    buildSettings.setMaxBlockSize(m_blockSize);
    m_engine.setBuildSettings(buildSettings);

    messages = {};
    if (!m_engine.load(messages, program, nullptr, nullptr))
    {
        return tl::unexpected{std::move(messages)};
    }

    messages = {};
    if (!m_engine.link(messages))
    {
        return tl::unexpected{std::move(messages)};
    }

    m_performer = m_engine.createPerformer();
    if (!m_performer)
    {
        return tl::unexpected{FailedToCreatePerformer{}};
    }

    cacheEndpoints();

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
        m_performer.setBlockSize(blockSize);
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
        m_performer.setInputFrames(m_inputEndpoints[i], inputs[i].data(), static_cast<uint32_t>(inputs[i].size()));
    }

    m_performer.advance();

    for (size_t i = 0; i < std::min(outputs.size(), m_outputEndpoints.size()); ++i)
    {
        m_performer.copyOutputFrames(m_outputEndpoints[i], outputs[i].data(), static_cast<uint32_t>(outputs[i].size()));
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

    if (!m_engine.isLinked())
    {
        return;
    }

    // Get input endpoints from the engine
    auto inputs = m_engine.getInputEndpoints();
    for (auto& endpoint : inputs.endpoints)
    {
        if (endpoint.isStream() && endpoint.getNumAudioChannels() > 0)
        {
            m_inputEndpoints.push_back(m_engine.getEndpointHandle(endpoint.endpointID));
        }
    }

    // Get output endpoints from the engine
    auto outputs = m_engine.getOutputEndpoints();
    for (auto& endpoint : outputs.endpoints)
    {
        if (endpoint.isStream() && endpoint.getNumAudioChannels() > 0)
        {
            m_outputEndpoints.push_back(m_engine.getEndpointHandle(endpoint.endpointID));
        }
    }
}

} // namespace ygglet::audio
