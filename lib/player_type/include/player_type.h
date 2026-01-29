#pragma once

#include <cstdint>
#include <cassert>
#include <array>
#include <cstdlib>

enum class PlayerType : uint8_t {
    Player_1 = 0,
    Player_2 = 1,
    NumberOfPlayers,
    System
};

constexpr const std::array<PlayerType, static_cast<size_t>(PlayerType::NumberOfPlayers)> kPlayerArray = {
    PlayerType::Player_1,
    PlayerType::Player_2
};

inline constexpr const char* board_player_type_to_string(const PlayerType player_type) {
    switch (player_type) {
        case PlayerType::Player_1:  return "Player 1";
        case PlayerType::Player_2:  return "Player 2";
        case PlayerType::System:    return "System";
        default: return "Unknown Player";
    }
}

inline constexpr const char* to_cstring(const PlayerType player_type) noexcept {
    return board_player_type_to_string(player_type);
}

inline constexpr PlayerType get_opponent_player(const PlayerType player_type) {
    switch (player_type) {
        case PlayerType::Player_1: return PlayerType::Player_2;
        case PlayerType::Player_2: return PlayerType::Player_1;
        default: assert(false && "Invalid PlayerType");
            return PlayerType::Player_1; // Default return to avoid compiler warning
    }
}

enum class PlayerError {
    Ok,
    InvalidPlayer
};

enum class OponentPlayerType {
    Human,
    Bot
};

enum class BotType {
    Random,
    Smart,
    NumberOfBotTypes,
};

inline constexpr const char* to_cstring(const OponentPlayerType player_type) {
    switch (player_type) {
        case OponentPlayerType::Human: return "Human";
        case OponentPlayerType::Bot:   return "Bot";
        default: assert(false && "Invalid Oponent type");
            return ""; // Default return to avoid compiler warning
    }
}
