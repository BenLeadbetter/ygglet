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
        auto engine = Engine(sampleRate, maxBlockSize);
        const auto& constEngine = engine;

        WHEN("process audio")
        {
            engine.process({}, {});
        }

        THEN("contains no nodes")
        {
            CHECK(engine.nodes().empty());
        }

        THEN("contains no connections")
        {
            CHECK(engine.connections().empty());
        }

        THEN("contains no inputs")
        {
            CHECK(engine.inputs().empty());
        }

        THEN("contains no outputs")
        {
            CHECK(engine.outputs().empty());
        }

        WHEN("add inputs")
        {
            engine.inputs().resize(3);

            THEN("engine reports new number of inputs")
            {
                CHECK(engine.inputs().size() == 3);
            }
        }

        WHEN("add outputs")
        {
            engine.outputs().resize(3);

            THEN("engine reports new number of inputs")
            {
                CHECK(engine.outputs().size() == 3);
            }
        }

        WHEN("insert a connected node")
        {
            auto node = std::make_unique<MockNode>(1, 1);
            auto* raw = node.get();
            engine.nodes().insert(std::move(node));

            // connect it up
            engine.inputs().resize(1);
            engine.outputs().resize(1);
            engine.connections().insert(Connection{
                .in = Connection::Endpoint{0},
                .out =
                    Connection::Node{
                        .id = raw->id(),
                        .port = raw->inputs().front(),
                    },
            });
            engine.connections().insert(Connection{
                .in =
                    Connection::Node{
                        .id = raw->id(),
                        .port = raw->outputs().front(),
                    },
                .out = Connection::Endpoint{0},
            });

            THEN("the engine publishes the single node")
            {
                CHECK(std::distance(engine.nodes().begin(), engine.nodes().end()) == 1);
            }

            THEN("access the node via an iterator")
            {
                CHECK(&*engine.nodes().begin() == raw);
            }

            // TODO: implement this behaviour
            AND_WHEN("remove the node")
            {
                engine.nodes().remove(raw->id());
            }

            AND_WHEN("process audio")
            {
                EXPECT_CALL(*raw, process(::testing::_)).Times(1).WillOnce([raw](auto inputs) {
                    REQUIRE(inputs.size() == 1);
                    REQUIRE(raw->buffers().size() == 1);
                    REQUIRE(inputs[0].size() == raw->buffers()[0].size());
                    std::memcpy(raw->buffers()[0].data(), inputs[0].data(), inputs[0].size() * sizeof(float));
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

                THEN("process is called")
                {
                    CHECK(testing::Mock::VerifyAndClearExpectations(raw));
                }

                THEN("signal is passed through")
                {
                    for (size_t i = 0; i < blockSize; ++i)
                    {
                        CHECK(output[i] == Catch::Approx(input[i]).margin(0.00001f));
                    }
                }
            }

            CHECK(testing::Mock::VerifyAndClearExpectations(raw));
        }

        WHEN("insert three nodes")
        {
            auto node0 = std::make_unique<MockNode>(0, 2);
            auto node1 = std::make_unique<MockNode>(2, 3);
            auto node2 = std::make_unique<MockNode>(3, 2);

            const auto id0 = node0->id();
            const auto id1 = node1->id();
            const auto id2 = node2->id();

            auto* raw0 = node0.get();
            auto* raw1 = node1.get();
            auto* raw2 = node2.get();

            engine.nodes().insert(std::move(node0));
            engine.nodes().insert(std::move(node1));
            engine.nodes().insert(std::move(node2));

            THEN("the engine publishes three nodes")
            {
                CHECK(std::distance(engine.nodes().begin(), engine.nodes().end()) == 3);
            }

            THEN("indexed access into the nodes")
            {
                CHECK(&engine.nodes()[id0] == raw0);
                CHECK(&engine.nodes()[id1] == raw1);
                CHECK(&engine.nodes()[id2] == raw2);
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

            THEN("const engine publishes three nodes")
            {
                CHECK(std::distance(constEngine.nodes().begin(), constEngine.nodes().end()) == 3);
            }

            THEN("indexed access via const engine into the nodes")
            {
                CHECK(&constEngine.nodes()[id0] == raw0);
                CHECK(&constEngine.nodes()[id1] == raw1);
                CHECK(&constEngine.nodes()[id2] == raw2);
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

            CHECK(testing::Mock::VerifyAndClearExpectations(raw0));
            CHECK(testing::Mock::VerifyAndClearExpectations(raw1));
            CHECK(testing::Mock::VerifyAndClearExpectations(raw2));
        }
    }
}

} // namespace ygglet::engine::test
