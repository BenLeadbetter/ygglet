#include <ygglet/audio/processor.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace ygglet::audio::test {

SCENARIO("Creating a processor, loading patches and processing audio", "[processor]")
{
    GIVEN("a processor")
    {
        auto processor = [] {
            auto result = Processor::make();
            REQUIRE(result);
            return std::move(*result);
        }();

        WHEN("load simple patch")
        {
            constexpr std::string_view patch = R"(
                processor SimpleProcessor {
                    input stream float audioIn;
                    output stream float audioOut;

                    void main() {
                        loop {
                            audioOut <- audioIn;
                            advance();
                        }
                    }
                }
            )";

            auto result = processor.load(patch, "simple.cmajor");

            THEN("patch is loaded")
            {
                CHECK(result);
                CHECK(processor.isLoaded());
            }
        }

        WHEN("load invalid code")
        {
            constexpr std::string_view patch = "this is not valid cmajor code";

            auto result = processor.load(patch, "simple.cmajor");

            THEN("patch is not loaded")
            {
                CHECK(!result);
                CHECK(!processor.isLoaded());
            }
        }
    }
}

// TEST_CASE("Processor can process audio through simple passthrough", "[processor]")
// {
//     Processor processor;
//
//     const char* passthroughCode = R"(
//         processor Passthrough {
//             input stream float audioIn;
//             output stream float audioOut;
//
//             void main() {
//                 loop {
//                     audioOut <- audioIn;
//                     advance();
//                 }
//             }
//         }
//     )";
//
//     REQUIRE(processor.load(passthroughCode));
//
//     SECTION("Processes audio correctly")
//     {
//         const size_t blockSize = 64;
//         std::vector<float> input(blockSize);
//         std::vector<float> output(blockSize, 0.0f);
//
//         // Generate test input (sine wave)
//         for (size_t i = 0; i < blockSize; ++i)
//         {
//             input[i] = std::sin(2.0f * M_PI * i / blockSize);
//         }
//
//         std::vector<std::span<float>> inputs = {input};
//         std::vector<std::span<float>> outputs = {output};
//
//         processor.setBlockSize(static_cast<uint32_t>(blockSize));
//         processor.process(inputs, outputs);
//
//         // Output should match input
//         for (size_t i = 0; i < blockSize; ++i)
//         {
//             REQUIRE_THAT(output[i], WithinAbs(input[i], 0.0001f));
//         }
//     }
// }
//
// TEST_CASE("Processor can generate audio", "[processor]")
// {
//     Processor processor;
//
//     const char* oscillatorCode = R"(
//         processor SimpleOscillator {
//             output stream float audioOut;
//
//             void main() {
//                 loop {
//                     audioOut <- 0.25f;
//                     advance();
//                 }
//             }
//         }
//     )";
//
//     REQUIRE(processor.load(oscillatorCode));
//
//     const size_t blockSize = 64;
//     std::vector<float> output(blockSize, 0.0f);
//     std::vector<std::span<float>> inputs = {};
//     std::vector<std::span<float>> outputs = {output};
//
//     processor.setBlockSize(static_cast<uint32_t>(blockSize));
//     processor.process(inputs, outputs);
//
//     // Output should be constant 0.25
//     for (size_t i = 0; i < blockSize; ++i)
//     {
//         REQUIRE_THAT(output[i], WithinAbs(0.25f, 0.0001f));
//     }
// }
//
// TEST_CASE("Processor handles reset correctly", "[processor]")
// {
//     Processor processor;
//
//     const char* statefulCode = R"(
//         processor Counter {
//             output stream int audioOut;
//
//             int counter;
//
//             void main() {
//                 loop {
//                     audioOut <- counter;
//                     counter++;
//                     advance();
//                 }
//             }
//         }
//     )";
//
//     REQUIRE(processor.load(statefulCode));
//
//     // Note: This test verifies that reset() can be called without errors
//     // Actual state reset verification would require more complex testing
//     REQUIRE_NOTHROW(processor.reset());
// }
//
// TEST_CASE("Processor reports endpoint counts", "[processor]")
// {
//     Processor processor;
//
//     SECTION("Simple processor has 1 input and 1 output")
//     {
//         const char* simpleCode = R"(
//             processor Simple {
//                 input stream float audioIn;
//                 output stream float audioOut;
//
//                 void main() {
//                     loop {
//                         audioOut <- audioIn;
//                         advance();
//                     }
//                 }
//             }
//         )";
//
//         REQUIRE(processor.load(simpleCode));
//         REQUIRE(processor.getNumInputs() == 1);
//         REQUIRE(processor.getNumOutputs() == 1);
//     }
//
//     SECTION("Generator has 0 inputs and 1 output")
//     {
//         const char* generatorCode = R"(
//             processor Generator {
//                 output stream float audioOut;
//
//                 void main() {
//                     loop {
//                         audioOut <- 0.0f;
//                         advance();
//                     }
//                 }
//             }
//         )";
//
//         REQUIRE(processor.load(generatorCode));
//         REQUIRE(processor.getNumInputs() == 0);
//         REQUIRE(processor.getNumOutputs() == 1);
//     }
// }
//
// TEST_CASE("Processor handles sample rate and block size", "[processor]")
// {
//     Processor processor;
//
//     SECTION("Default values are set")
//     {
//         REQUIRE(processor.getSampleRate() == 44100.0);
//         REQUIRE(processor.getBlockSize() == 512);
//     }
//
//     SECTION("Can change sample rate")
//     {
//         processor.setSampleRate(48000.0);
//         REQUIRE(processor.getSampleRate() == 48000.0);
//     }
//
//     SECTION("Can change block size")
//     {
//         processor.setBlockSize(256);
//         REQUIRE(processor.getBlockSize() == 256);
//     }
// }
//
// TEST_CASE("Processor handles empty program gracefully", "[processor]")
// {
//     Processor processor;
//
//     SECTION("Processing with no loaded program produces silence")
//     {
//         const size_t blockSize = 64;
//         std::vector<float> input(blockSize, 1.0f);
//         std::vector<float> output(blockSize, 1.0f);
//
//         std::vector<std::span<float>> inputs = {input};
//         std::vector<std::span<float>> outputs = {output};
//
//         // Should not crash and should zero the output
//         REQUIRE_NOTHROW(processor.process(inputs, outputs));
//
//         for (size_t i = 0; i < blockSize; ++i)
//         {
//             REQUIRE(output[i] == 0.0f);
//         }
//     }
// }

} // namespace ygglet::audio::test
