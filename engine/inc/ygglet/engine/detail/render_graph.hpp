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
        std::vector<Node> nodes;
        std::vector<std::variant<std::span<const float>*, std::uint32_t>> inputs;
        std::vector<std::span<float>*> outputs;
        std::vector<float> silence;
    };

    Graph& current();
    Graph& inactive();
    void publish();

private:
    std::atomic<std::uint32_t> index{};
    std::array<Graph, 2> buffers{};
};

} // namespace ygglet::engine::detail
