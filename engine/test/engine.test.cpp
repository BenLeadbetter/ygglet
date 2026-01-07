#include <ygglet/engine/engine.hpp>
#include <ygglet/engine/node.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <gmock/gmock.h>

#include <boost/uuid/generators.hpp>

namespace ygglet::engine::test {

struct MockNode : Node
{
    MockNode(std::size_t inputs, std::size_t outputs)
    : Node(inputs, outputs)
    {
    }

    MOCK_METHOD(void, process, (std::span<std::span<const float>> inputs), (override));

    using Node::buffers;
};

SCENARIO("Mutating an audio engine", "[engine]")
{
    GIVEN("an empty audio engine")
    {
        const double sampleRate = 44100.0;
        const size_t maxBlockSize = 512;
        const size_t blockSize = 64;
        auto engine = Engine(1, 1, sampleRate, maxBlockSize);
        const auto& constEngine = engine;

        WHEN("process audio")
        {
            engine.process({}, {});
        }

        THEN("contains input and output nodes")
        {
            CHECK(!engine.nodes().empty());
            CHECK(engine.nodes().size() == 2);
        }

        THEN("contains no connections")
        {
            CHECK(engine.connections().empty());
        }

        WHEN("insert a connected node")
        {
            auto [node, id] = [&]() {
                auto node = std::make_unique<MockNode>(1, 1);
                auto* raw = node.get();
                auto id = node->id();
                engine.nodes().insert(std::move(node));
                return std::make_pair(raw, id);
            }();

            // connect it up
            engine.connections().insert({
                .in = {.node = id, .port = 0},
                .out = {.node = engine.nodes().input().id(), .port = 0},
            });
            engine.connections().insert({
                .in = {.node = engine.nodes().output().id(), .port = 0},
                .out = {.node = id, .port = 0},
            });

            THEN("the engine publishes the additional node")
            {
                CHECK(engine.nodes().size() == 3);
            }

            // TODO: implement this behaviour
            AND_WHEN("remove the node")
            {
                engine.nodes().remove(id);
            }

            AND_WHEN("process audio")
            {
                EXPECT_CALL(*node, process(::testing::_)).Times(1).WillOnce([node](auto inputs) {
                    REQUIRE(inputs.size() == 1);
                    REQUIRE(node->buffers().size() == 1);
                    REQUIRE(inputs[0].size() == node->buffers()[0].size());
                    // passthrough
                    std::memcpy(node->buffers()[0].data(), inputs[0].data(), inputs[0].size() * sizeof(float));
                });

                engine.compile();

                std::vector<float> input(blockSize);
                std::vector<float> output(blockSize, 0.0f);

                // test input (sine wave)
                for (size_t i = 0; i < blockSize; ++i)
                {
                    input[i] = std::sin(2.0f * M_PI * i / blockSize);
                }

                std::vector<std::span<const float>> inputs = {input};
                std::vector<std::span<float>> outputs = {output};

                engine.process(inputs, outputs);

                THEN("output contains processed audio")
                {
                    REQUIRE(testing::Mock::VerifyAndClearExpectations(node));

                    for (size_t i = 0; i < blockSize; ++i)
                    {
                        CHECK(output[i] == Catch::Approx(input[i]).margin(0.00001f));
                    }
                }
            }

            CHECK(testing::Mock::VerifyAndClearExpectations(node));
        }

        WHEN("insert three nodes")
        {
            const auto insert = [&](std::uint32_t i, std::uint32_t o) {
                auto node = std::make_unique<MockNode>(i, o);
                auto id = node->id();
                auto* raw = node.get();
                engine.nodes().insert(std::move(node));
                return std::make_pair(raw, id);
            };

            auto [node0, id0] = insert(0, 2);
            auto [node1, id1] = insert(2, 3);
            auto [node2, id2] = insert(3, 2);

            THEN("the engine publishes additional three nodes")
            {
                CHECK(engine.nodes().size() == 5);
            }

            THEN("indexed access into the nodes")
            {
                CHECK(&engine.nodes()[id0] == node0);
                CHECK(&engine.nodes()[id1] == node1);
                CHECK(&engine.nodes()[id2] == node2);
            }

            THEN("find node by id")
            {
                CHECK(engine.nodes().find(id0) != engine.nodes().end());
                CHECK(engine.nodes().find(id1) != engine.nodes().end());
                CHECK(engine.nodes().find(id2) != engine.nodes().end());
            }

            THEN("non existent node id returns end")
            {
                CHECK(engine.nodes().find(boost::uuids::random_generator{}()) == engine.nodes().end());
            }

            THEN("const engine publishes additional three nodes")
            {
                CHECK(engine.nodes().size() == 5);
            }

            THEN("indexed access via const engine into the nodes")
            {
                CHECK(&constEngine.nodes()[id0] == node0);
                CHECK(&constEngine.nodes()[id1] == node1);
                CHECK(&constEngine.nodes()[id2] == node2);
            }

            THEN("find node by id on const engine")
            {
                CHECK(constEngine.nodes().find(id0) != constEngine.nodes().end());
                CHECK(constEngine.nodes().find(id1) != constEngine.nodes().end());
                CHECK(constEngine.nodes().find(id2) != constEngine.nodes().end());
            }

            THEN("non-existent node id returns end on const engine")
            {
                CHECK(constEngine.nodes().find(boost::uuids::random_generator{}()) == constEngine.nodes().end());
            }

            CHECK(testing::Mock::VerifyAndClearExpectations(node0));
            CHECK(testing::Mock::VerifyAndClearExpectations(node1));
            CHECK(testing::Mock::VerifyAndClearExpectations(node2));
        }
    }
}

} // namespace ygglet::engine::test
