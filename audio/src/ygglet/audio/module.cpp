#include <ygglet/audio/logger.h>
#include <ygglet/audio/module.h>

#include <mutex>

namespace ygglet::audio {

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
}

} // namespace ygglet::audio
