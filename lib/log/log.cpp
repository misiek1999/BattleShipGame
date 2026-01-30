#include "log.h"
#include <spdlog/sinks/basic_file_sink.h>

constexpr const auto kLogPath = "log/game.log";
void init_logger(const bool dump_to_file) {
    if (dump_to_file) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            kLogPath, true
        );

        auto logger = std::make_shared<spdlog::logger>(
            "game", file_sink
        );
        spdlog::set_level(spdlog::level::trace);
        spdlog::set_default_logger(logger);
    } else {

        spdlog::set_level(spdlog::level::off);
    }
    spdlog::set_pattern("[%H:%M:%S.%e][%L][tid:%t] %s:%# %v");
    LOG_I("Logger initialized");
}
