#pragma once

#include <memory>

namespace ygglet::processor {

struct Logger;

struct Module
{
    static std::shared_ptr<Module> aquire();
    ~Module();

private:
    Module();
    std::shared_ptr<Logger> m_logger;
};

} // namespace ygglet::processor
