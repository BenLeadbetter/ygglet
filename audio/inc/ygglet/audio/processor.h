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

    struct FailedToCreatePerformer
    {
    };
    using LoadError = std::variant<cmaj::DiagnosticMessageList, FailedToCreatePerformer>;
    tl::expected<std::monostate, LoadError> load(std::string_view source, std::string_view filename = "");

    void setSampleRate(double sampleRate);
    double getSampleRate() const { return m_sampleRate; }

    void setBlockSize(uint32_t blockSize);
    uint32_t getBlockSize() const { return m_blockSize; }

    void process(std::span<std::span<float>> inputs, std::span<std::span<float>> outputs);
    void reset();

    size_t getNumInputs() const;
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
