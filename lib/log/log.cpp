#include "log.h"

void init_logger() {
    // spdlog::set_level(spdlog::level::trace);
    // spdlog::set_level(spdlog::level::err);
    spdlog::set_level(spdlog::level::off);
    spdlog::set_pattern("[%H:%M:%S.%e][%L][tid:%t] %s:%# %v");
    LOG_I("Logger initialized");
}
