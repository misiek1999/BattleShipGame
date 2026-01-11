#pragma once

#include "game_types.h"
#include "board.h"
#include "game_action_event.h"
#include "game_engine.h"

namespace Player {
    /*
    /// @brief Interface for player callbacks to receive game session events
    /// @note All callback methods should be implemented by clients
    */
    class IPlayerCallbacks {
    public:
        virtual ~IPlayerCallbacks() = default;

        /// @brief Client request result callback
        /// @param req_id number of the client request
        /// @param result result of the request processing
        /// @note client -> server request are async, so clients should listen for this callback to get the result of their requests
        /// In case of success, the result is RequestResult::Ok
        virtual void onRequestResult(const RequestId req_id, const GameSession::RequestResult result) = 0;

        /// @brief Notify that the player shot result
        /// @param position position of the shot
        /// @param is_hit true if the shot was a hit, false otherwise
        /// @param is_ship_sunk true if the shot sunk a ship, false otherwise
        /// @note this callback is sent to the player who made a short
        virtual void onPlayerShotResult(const Board::Position& position, const bool is_hit,
            const bool is_ship_sunk) = 0;

        /// @brief Notify that the opponent shot result
        /// @param position position of the shot
        /// @param is_hit true if the shot was a hit, false otherwise
        /// @param is_ship_sunk true if the shot sunk a ship, false otherwise
        /// @note this callback is sent to the player when oponent make a shot
        virtual void onOpponentShotResult(const Board::Position& position, const bool is_hit,
                                          const bool is_ship_sunk) = 0;

        /// @brief Notify that the game has finished
        /// @note clients should stop server communication upon receiving this callback
        virtual void onGameFinished() = 0;

        /// @brief Notify that the round has ended
        /// @param round_result result of the round
        virtual void onRoundEnded(const GameEngine::RoundResult round_result) = 0;

        /// @brief Notify that the score has been updated
        /// @param player_score current player's score
        /// @param opponent_score current opponent's score
        virtual void onScoreUpdated(const int player_score, const int opponent_score) = 0;

        /// @brief Receive the game board for the player
        /// @param board the game board
        /// @note this board is sended by game session upon request and on the client request
        virtual void onBoardReceived(const Board::BoardType board) = 0;

        /// @brief Receive the oponent board
        /// @param board the game board for oponent player
        /// @note this board is sended by game session upon request and on the client request
        virtual void onOponentBoardReceived(const Board::BoardType board) = 0;

        /// @brief Receive the count of player remaining ships
        /// @param ships_count map of ship type to remaining count
        virtual void onShipsCountReceived(const Board::ShipCountMap ships_count) = 0;

        /// @brief Receive the count of oponent remaining ships
        /// @param ships_count map of ship type to remaining count for oponent
        virtual void onOponentShipsCountReceived(const Board::ShipCountMap ships_count) = 0;

        /// @brief Notify that it's the player's turn
        /// @note server send this callback to the player whose turn is next
        virtual void onPlayerTurnNotify() = 0;

        /// @brief Notify about the new player turn
        /// @param player_turn player whose turn is next
        /// @note this callback is sent to all players on turn change
        virtual void onNewPlayerTurnReceived(const PlayerType player_turn) = 0;

        /// @brief Receive the current game status
        /// @param game_status Game status
        /// @note this callback is sent upon request and on game status change
        /// Clients should make a different behavior based on the game status
        virtual void onGameStatusReceived(const GameEngine::GameStatus game_status) = 0;

        /// @brief Called by served when registered client callback
        virtual void onCallbackActivation() = 0;

        /// @brief Notify that players should place a ships on map
        // virtual void onStartShipPlace() = 0;
    };

} // namespace Player
