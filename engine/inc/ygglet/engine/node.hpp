#pragma once

#include <boost/uuid/uuid.hpp>

#include <span>

namespace ygglet::engine {

struct Engine;

struct Node
{
    Node(std::uint32_t inputs, std::uint32_t outputs);
    virtual ~Node();

    boost::uuids::uuid id() const;
    std::uint32_t inputs() const;
    std::uint32_t outputs() const;

    virtual void process(std::span<std::span<const float>> inputs) = 0;

protected:
    std::span<std::span<float>> buffers();

private:
    friend struct Engine;

    boost::uuids::uuid m_id;

    struct
    {
        std::vector<std::vector<float>> storage;
        std::vector<std::span<float>> buffers;
    } m_buffers;

    std::uint32_t m_inputs{};
    std::uint32_t m_outputs{};
};

} // namespace ygglet::engine
