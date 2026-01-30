#pragma once

#include "board.h"
#include "player_interface.h"
#include "game_player_action.h"
#include "user_interface.h"
#include "host_action.h"

#include <memory>
#include <mutex>
#include <semaphore>

namespace UserInterface {

    /// @brief Implementaion of host player
    class PlayerHost : public Player::IPlayer, public IHostAction {
        public:
        explicit PlayerHost(std::shared_ptr<UserInterface> ui);

        bool setActionHandler(std::shared_ptr<GameSession::IGamePlayerAction> action_interface);

        virtual bool makeShot(const Board::Position& position, bool& was_hit, bool& was_sunk) override;

        virtual bool placeShip(const Board::ShipType ship_type, const Board::Position& position, const bool is_horizontal) override;

        virtual bool sendMessage(const std::string& message) override;

        virtual bool notifyReady() override;

        virtual bool endGame() override;


        // IPlayer interface implementation
        virtual PlayerType getPlayerType() const override;

        virtual bool isReady() const override;

        virtual bool isConnected() const override;

        virtual Board::BoardType getBoard() const override;

        // IPlayerCallbacks interface implementation
        virtual void onRequestResult(const RequestId req_id, const GameSession::RequestResult result) override;

        virtual void onPlayerShotResult(const Board::Position& position, const bool is_hit,
            const bool is_ship_sunk) override;

        virtual void onOpponentShotResult(const Board::Position& position, const bool is_hit,
                                const bool is_ship_sunk) override;

        virtual void onGameFinished() override;

        virtual void onRoundEnded(const GameEngine::RoundResult round_result) override;

        virtual void onScoreUpdated(const int player_score, const int opponent_score) override;

        virtual void onBoardReceived(const Board::BoardType board) override;

        virtual void onOponentBoardReceived(const Board::BoardType board) override;

        virtual void onShipsCountReceived(const Board::ShipCountMap ships_count) override;

        virtual void onOponentShipsCountReceived(const Board::ShipCountMap oponent_ships_count) override;

        virtual void onPlayerTurnNotify() override;

        virtual void onNewPlayerTurnReceived(const PlayerType player_turn) override;

        virtual void onGameStatusReceived(const GameEngine::GameStatus game_status) override;

        virtual void onCallbackActivation() override;

    private:
        std::shared_ptr<GameSession::IGamePlayerAction> action_interface_;
        std::shared_ptr<UserInterface> ui_;
        PlayerType player_type_{PlayerType::Player_1};
        Board::BoardType board_;
        GameEngine::GameStatus current_game_status_ = GameEngine::GameStatus::NotStarted;
        Board::ShipCountMap ships_count_ = {};
        Board::ShipCountMap oponent_ships_count_ = {};
        bool shot_result_is_hit_ {};
        bool shot_result_ship_sunk_ {};
        bool shot_result_was_correct_ {};
        bool place_ship_was_correct_ {};
        std::binary_semaphore make_shot_sem_{0};
        std::binary_semaphore place_ship_sem_{0};
        RequestId last_request_id_ {};

        std::mutex mutex_;
    };

}   // namespace UserInterface
