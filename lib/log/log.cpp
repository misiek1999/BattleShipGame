#include "log.h"

void init_logger() {
    // spdlog::set_level(spdlog::level::trace);
    spdlog::set_level(spdlog::level::err); // Set global log level to error
    spdlog::set_pattern("[%H:%M:%S.%e][%L][tid: %t]%s:%# %v");
    spdlog::info("Logger initialized");
}
