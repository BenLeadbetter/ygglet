#pragma once

#include <boost/uuid/uuid.hpp>

#include <span>

namespace ygglet::engine {

template <typename> struct Nodes;
struct Engine;

struct Node
{
    Node(std::size_t inputs, std::size_t outputs);
    virtual ~Node();

    boost::uuids::uuid id() const;
    std::span<const boost::uuids::uuid> inputs() const;
    std::span<const boost::uuids::uuid> outputs() const;

    virtual void process(std::span<std::span<const float>> inputs) = 0;

protected:
    std::span<std::span<float>> buffers();

private:
    friend struct Engine;
    template <typename> friend struct Nodes;

    boost::uuids::uuid m_id;

    struct
    {
        std::vector<std::vector<float>> storage;
        std::vector<std::span<float>> buffers;
    } m_buffers;

    using Endpoints = std::vector<boost::uuids::uuid>;
    Endpoints m_inputs;
    Endpoints m_outputs;
};

} // namespace ygglet::engine
