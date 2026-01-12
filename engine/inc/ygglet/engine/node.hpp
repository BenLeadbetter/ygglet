#pragma once

#include <ygglet/engine/node_id.hpp>

#include <strong_type/strong_type.hpp>

#include <boost/uuid/uuid.hpp>

#include <span>

namespace ygglet::engine {

struct Engine;

struct Node
{
    using Id = NodeId;

    Node(std::uint32_t inputs, std::uint32_t outputs);
    virtual ~Node();

    Id id() const;
    std::uint32_t inputs() const;
    std::uint32_t outputs() const;

    virtual void process(std::span<std::span<const float>> inputs) = 0;

protected:
    std::span<std::span<float>> buffers();

private:
    friend struct Engine;

    Id m_id;

    struct
    {
        std::vector<std::vector<float>> storage;
        std::vector<std::span<float>> buffers;
    } m_buffers;

    std::uint32_t m_inputs{};
    std::uint32_t m_outputs{};
};

} // namespace ygglet::engine
