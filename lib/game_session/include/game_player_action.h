#pragma once

#include <memory>
#include <string>
#include "game_types.h"
#include "board.h"
#include "player_type.h"
#include "game_session_api.h"

namespace GameSession {

    class IGamePlayerAction {
    public:
        virtual ~IGamePlayerAction() = default;

        virtual RequestId placeShip(const Board::ShipType ship_type,
                                    const Board::Position& position,
                                    const bool is_horizontal) = 0;

        virtual RequestId makeShot(const Board::Position& position) = 0;

        virtual RequestId notifyReadyForNextRound() = 0;

        virtual RequestId disconnect() = 0;

        virtual RequestId stopGame() = 0;

        virtual RequestId restartGame() = 0;

        virtual RequestId sendChatMessage(const std::string& message) = 0;

        virtual RequestId surrender() = 0;

        virtual RequestId pauseGame() = 0;

        virtual RequestId resumeGame() = 0;

        virtual RequestId requestPlayerBoard() = 0;

        virtual RequestId requestOpponentBoard() = 0;

        virtual RequestId requestGameStatus() = 0;

        virtual RequestId requestScore() = 0;

        virtual RequestId requestShipsCount() = 0;

        virtual RequestId requestCurrentTurnPlayer() = 0;
    };

    class GamePlayerActionImpl;

    class GamePlayerAction : public IGamePlayerAction {
    public:
        GamePlayerAction(std::shared_ptr<IGameSessionApi> game_session,
                   const PlayerType player_id);
        ~GamePlayerAction() = default;

        RequestId placeShip(const Board::ShipType ship_type,
                            const Board::Position& position,
                            const bool is_horizontal) override {
            return impl_->placeShip(ship_type, position, is_horizontal);
        }

        RequestId makeShot(const Board::Position& position) override {
            return impl_->makeShot(position);
        }

        RequestId notifyReadyForNextRound() override {
            return impl_->notifyReadyForNextRound();
        }

        RequestId disconnect() override {
            return impl_->disconnect();
        }

        RequestId stopGame() override {
            return impl_->stopGame();
        }

        RequestId restartGame() override {
            return impl_->restartGame();
        }

        RequestId sendChatMessage(const std::string& message) override {
            return impl_->sendChatMessage(message);
        }

        RequestId surrender() override {
            return impl_->surrender();
        }

        RequestId pauseGame() override {
            return impl_->pauseGame();
        }

        RequestId resumeGame() override {
            return impl_->resumeGame();
        }

        RequestId requestPlayerBoard() override {
            return impl_->requestPlayerBoard();
        }

        RequestId requestOpponentBoard() override {
            return impl_->requestOpponentBoard();
        }

        RequestId requestGameStatus() override {
            return impl_->requestGameStatus();
        }

        RequestId requestScore() override {
            return impl_->requestScore();
        }

        RequestId requestShipsCount() override {
            return impl_->requestShipsCount();
        }

        RequestId requestCurrentTurnPlayer() override {
            return impl_->requestCurrentTurnPlayer();
        }

    private:
        std::unique_ptr<IGamePlayerAction> impl_;
    };



}   // namespace GameSession
