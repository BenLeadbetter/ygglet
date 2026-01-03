#pragma once

#include <optional>
#include <string>

namespace ygglet::processor {

struct Patch
{
    std::string source;
    std::optional<std::string> filename;
};

} // namespace ygglet::processor
