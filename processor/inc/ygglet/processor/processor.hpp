#pragma once

#include <ygglet/engine/engine.hpp>

#include <memory>

namespace ygglet::processor {

struct Module;

struct Processor
{
    Processor();
    ~Processor();

    Processor(const Processor&) = delete;
    Processor& operator=(const Processor&) = delete;
    Processor(Processor&&) noexcept = delete;
    Processor& operator=(Processor&&) noexcept = delete;

    void setSampleRate(double sampleRate);
    double getSampleRate() const;

    void setBlockSize(uint32_t blockSize);
    uint32_t getBlockSize() const;

private:
    double m_sampleRate{44100.0};
    uint32_t m_blockSize{512};

    engine::Engine m_graph;
    std::shared_ptr<Module> m_module;
};

} // namespace ygglet::processor
