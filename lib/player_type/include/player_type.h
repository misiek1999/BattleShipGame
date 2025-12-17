#pragma once

#include <cstdint>

enum class BoardPlayerType : uint8_t {
    Player_1 = 0,
    Player_2 = 1
};

inline constexpr const char* board_player_type_to_string(const BoardPlayerType player_type) {
    switch (player_type) {
        case BoardPlayerType::Player_1: return "Player 1";
        case BoardPlayerType::Player_2: return "Player 2";
        default: return "Unknown Player";
    }
}

inline constexpr BoardPlayerType get_opponent_player(const BoardPlayerType player_type) {
    switch (player_type) {
        case BoardPlayerType::Player_1: return BoardPlayerType::Player_2;
        case BoardPlayerType::Player_2: return BoardPlayerType::Player_1;
        default: throw std::invalid_argument("Invalid player type");
    }
}

enum class PlayerError {
    NONE,
    INVALID_PLAYER
};
