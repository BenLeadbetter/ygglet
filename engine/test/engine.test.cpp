#include <ygglet/engine/engine.hpp>
#include <ygglet/engine/node.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <gmock/gmock.h>

#include <boost/range/iterator_range.hpp>
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

        THEN("ready")
        {
            CHECK(engine.ready());
        }

        WHEN("compile")
        {
            engine.compile();

            THEN("not ready")
            {
                CHECK(!engine.ready());

                AND_WHEN("process audio")
                {
                    std::vector<float> input(blockSize, 0.0f);
                    std::vector<float> output(blockSize, 0.0f);
                    std::vector<std::span<const float>> inputs = {input};
                    std::vector<std::span<float>> outputs = {output};
                    engine.process(inputs, outputs);

                    THEN("ready")
                    {
                        CHECK(engine.ready());
                    }
                }
            }
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
            REQUIRE(engine.connections().insert({
                .in = {.node = id, .port = 0},
                .out = {.node = engine.nodes().input().id(), .port = 0},
            }));
            REQUIRE(engine.connections().insert({
                .in = {.node = engine.nodes().output().id(), .port = 0},
                .out = {.node = id, .port = 0},
            }));

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

                EXPECT_CALL(*node, process(::testing::_)).Times(1).WillOnce([node](auto inputs) {
                    REQUIRE(inputs.size() == 1);
                    REQUIRE(node->buffers().size() == 1);
                    REQUIRE(inputs[0].size() == node->buffers()[0].size());
                    // passthrough
                    std::memcpy(node->buffers()[0].data(), inputs[0].data(), inputs[0].size() * sizeof(float));
                });

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

        WHEN("connection external input to external output")
        {
            REQUIRE(engine.connections().insert({
                .in = {.node = engine.nodes().output().id(), .port = 0},
                .out = {.node = engine.nodes().input().id(), .port = 0},
            }));

            AND_WHEN("process audio")
            {
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

                THEN("input is passed through to the output")
                {
                    for (size_t i = 0; i < blockSize; ++i)
                    {
                        CHECK(output[i] == Catch::Approx(input[i]).margin(0.00001f));
                    }
                }
            }
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

            auto [node0, id0] = insert(1, 2);
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

            AND_WHEN("connect to external input that doesn't exist")
            {
                const auto result = engine.connections().insert({
                    .in = {.node = id1, .port = 0},
                    .out = {.node = engine.nodes().input().id(), .port = 1},
                });

                THEN("fails gracefully with error")
                {
                    using namespace connection_error;
                    CHECK(result == tl::unexpected{Error{NonExistentPort{.port{
                                        .node = engine.nodes().input().id(),
                                        .port = 1,
                                    }}}});
                }
            }

            AND_WHEN("connect to external output that doesn't exist")
            {
                const auto result = engine.connections().insert({
                    .in = {.node = engine.nodes().output().id(), .port = 1},
                    .out = {.node = id0, .port = 0},
                });

                THEN("fails gracefully with error")
                {
                    using namespace connection_error;
                    CHECK(result == tl::unexpected{Error{NonExistentPort{.port{
                                        .node = engine.nodes().output().id(),
                                        .port = 1,
                                    }}}});
                }
            }

            AND_WHEN("connect to input node that doesn't exist")
            {
                const auto result = engine.connections().insert({
                    .in = {.node = {}, .port = 0},
                    .out = {.node = id0, .port = 0},
                });

                THEN("fails gracefully with error")
                {
                    using namespace connection_error;
                    CHECK(result == tl::unexpected{Error{NonExistentPort{.port{
                                        .node = {},
                                        .port = 0,
                                    }}}});
                }
            }

            AND_WHEN("connect to output node that doesn't exist")
            {
                const auto result = engine.connections().insert({
                    .in = {.node = id1, .port = 0},
                    .out = {.node = {}, .port = 0},
                });

                THEN("fails gracefully with error")
                {
                    using namespace connection_error;
                    CHECK(result == tl::unexpected{Error{NonExistentPort{.port{
                                        .node = {},
                                        .port = 0,
                                    }}}});
                }
            }

            AND_WHEN("connect a non-trivial chain")
            {
                const auto result = engine.connections().insert({
                    .in = {.node = id1, .port = 0},
                    .out = {.node = {}, .port = 0},
                });

                THEN("fails gracefully with error")
                {
                    using namespace connection_error;
                    CHECK(result == tl::unexpected{Error{NonExistentPort{.port{
                                        .node = {},
                                        .port = 0,
                                    }}}});
                }
            }

            AND_WHEN("connect a node to itself")
            {
                const auto connection = Connection{
                    .in = {.node = id1, .port = 0},
                    .out = {.node = id1, .port = 0},
                };
                const auto result = engine.connections().insert(connection);

                THEN("fails gracefully with error")
                {
                    using namespace connection_error;
                    CHECK(result == tl::unexpected{Error{FeedbackLoop{connection}}});
                }
            }

            AND_WHEN("connect non-trivial graph")
            {
                // in.0 -> n1.0
                // n0.0 -> n1.1
                //         n1.2 -> out.0

                boost::uuids::uuid connection0{};
                boost::uuids::uuid connection1{};
                boost::uuids::uuid connection2{};

                REQUIRE(engine.connections()
                            .insert({
                                .in = {.node = id1, .port = 0},
                                .out = {.node = engine.nodes().input().id(), .port = 0},
                            })
                            .map([&](auto id) {
                                connection0 = id;
                            }));
                REQUIRE(engine.connections()
                            .insert({
                                .in = {.node = id1, .port = 1},
                                .out = {.node = id0, .port = 0},
                            })
                            .map([&](auto id) {
                                connection1 = id;
                            }));
                REQUIRE(engine.connections()
                            .insert({
                                .in = {.node = engine.nodes().output().id(), .port = 0},
                                .out = {.node = id1, .port = 2},
                            })
                            .map([&](auto id) {
                                connection2 = id;
                            }));

                REQUIRE(connection0 != boost::uuids::uuid{});
                REQUIRE(connection1 != boost::uuids::uuid{});
                REQUIRE(connection2 != boost::uuids::uuid{});

                THEN("connections reported")
                {
                    const auto connections = std::vector(engine.connections().begin(), engine.connections().end());
                    CHECK(connections.size() == 3);
                }

                AND_WHEN("disconnect node")
                {
                    engine.connections().disconnect(id1);

                    THEN("connections removed")
                    {
                        CHECK(
                            std::find_if(engine.connections().begin(), engine.connections().end(), [&](const auto& p) {
                                return p.second.in.node == id1 || p.second.out.node == id1;
                            }) == engine.connections().end());
                    }
                }

                AND_WHEN("connect feedback loop")
                {
                    const auto connection = Connection{
                        .in = {.node = id0, .port = 0},
                        .out = {.node = id1, .port = 1},
                    };
                    const auto result = engine.connections().insert(connection);

                    THEN("fails gracefully with error")
                    {
                        using namespace connection_error;
                        CHECK(result == tl::unexpected{Error{FeedbackLoop{connection}}});
                    }
                }

                AND_WHEN("connect to already connected port")
                {
                    const auto connection = Connection{
                        .in = {.node = engine.nodes().output().id(), .port = 0},
                        .out = {.node = id2, .port = 0},
                    };
                    const auto result = engine.connections().insert(connection);

                    THEN("fails gracefully with error")
                    {
                        using namespace connection_error;
                        CHECK(result == tl::unexpected{Error{AlreadyConnected{connection2}}});
                    }
                }

                AND_WHEN("connect to already connected port with force")
                {
                    const auto connection = Connection{
                        .in = {.node = engine.nodes().output().id(), .port = 0},
                        .out = {.node = id2, .port = 0},
                    };
                    const auto result = engine.connections().insert(connection, true);

                    THEN("connection is made")
                    {
                        CHECK(result);
                    }

                    THEN("previous connection is removed")
                    {
                        CHECK(engine.connections().find(connection2) == engine.connections().end());
                    }
                }

                AND_WHEN("process audio")
                {
                    engine.compile();

                    EXPECT_CALL(*node0, process(::testing::_)).Times(1).WillOnce([node = node0](auto inputs) {
                        REQUIRE(inputs.size() == 1);
                        REQUIRE(node->buffers().size() == 2);
                        // constant signal
                        std::ranges::fill(node->buffers()[0], 0.4f);
                        std::ranges::fill(node->buffers()[1], 0.4f);
                    });
                    EXPECT_CALL(*node1, process(::testing::_)).Times(1).WillOnce([node = node1](auto inputs) {
                        REQUIRE(inputs.size() == 2);
                        REQUIRE(node->buffers().size() == 3);
                        // add inputs to out 3
                        for (auto i = 0; i != inputs[0].size(); ++i)
                        {
                            node->buffers()[2][i] = inputs[0][i] + inputs[1][i];
                        }
                    });
                    // node not connected to output => not called to process
                    EXPECT_CALL(*node2, process(::testing::_)).Times(0);

                    std::vector<float> input(blockSize, 0.3f);
                    std::vector<float> output(blockSize, 0.0f);
                    std::vector<std::span<const float>> inputs = {input};
                    std::vector<std::span<float>> outputs = {output};

                    engine.process(inputs, outputs);

                    THEN("output contains processed audio")
                    {
                        REQUIRE(testing::Mock::VerifyAndClearExpectations(node0));
                        REQUIRE(testing::Mock::VerifyAndClearExpectations(node1));
                        REQUIRE(testing::Mock::VerifyAndClearExpectations(node2));

                        for (size_t i = 0; i < blockSize; ++i)
                        {
                            CHECK(output[i] == Catch::Approx(0.7f).margin(0.00001f));
                        }
                    }
                }
            }

            CHECK(testing::Mock::VerifyAndClearExpectations(node0));
            CHECK(testing::Mock::VerifyAndClearExpectations(node1));
            CHECK(testing::Mock::VerifyAndClearExpectations(node2));
        }
    }
}

} // namespace ygglet::engine::test
