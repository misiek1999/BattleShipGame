#pragma once

#include "player_interface.h"

#include "bot_factory.h"

#include <memory>

namespace Player {

    class PlayerBot : public Player::IPlayer {
    public:
        PlayerBot(std::unique_ptr<IBotFactory>& factory, std::shared_ptr<GameSession::IGamePlayerAction>& action_interface, const PlayerType player_type);
        virtual ~PlayerBot() = default;

        // IPlayer interface implementation
        PlayerType getPlayerType() const override {
            return impl_->getPlayerType();
        }

        bool isReady() const override {
            return impl_->isReady();
        }

        bool isConnected() const override {
            return impl_->isConnected();
        }

        Board::BoardType getBoard() const override {
            return impl_->getBoard();
        }

        // IPlayerCallbacks interface implementation
        void onRequestResult(const RequestId req_id, const GameSession::RequestResult result) override {
            impl_->onRequestResult(req_id, result);
        }

        void onPlayerShotResult(const Board::Position& position, const bool is_hit,
            const bool is_ship_sunk) override {
            impl_->onPlayerShotResult(position, is_hit, is_ship_sunk);
        }

        void onOpponentShotResult(const Board::Position& position, const bool is_hit,
                                const bool is_ship_sunk) override {
            impl_->onOpponentShotResult(position, is_hit, is_ship_sunk);
        }

        void onGameFinished() override {
            impl_->onGameFinished();
        }

        void onRoundEnded(const GameEngine::RoundResult round_result) override {
            impl_->onRoundEnded(round_result);
        }

        void onScoreUpdated(const int player_score, const int opponent_score) override {
            impl_->onScoreUpdated(player_score, opponent_score);
        }

        void onBoardReceived(const Board::BoardType board) override {
            impl_->onBoardReceived(board);
        }

        void onOponentBoardReceived(const Board::BoardType board) override {
            impl_->onOponentBoardReceived(board);
        }

        void onShipsCountReceived(const Board::ShipCountMap ships_count) override {
            impl_->onShipsCountReceived(ships_count);
        }

        void onOponentShipsCountReceived(const Board::ShipCountMap oponent_ships_count) override {
            impl_->onOponentShipsCountReceived(oponent_ships_count);
        }

        void onPlayerTurnNotify() override {
            impl_->onPlayerTurnNotify();
        }

        void onNewPlayerTurnReceived(const PlayerType player_turn) override {
            impl_->onNewPlayerTurnReceived(player_turn);
        }

        void onGameStatusReceived(const GameEngine::GameStatus game_status) override {
            impl_->onGameStatusReceived(game_status);
        }

        void onCallbackActivation() override {
            impl_->onCallbackActivation();
        }

    private:
        std::unique_ptr<IPlayer> impl_;
    };

}
