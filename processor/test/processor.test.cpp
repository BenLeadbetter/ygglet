#include <ygglet/processor/processor.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace ygglet::processor::test {

SCENARIO("Creating a processor, loading patches and processing audio", "[processor]")
{
    GIVEN("a processor")
    {
        auto processor = [] {
            auto result = Processor::make();
            REQUIRE(result);
            return std::move(*result);
        }();

        WHEN("process audio")
        {
            const size_t blockSize = 64;
            std::vector<float> input(blockSize, 1.0f);
            std::vector<float> output(blockSize, 1.0f);

            std::vector<std::span<float>> inputs = {input};
            std::vector<std::span<float>> outputs = {output};

            processor.process(inputs, outputs);

            THEN("silence is rendered")
            {
                for (size_t i = 0; i < blockSize; ++i)
                {
                    REQUIRE(output[i] == 0.0f);
                }
            }
        }

        WHEN("load simple passthrough patch")
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

            auto result = processor.load(patch);

            THEN("patch is loaded")
            {
                CHECK(result);
            }

            AND_WHEN("process audio")
            {
                const size_t blockSize = 64;
                std::vector<float> input(blockSize);
                std::vector<float> output(blockSize, 0.0f);

                // test input (sine wave)
                for (size_t i = 0; i < blockSize; ++i)
                {
                    input[i] = std::sin(2.0f * M_PI * i / blockSize);
                }

                std::vector<std::span<float>> inputs = {input};
                std::vector<std::span<float>> outputs = {output};

                processor.setBlockSize(static_cast<uint32_t>(blockSize));
                processor.process(inputs, outputs);

                THEN("signal is passed through")
                {
                    for (size_t i = 0; i < blockSize; ++i)
                    {
                        CHECK(output[i] == Catch::Approx(input[i]).margin(0.00001f));
                    }
                }
            }
        }

        WHEN("load an audio genertor patch")
        {
            constexpr std::string_view patch = R"(
                processor SimpleOscillator {
                    output stream float audioOut;

                    void main() {
                        loop {
                            audioOut <- 0.25f;
                            advance();
                        }
                    }
                }
            )";

            auto result = processor.load(patch);

            THEN("patch is loaded")
            {
                CHECK(result);
            }

            AND_WHEN("process audio")
            {
                const size_t blockSize = 64;
                std::vector<float> output(blockSize, 0.0f);
                std::vector<std::span<float>> inputs = {};
                std::vector<std::span<float>> outputs = {output};

                processor.setBlockSize(static_cast<uint32_t>(blockSize));
                processor.process(inputs, outputs);

                THEN("signal is generated")
                {
                    for (size_t i = 0; i < blockSize; ++i)
                    {
                        CHECK(output[i] == Catch::Approx(0.25f).margin(0.00001f));
                    }
                }
            }
        }

        WHEN("load invalid code")
        {
            constexpr std::string_view patch = "this is not valid cmajor code";

            auto result = processor.load(patch);

            THEN("patch is not loaded")
            {
                CHECK(!result);
            }
        }
    }
}

} // namespace ygglet::processor::test
