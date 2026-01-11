#pragma once

#include <memory>
#include <functional>
#include "board.h"
#include "game_engine.h"
#include "player_type.h"
#include "game_action_event.h"

namespace GameSession {

    struct GameSessionCallbacks {
        std::function<void(const PlayerType player, const RequestId req_id, const RequestResult result)> onRequestResult = {};
        std::function<void(const PlayerType player, const Board::Position& position, const bool is_hit, const bool is_ship_sunk)> onPlayerShotResult = {};
        std::function<void(const PlayerType player, const Board::Position& position, const bool is_hit, const bool is_ship_sunk)> onOpponentShotResult = {};
        std::function<void()> onGameFinished = {};
        std::function<void(const GameEngine::RoundResult round_result)> onRoundEnded = {};
        std::function<void(const int player_1_score, const int player_2_score)> onScoreUpdated = {};
        std::function<void(const PlayerType player, const Board::BoardType& board)> onBoardReceived = {};
        std::function<void(const PlayerType oponent_player, const Board::BoardType& board)> onOponentBoardReceived = {};
        std::function<void(const PlayerType player, const Board::ShipCountMap& ships_count)> onShipsCountReceived = {};
        std::function<void(const PlayerType player, const Board::ShipCountMap& ships_count)> onOponentShipsCountReceived = {};
        std::function<void(const PlayerType player)> onPlayerTurnNotify = {};
        std::function<void(const PlayerType player_turn)> onBroadcastPlayerNewTurn = {};
        std::function<void(const GameEngine::GameStatus game_status)> onGameStatusReceived = {};
    };

    class IGameSession {
    public:
        virtual ~IGameSession() = default;

        /// @brief Add a game action event to the session's event queue
        /// @param event The game action event to add
        /// @return True if the event was successfully added, false otherwise
        virtual bool addActionEvent(const GameActionEvent& event) = 0;

        /// @brief Stop the game session
        virtual void stopSession() = 0;
    };


    class GameSession : public IGameSession {
    public:
        explicit GameSession(GameSessionCallbacks callbacks);
        ~GameSession() = default;

        bool addActionEvent(const GameActionEvent& event) override {
            return impl_->addActionEvent(event);
        }

        void stopSession() override {
            impl_->stopSession();
        }
    private:
        std::unique_ptr<IGameSession> impl_;
    };

}; // namespace GameSession
