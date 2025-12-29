#pragma once

#include <memory>

#include <spdlog/spdlog.h>

namespace ygglet::audio {

class Module;

struct Logger
{
    class Factory
    {
        friend Module;
        static std::shared_ptr<Logger> make();
    };

    template <spdlog::level::level_enum L, typename... Args>
    static void log(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        if (auto instance = s_instance.lock())
        {
            instance->m_logger->log(L, fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args> static void trace(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        log<spdlog::level::trace>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> static void debug(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        log<spdlog::level::debug>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> static void info(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        log<spdlog::level::info>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> static void warn(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        log<spdlog::level::warn>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> static void error(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        log<spdlog::level::debug>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> static void critical(const spdlog::format_string_t<Args...>& fmt, Args&&... args)
    {
        log<spdlog::level::critical>(fmt, std::forward<Args>(args)...);
    }

private:
    Logger();
    static std::weak_ptr<Logger> s_instance;
    std::unique_ptr<spdlog::logger> m_logger{};
};

} // namespace ygglet::audio
