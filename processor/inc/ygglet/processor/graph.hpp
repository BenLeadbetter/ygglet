#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/uuid/uuid.hpp>

#include <array>
#include <atomic>
#include <span>
#include <vector>

namespace ygglet::processor {

struct Node;
struct Connection;

namespace graph {

template <typename> struct Nodes;
template <typename> struct Connections;
template <typename, typename> struct Endpoints;

namespace detail {

struct Inputs
{
    static std::span<const boost::uuids::uuid> endpoints(const Node&);
};

struct Outputs
{
    static std::span<const boost::uuids::uuid> endpoints(const Node&);
};

struct ControlGraph
{
    using Nodes = boost::container::flat_map<boost::uuids::uuid, std::unique_ptr<Node>>;
    using Connections = boost::container::flat_set<Connection>;
    Nodes nodes;
    Connections connections;
};

struct RenderGraph
{
    struct Node
    {
        std::vector<std::span<float>> inputs;
        Node* node;
    };
    using Graph = std::vector<Node>;
    struct
    {
        std::atomic<std::uint32_t> current{};
        std::array<Graph, 2> buffers{};
    } buffer;
};

} // namespace detail
} // namespace graph

struct Graph
{
    Graph(const Graph&) = delete;
    Graph(Graph&&) = delete;
    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) = delete;

    // audio thread

    void process(std::span<std::span<float>> inputs, std::span<std::span<float>> outputs);

    // control threads

    Graph(double sampleRate, std::uint32_t blockSize);
    ~Graph();

    void compile();

    auto nodes() -> graph::Nodes<Graph>;
    auto nodes() const -> graph::Nodes<const Graph>;

    auto connections() -> graph::Connections<Graph>;
    auto connections() const -> graph::Connections<const Graph>;

    auto inputs() const -> graph::Endpoints<const Graph, graph::detail::Inputs>;
    auto outputs() const -> graph::Endpoints<const Graph, graph::detail::Outputs>;

private:
    template <typename, typename> friend struct graph::Endpoints;
    template <typename> friend struct graph::Connections;
    template <typename> friend struct graph::Nodes;

    double m_sampleRate;
    std::uint32_t m_blockSize;

    struct
    {
        graph::detail::ControlGraph control;
        graph::detail::RenderGraph render;
    } m_graph;
};

} // namespace ygglet::processor
