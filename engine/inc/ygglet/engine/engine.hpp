#pragma once

#include <ygglet/engine/detail/connections.hpp>
#include <ygglet/engine/detail/control_graph.hpp>
#include <ygglet/engine/detail/nodes.hpp>
#include <ygglet/engine/detail/render_graph.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/uuid/uuid.hpp>

#include <span>

namespace ygglet::engine {

struct Connection;
struct Engine;
struct Node;

namespace detail {

template <typename> struct Connections;
template <typename> struct Nodes;

} // namespace detail

struct Engine
{
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

    // audio thread

    void process(std::span<std::span<const float>> inputs, std::span<std::span<float>> outputs);

    // control threads
    Engine(std::size_t inputs, std::size_t outputs, double sampleRate, std::uint32_t blockSize);
    ~Engine();

    void compile();

    auto nodes() -> detail::Nodes<Engine>;
    auto nodes() const -> detail::Nodes<const Engine>;
    auto connections() -> detail::Connections<Engine>;
    auto connections() const -> detail::Connections<const Engine>;

private:
    template <typename> friend struct detail::Connections;
    template <typename> friend struct detail::Nodes;

    double m_sampleRate;
    std::uint32_t m_blockSize;

    detail::ControlGraph m_control;
    detail::RenderGraph m_render;
};

} // namespace ygglet::engine
