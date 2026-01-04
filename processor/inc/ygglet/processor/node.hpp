#pragma once

#include <cmajor/API/cmaj_Performer.h>

#include <boost/uuid/uuid.hpp>

#include <span>

namespace ygglet::processor {

struct Graph;
struct Kernal;

struct Node
{
    Node(std::shared_ptr<Kernal>);

    boost::uuids::uuid id() const;
    std::span<const boost::uuids::uuid> inputs() const;
    std::span<const boost::uuids::uuid> outputs() const;

private:
    friend class Graph;

    boost::uuids::uuid m_id;

    std::shared_ptr<Kernal> m_kernal;
    cmaj::Performer m_performer;

    std::vector<std::vector<float>> m_buffers{};

    using Endpoints = std::vector<boost::uuids::uuid>;
    Endpoints m_inputs;
    Endpoints m_outputs;
};

} // namespace ygglet::processor
