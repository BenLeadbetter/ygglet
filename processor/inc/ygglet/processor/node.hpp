#pragma once

#include <cmajor/API/cmaj_Engine.h>
#include <cmajor/API/cmaj_Performer.h>
#include <cmajor/API/cmaj_Program.h>

#include <boost/uuid/uuid.hpp>

#include <tl/expected.hpp>

namespace ygglet::processor {

struct Graph;

struct Node
{
    void process();

    std::uint32_t numberOfInputs() const;
    std::uint32_t numberOfOutputs() const;

    boost::uuids::uuid id() const;
    boost::uuids::uuid input(std::uint32_t) const;
    boost::uuids::uuid output(std::uint32_t) const;

private:
    friend class Graph;
    friend class NodeBuilder;

    double m_sampleRate = 44100.0;
    std::uint32_t m_blockSize = 512;

    cmaj::Engine m_engine;
    cmaj::Performer m_performer;

    struct Endpoint
    {
        cmaj::EndpointHandle handle;
        boost::uuids::uuid id;
    };
    using Endpoints = std::vector<Endpoint>;
    struct
    {
        Endpoints inputs;
        Endpoints ouputs;
    } m_endpoints;
};

} // namespace ygglet::processor
