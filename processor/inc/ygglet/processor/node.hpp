#pragma once

#include <ygglet/engine/node.hpp>

#include <cmajor/API/cmaj_Performer.h>

namespace ygglet::processor {

struct Kernal;

struct Node : engine::Node
{
    Node(std::shared_ptr<Kernal>);

    void process(std::span<std::span<const float>> inputs) override;

private:
    std::shared_ptr<Kernal> m_kernal;
    cmaj::Performer m_performer;
};

} // namespace ygglet::processor
