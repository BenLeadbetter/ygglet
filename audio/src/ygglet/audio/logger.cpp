#include <ygglet/audio/logger.h>

#include <boost/assert.hpp>
#include <boost/predef/os.h>


#if BOOST_OS_WINDOWS
#include <spdlog/sinks/msvc_sink.h>
#else
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/syslog_sink.h>
#endif

namespace ygglet::audio {

Logger::Logger() = default;

std::weak_ptr<Logger> Logger::s_instance = {};

std::shared_ptr<Logger> Logger::Factory::make()
{
    BOOST_ASSERT_MSG(!s_instance.lock(), "Only one instance per process");

    auto instance = std::shared_ptr<Logger>(new Logger);

    const auto sinks = spdlog::sinks_init_list{
#if BOOST_OS_WINDOWS
        std::make_shared<spdlog::sinks::msvc_sink_mt>(),
#else
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
        std::make_shared<spdlog::sinks::syslog_sink_mt>("ygglet", 0, LOG_USER, true),
#endif
    };

    instance->m_logger = std::make_unique<spdlog::logger>("ygglet.audio", std::move(sinks));

    instance->m_logger->set_level(
#ifdef NDEBUG
        spdlog::level::trace
#else
        spdlog::level::warn
#endif
    );

    Logger::s_instance = instance;

    return instance;
}

} // namespace ygglet::audio
