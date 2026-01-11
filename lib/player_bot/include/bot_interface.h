#pragma once

#include "player_interface.h"

/// @brief Interface for Player Bots
/// @note All bot classes should inherit from this interface
/// Currently no additional methods are defined beyond IPlayer
class IPlayerBot : public Player::IPlayer {
public:
    virtual ~IPlayerBot() = default;
};
