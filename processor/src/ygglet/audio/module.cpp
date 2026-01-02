#include <ygglet/audio/cmajor.hpp>
#include <ygglet/audio/logger.hpp>
#include <ygglet/audio/module.hpp>

#include <mutex>

namespace ygglet::processor {

std::shared_ptr<Module> Module::aquire()
{
    static std::weak_ptr<Module> s_instance;
    static std::mutex mutex;

    std::lock_guard lock(mutex);

    auto instance = s_instance.lock();

    if (!instance)
    {
        instance = std::shared_ptr<Module>(new Module);
        s_instance = instance;
    }

    return instance;
}

Module::Module()
: m_logger(Logger::Factory::make())
{
    cmajor::init();
}

Module::~Module()
{
    cmajor::deinit();
}

} // namespace ygglet::processor
