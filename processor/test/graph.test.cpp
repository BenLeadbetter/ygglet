#include <ygglet/processor/graph.hpp>
#include <ygglet/processor/graph/endpoints.hpp>
#include <ygglet/processor/graph/nodes.hpp>
#include <ygglet/processor/kernal_manager.hpp>
#include <ygglet/processor/module.hpp>
#include <ygglet/processor/node.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <boost/uuid/generators.hpp>

namespace ygglet::processor::test {

SCENARIO("Mutating an audio graph", "[graph]")
{
    GIVEN("an empty audio graph")
    {
        const double sampleRate = 44100.0;
        const size_t maxBlockSize = 512;
        const size_t blockSize = 64;
        const auto module = Module::aquire();
        auto manager = KernalManager(sampleRate, maxBlockSize);
        auto graph = Graph(sampleRate, maxBlockSize);
        const auto& constGraph = graph;

        WHEN("process audio")
        {
            graph.process({}, {});
        }

        WHEN("insert simple pass through node")
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

            auto [key, kernal] = [&] {
                auto result = manager.load(patch);
                REQUIRE(result);
                return std::move(*result);
            }();

            auto node = std::make_unique<Node>(std::move(kernal));
            const auto* raw = node.get();
            auto id = node->id();
            graph.nodes().insert(std::move(node));

            THEN("the graph publishes the single node")
            {
                CHECK(std::distance(graph.nodes().begin(), graph.nodes().end()) == 1);
            }

            THEN("access the node via an iterator")
            {
                CHECK(&*graph.nodes().begin() == raw);
            }

            THEN("graph reports one input")
            {
                CHECK(std::distance(graph.inputs().begin(), graph.inputs().end()) == 1);
            }

            THEN("graph reports one output")
            {
                CHECK(std::distance(graph.outputs().begin(), graph.outputs().end()) == 1);
            }

            // TODO: implement this behaviour
            AND_WHEN("remove the node")
            {
                graph.nodes().remove(id);
            }

            // AND_WHEN("process audio")
            // {
            //     graph.compile();
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
            //     std::vector<std::span<float>> inputs = {input};
            //     std::vector<std::span<float>> outputs = {output};
            //
            //     graph.process(inputs, outputs);
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

            auto [key, kernal] = [&] {
                auto result = manager.load(patch);
                REQUIRE(result);
                return std::move(*result);
            }();

            auto node0 = std::make_unique<Node>(kernal);
            auto node1 = std::make_unique<Node>(kernal);
            auto node2 = std::make_unique<Node>(kernal);

            const auto id0 = node0->id();
            const auto id1 = node1->id();
            const auto id2 = node2->id();

            const auto* raw0 = node0.get();
            const auto* raw1 = node1.get();
            const auto* raw2 = node2.get();

            graph.nodes().insert(std::move(node0));
            graph.nodes().insert(std::move(node1));
            graph.nodes().insert(std::move(node2));

            THEN("the graph publishes three nodes")
            {
                CHECK(std::distance(graph.nodes().begin(), graph.nodes().end()) == 3);
            }

            THEN("graph reports three inputs")
            {
                CHECK(std::distance(graph.inputs().begin(), graph.inputs().end()) == 3);
            }

            THEN("graph reports three outputs")
            {
                CHECK(std::distance(graph.outputs().begin(), graph.outputs().end()) == 3);
            }

            THEN("indexed access into the nodes")
            {
                CHECK(&graph.nodes()[id0] == raw0);
                CHECK(&graph.nodes()[id1] == raw1);
                CHECK(&graph.nodes()[id2] == raw2);
            }

            THEN("find node by id")
            {
                CHECK(graph.nodes().find(id0) != graph.nodes().end());
                CHECK(graph.nodes().find(id1) != graph.nodes().end());
                CHECK(graph.nodes().find(id2) != graph.nodes().end());
            }

            THEN("non existend node id returns end")
            {
                CHECK(graph.nodes().find(boost::uuids::random_generator{}()) == graph.nodes().end());
            }

            THEN("const graph publishes three nodes")
            {
                CHECK(std::distance(constGraph.nodes().begin(), constGraph.nodes().end()) == 3);
            }

            THEN("indexed access via const graph into the nodes")
            {
                CHECK(&constGraph.nodes()[id0] == raw0);
                CHECK(&constGraph.nodes()[id1] == raw1);
                CHECK(&constGraph.nodes()[id2] == raw2);
            }

            THEN("find node by id on const graph")
            {
                CHECK(constGraph.nodes().find(id0) != constGraph.nodes().end());
                CHECK(constGraph.nodes().find(id1) != constGraph.nodes().end());
                CHECK(constGraph.nodes().find(id2) != constGraph.nodes().end());
            }

            THEN("non-existent node id returns end on const graph")
            {
                CHECK(constGraph.nodes().find(boost::uuids::random_generator{}()) == constGraph.nodes().end());
            }
        }
    }
}

} // namespace ygglet::processor::test
