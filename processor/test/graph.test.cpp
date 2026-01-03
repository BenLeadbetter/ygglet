#include <ygglet/processor/graph.hpp>
#include <ygglet/processor/node.hpp>

#include <catch2/catch_test_macros.hpp>

namespace ygglet::processor::test {

SCENARIO("Mutating an audio graph", "[graph]")
{
    GIVEN("an empty audio graph")
    {
        auto graph = Graph{};

        WHEN("insert a node")
        {
        }
    }
}

} // namespace ygglet::processor::test
