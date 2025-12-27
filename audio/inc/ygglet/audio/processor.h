#pragma once

#include <cmajor/API/cmaj_Engine.h>
#include <cmajor/API/cmaj_Performer.h>
#include <cmajor/API/cmaj_Program.h>

#include <tl/expected.hpp>

#include <span>
#include <string_view>
#include <variant>
#include <vector>

namespace ygglet::audio {

class Processor
{
public:
    ~Processor();

    Processor(const Processor&) = delete;
    Processor& operator=(const Processor&) = delete;

    Processor(Processor&&) noexcept;
    Processor& operator=(Processor&&) noexcept;

    enum class MakeError
    {
        FailedToInitializeCMajorEngine,
    };

    static tl::expected<Processor, MakeError> make();

    struct FailedToCreatePerformer {};
    using LoadError = std::variant<cmaj::DiagnosticMessageList, FailedToCreatePerformer>;
    tl::expected<std::monostate, LoadError> load(std::string_view source, std::string_view filename = {});

    /// Check if a program is loaded and ready to process
    bool isLoaded() const { return m_engine.isLinked(); }

    /// Set the sample rate
    void setSampleRate(double sampleRate);

    /// Get the current sample rate
    double getSampleRate() const { return m_sampleRate; }

    /// Set the block size for processing
    void setBlockSize(uint32_t blockSize);

    /// Get the current block size
    uint32_t getBlockSize() const { return m_blockSize; }

    void process(std::span<std::span<float>> inputs, std::span<std::span<float>> outputs);

    /// Reset the processor state
    void reset();

    /// Get the number of input endpoints
    size_t getNumInputs() const;

    /// Get the number of output endpoints
    size_t getNumOutputs() const;

private:
    Processor();
    void cacheEndpoints();

    cmaj::Engine m_engine;
    cmaj::Performer m_performer;
    double m_sampleRate = 44100.0;
    std::vector<cmaj::EndpointHandle> m_inputEndpoints;
    std::vector<cmaj::EndpointHandle> m_outputEndpoints;
    uint32_t m_blockSize = 512;
};

} // namespace ygglet::audio
