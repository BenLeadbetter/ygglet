#include <ygglet/processor/logger.hpp>
#include <ygglet/processor/module.hpp>
#include <ygglet/processor/processor.hpp>

#include <cmajor/API/cmaj_DiagnosticMessages.h>
#include <cmajor/API/cmaj_Endpoints.h>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>

namespace ygglet::processor {


Processor::Processor()
: m_module(Module::aquire())
{
}

Processor::~Processor() = default;

void Processor::setSampleRate(double sampleRate)
{
    m_sampleRate = sampleRate;
    // TODO: Changing sample rate requires resetting the graph
}

void Processor::setBlockSize(uint32_t blockSize)
{
    m_blockSize = blockSize;
    // TODO: forward to graph
}

double Processor::getSampleRate() const
{
    return m_sampleRate;
}

uint32_t Processor::getBlockSize() const
{
    return m_blockSize;
}

// void Processor::process(std::span<std::span<float>> inputs, std::span<std::span<float>> outputs)
// {
//     // TODO: forward to graph
//     if (!m_performer)
//     {
//         for (auto output : outputs)
//         {
//             std::memset(output.data(), 0, output.size() * sizeof(float));
//         }
//         return;
//     }
//
//     for (size_t i = 0; i < std::min(inputs.size(), m_inputEndpoints.size()); ++i)
//     {
//         auto result =
//             m_performer.setInputFrames(m_inputEndpoints[i], inputs[i].data(),
//             static_cast<uint32_t>(inputs[i].size()));
//         if (result != cmaj::Result::Ok)
//         {
//             Logger::error("setInputFrames failed with result: {}", static_cast<int>(result));
//         }
//     }
//
//     auto advanceResult = m_performer.advance();
//     if (advanceResult != cmaj::Result::Ok)
//     {
//         Logger::error("advance() failed with result: {}", static_cast<int>(advanceResult));
//     }
//
//     for (size_t i = 0; i < std::min(outputs.size(), m_outputEndpoints.size()); ++i)
//     {
//         auto result = m_performer.copyOutputFrames(m_outputEndpoints[i], outputs[i].data(),
//                                                    static_cast<uint32_t>(outputs[i].size()));
//         if (result != cmaj::Result::Ok)
//         {
//             Logger::error("copyOutputFrames failed with result: {}", static_cast<int>(result));
//         }
//     }
// }

} // namespace ygglet::processor
