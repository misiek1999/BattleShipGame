#pragma once

#include <utility>
#include <memory>

#include "log.h"
#include "player_type.h"
#include "board.h"
#include "player_interface_callbacks.h"


namespace Player {

    class IPlayer : public IPlayerCallbacks{
    public:
        virtual ~IPlayer() = default;

        /// @brief Get the player type
        /// @return PlayerType enum value representing the player type
        virtual PlayerType getPlayerType() const = 0;

        /// @brief Check if the player is ready
        /// @return true if the player is ready, false otherwise
        virtual bool isReady() const = 0;

        /// @brief Check if the player is connected
        /// @return true if the player is connected, false otherwise
        virtual bool isConnected() const = 0;

        /// @brief Get the player's game board
        /// @return The player's game board as a 2D array
        virtual Board::BoardType getBoard() const = 0;
    };

} // namespace Player
