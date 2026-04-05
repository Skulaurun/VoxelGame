#include "Log.hpp"

std::shared_ptr<spdlog::logger> Log::coreLogger;

void Log::init() {
    spdlog::set_pattern("[%T] [%n] [%^%l%$] %v");

    coreLogger = spdlog::stdout_color_mt("Core");
    coreLogger->set_level(spdlog::level::trace);
}
