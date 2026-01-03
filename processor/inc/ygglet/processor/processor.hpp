#pragma once

#include <ygglet/processor/graph.hpp>

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
    Graph m_graph;
    std::shared_ptr<Module> m_module;
    double m_sampleRate = 44100.0;
    uint32_t m_blockSize = 512;
};

} // namespace ygglet::processor
