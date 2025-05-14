#pragma once

#include <cstdint>

enum class BoardPlayerType : uint8_t {
    X = 0,
    O = 1
};

enum class PlayerError {
    NONE,
    INVALID_PLAYER
};
