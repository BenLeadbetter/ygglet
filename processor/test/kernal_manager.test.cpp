#include <ygglet/processor/kernal_manager.hpp>
#include <ygglet/processor/module.hpp>

#include <catch2/catch_test_macros.hpp>

namespace ygglet::processor::test {

SCENARIO("managing cmajor kernal instances", "[kernal_manager]")
{
    GIVEN("a kernal manager")
    {
        auto patch = Patch{
            .source = R"(
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
            )",
        };
        auto module = Module::aquire();
        auto manager = KernalManager(44100.0, 256);

        WHEN("aquire an unloaded kernakernal")
        {
            auto result = manager.aquire(patch);

            THEN("no kernal returned")
            {
                CHECK(!result);
            }
        }

        WHEN("create an kernal with simple passthrough patch")
        {
            auto result = manager.load(patch);

            THEN("kernal is successfully created")
            {
                CHECK(result);
            }

            REQUIRE(result);
            const auto key = result->first;
            auto kernal = std::move(result->second);

            THEN("kernal is cached")
            {
                CHECK(manager.aquire(key));
            }

            THEN("inputs are cached")
            {
                CHECK(kernal->inputs.size() == 1);
            }

            THEN("outputs are cached")
            {
                CHECK(kernal->outputs.size() == 1);
            }

            AND_WHEN("kernal is destroyed")
            {
                kernal.reset();

                THEN("kernal is no longer cached")
                {
                    CHECK(!manager.aquire(key));
                }
            }
        }
    }
}

} // namespace ygglet::processor::test
