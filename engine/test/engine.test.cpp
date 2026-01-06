#include <ygglet/engine/connections.hpp>
#include <ygglet/engine/endpoints.hpp>
#include <ygglet/engine/engine.hpp>
#include <ygglet/engine/node.hpp>
#include <ygglet/engine/nodes.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <boost/uuid/generators.hpp>

namespace ygglet::engine::test {

struct PassthroughNode : Node
{
    PassthroughNode()
    : Node(1, 1)
    {
    }

    void process(std::span<std::span<const float>> inputs) override
    {
        REQUIRE(inputs.size() == 1);
        REQUIRE(buffers().size() == 1);
        REQUIRE(inputs[0].size() == buffers()[0].size());
        std::memcpy(buffers()[0].data(), inputs[0].data(), inputs[0].size());
    }
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
            CHECK(std::distance(engine.nodes().begin(), engine.nodes().end()) == 0);
        }

        THEN("contains no connections")
        {
            CHECK(std::distance(engine.connections().begin(), engine.connections().end()) == 0);
        }

        THEN("contains no inputs")
        {
            CHECK(std::distance(engine.inputs().begin(), engine.inputs().end()) == 0);
        }

        THEN("contains no outputs")
        {
            CHECK(std::distance(engine.outputs().begin(), engine.outputs().end()) == 0);
        }

        WHEN("insert simple pass through node")
        {
            auto node = std::make_unique<PassthroughNode>();
            const auto* raw = node.get();
            auto id = node->id();
            engine.nodes().insert(std::move(node));

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
                engine.nodes().remove(id);
            }

            // AND_WHEN("process audio")
            // {
            //     engine.compile();
            //
            //     std::vector<float> input(blockSize);
            //     std::vector<float> output(blockSize, 0.0f);
            //
            //     // test input (sine wave)
            //     for (size_t i = 0; i < blockSize; ++i)
            //     {
            //         input[i] = std::sin(2.0f * M_PI * i / blockSize);
            //     }
            //
            //     std::vector<std::span<const float>> inputs = {input};
            //     std::vector<std::span<float>> outputs = {output};
            //
            //     engine.process(inputs, outputs);
            //
            //     THEN("signal is passed through")
            //     {
            //         for (size_t i = 0; i < blockSize; ++i)
            //         {
            //             CHECK(output[i] == Catch::Approx(input[i]).margin(0.00001f));
            //         }
            //     }
            // }
        }

        WHEN("insert three simple pass through nodes")
        {
            auto node0 = std::make_unique<PassthroughNode>();
            auto node1 = std::make_unique<PassthroughNode>();
            auto node2 = std::make_unique<PassthroughNode>();

            const auto id0 = node0->id();
            const auto id1 = node1->id();
            const auto id2 = node2->id();

            const auto* raw0 = node0.get();
            const auto* raw1 = node1.get();
            const auto* raw2 = node2.get();

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
        }
    }
}

} // namespace ygglet::engine::test
