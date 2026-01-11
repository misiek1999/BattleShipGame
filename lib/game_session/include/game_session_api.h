#pragma once

#include <memory>
#include "game_engine.h"
#include "player_type.h"
#include "player_interface_callbacks.h"

namespace GameSession {

    class IGameSessionApi {
    public:
        virtual ~IGameSessionApi() = default;

        /// @brief Register a player in the game session
        /// @param player player type
        /// @param player_callback shared pointer to the player callbacks interface
        /// @return Return true if the player was successfully registered, false otherwise
        virtual bool registerPlayer(const PlayerType player,
                                    std::shared_ptr<Player::IPlayerCallbacks> player_callback) = 0;

        /// @brief Unregister a player from the game session
        /// @param player_id unique player ID
        /// @return true if the player was successfully unregistered, false otherwise
        virtual bool unregisterPlayer(const PlayerType player_id) = 0;

        /// @brief Reset the game session to its initial state
        /// @return true if the session was successfully reset, false otherwise
        virtual bool resetSession() = 0;

        /// @brief Reset the game boards for both players
        /// @return true if the boards were successfully reset, false otherwise
        virtual bool resetBoards() = 0;

        /// @brief Get the game board for the specified player, board will be sent via callback
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId getBoard(const PlayerType player_id) = 0;

        /// @brief Get the opponent's game board for the specified player, board will be sent via callback
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId getOponentBoard(const PlayerType player_id) = 0;

        /// @brief Get the number of ships for the specified player, count will be sent via callback
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId getPlayerShipsCount(const PlayerType player_id) = 0;

        /// @brief Get the score for the specified player, score will be sent via callback
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId getScore(const PlayerType player_id) = 0;

        /// @brief Get the current turn player ID, will be sent via callback
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId getPlayerTurn(const PlayerType player_id) = 0;

        /// @brief Get the current game status, status will be sent via callback
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId getGameStatus(const PlayerType player_id) = 0;

        /// @brief Set a ship on the player's board
        /// @param player_id unique player ID
        /// @param ship_type type of the ship to place
        /// @param position position to place the ship
        /// @param is_horizontal orientation of the ship
        /// @return RequestId of the operation
        virtual RequestId setPlayerShip(const PlayerType player_id,
                                               const Board::ShipType ship_type,
                                               const Board::Position& position,
                                               bool is_horizontal) = 0;

        /// @brief Make a shot on the opponent's board
        /// @param player_id unique player ID
        /// @param position position of the shot
        /// @return RequestId of the operation
        virtual RequestId setPlayerShot(const PlayerType player_id,
                                               const Board::Position& position) = 0;

        /// @brief Notify that the player is ready to start game
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId notifyPlayerReadyForNextRound(const PlayerType player_id) = 0;

        /// @brief Notify that the player has disconnected
        /// @param player_id unique player ID
        /// @return RequestId of the operation
        virtual RequestId notifyPlayerDisconnected(const PlayerType player_id) = 0;

        /// @brief stop game
        /// @return RequestId of the operation
        virtual RequestId stopGame() = 0;
    };


    class GameSessionApi : public IGameSessionApi{
    public:

        GameSessionApi();
        ~GameSessionApi() = default;

        bool registerPlayer(const PlayerType player,
                            std::shared_ptr<Player::IPlayerCallbacks> player_callback) override {
            return impl_->registerPlayer(player, player_callback);
        }

        bool unregisterPlayer(const PlayerType player_id) override {
            return impl_->unregisterPlayer(player_id);
        }

        bool resetSession() override {
            return impl_->resetSession();
        }

        bool resetBoards() override {
            return impl_->resetBoards();
        }

        RequestId getBoard(const PlayerType player_id) override {
            return impl_->getBoard(player_id);
        }

        RequestId getOponentBoard(const PlayerType player_id) override {
            return impl_->getOponentBoard(player_id);
        }

        RequestId getPlayerShipsCount(const PlayerType player_id) override {
            return impl_->getPlayerShipsCount(player_id);
        }

        RequestId getScore(const PlayerType player_id) override {
            return impl_->getScore(player_id);
        }

        RequestId getPlayerTurn(const PlayerType player_id) override {
            return impl_->getPlayerTurn(player_id);
        }

        RequestId getGameStatus(const PlayerType player_id) override {
            return impl_->getGameStatus(player_id);
        }

        RequestId setPlayerShip(const PlayerType player_id,
                                       const Board::ShipType ship_type,
                                       const Board::Position& position,
                                       bool is_horizontal) override {
            return impl_->setPlayerShip(player_id, ship_type, position, is_horizontal);
        }

        RequestId setPlayerShot(const PlayerType player_id,
                                       const Board::Position& position) override {
            return impl_->setPlayerShot(player_id, position);
        }

        RequestId notifyPlayerReadyForNextRound(const PlayerType player_id) override {
            return impl_->notifyPlayerReadyForNextRound(player_id);
        }

        RequestId notifyPlayerDisconnected(const PlayerType player_id) override {
            return impl_->notifyPlayerDisconnected(player_id);
        }

        RequestId stopGame() override {
            return impl_->stopGame();
        }
    private:
        std::unique_ptr<IGameSessionApi> impl_;
    };

}; // namespace GameSession
