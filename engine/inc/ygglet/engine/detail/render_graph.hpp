#pragma once

#include <array>
#include <atomic>
#include <span>
#include <vector>

namespace ygglet::engine {
struct Node;
}

namespace ygglet::engine::detail {

struct RenderGraph
{
    struct Node
    {
        engine::Node* node;
        std::vector<std::span<const float>> inputs;
    };
    struct Graph
    {
        using BufferReferenceConst = std::span<const float>*;
        using BufferReference = std::span<float>*;
        using Passthrough = std::uint32_t;
        std::vector<Node> nodes;
        std::vector<std::variant<BufferReferenceConst, Passthrough>> inputs;
        std::vector<BufferReference> outputs;
        std::vector<float> silence;
        std::atomic<std::uint32_t> epoch{};
    };

    Graph& current();
    Graph& inactive();
    void publish();

private:
    std::atomic<std::uint32_t> index{};
    std::array<Graph, 2> buffers{};
};

} // namespace ygglet::engine::detail
