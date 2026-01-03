#pragma once

#include <cmajor/API/cmaj_Engine.h>
#include <cmajor/API/cmaj_Performer.h>

namespace ygglet::processor {

struct Kernal
{
    cmaj::Engine engine;
    std::vector<cmaj::EndpointHandle> inputs;
    std::vector<cmaj::EndpointHandle> outputs;
};

} // namespace ygglet::processor
