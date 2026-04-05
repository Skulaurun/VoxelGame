#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

class Log {

public:
    static void init();

    static std::shared_ptr<spdlog::logger> getCoreLogger() { return coreLogger; }

private:
    static std::shared_ptr<spdlog::logger> coreLogger;

};

#ifndef NDEBUG
    #define LOG_CORE_TRACE(...) Log::getCoreLogger()->trace(__VA_ARGS__)
    #define LOG_CORE_DEBUG(...) Log::getCoreLogger()->debug(__VA_ARGS__)
#else
    #define LOG_CORE_TRACE(...)
    #define LOG_CORE_DEBUG(...)
#endif

#define LOG_CORE_INFO(...) Log::getCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...) Log::getCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) Log::getCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) Log::getCoreLogger()->critical(__VA_ARGS__)
