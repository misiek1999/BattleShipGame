#include "game_player_action.h"
#include <memory>

namespace GameSession {

    class GamePlayerActionImpl : public IGamePlayerAction {
    public:
        GamePlayerActionImpl(std::shared_ptr<IGameSessionApi> game_session,
                       const PlayerType player_id)
            : game_session_(game_session),
              player_id_(player_id) {}

        RequestId placeShip(const Board::ShipType ship_type,
                            const Board::Position& position,
                            const bool is_horizontal) {
            return game_session_->setPlayerShip(player_id_, ship_type, position, is_horizontal);
        }

        RequestId makeShot(const Board::Position& position) {
            return game_session_->setPlayerShot(player_id_, position);
        }

        RequestId notifyReadyForNextRound() {
            return game_session_->notifyPlayerReadyForNextRound(player_id_);
        }

        RequestId disconnect() {
            return game_session_->notifyPlayerDisconnected(player_id_);
        }

        RequestId stopGame() {
            // Implementation of stop game action
            // This is a placeholder implementation
            return 0; // Dummy return
        }

        RequestId restartGame() {
            // Implementation of restart game action
            // This is a placeholder implementation
            return 0; // Dummy return
        }

        RequestId sendChatMessage(const std::string& message) {
            std::ignore = message;
            // Implementation of send chat message action
            // This is a placeholder implementation
            return 0; // Dummy return
        }

        RequestId surrender() {
            // Implementation of surrender action
            // This is a placeholder implementation
            return 0; // Dummy return
        }

        RequestId pauseGame() {
            // Implementation of pause game action
            // This is a placeholder implementation
            return 0; // Dummy return
        }

        RequestId resumeGame() {
            // Implementation of resume game action
            // This is a placeholder implementation
            return 0; // Dummy return
        }

        RequestId requestPlayerBoard() {
            return game_session_->getBoard(player_id_);
        }

        RequestId requestOpponentBoard() {
            return game_session_->getOponentBoard(player_id_);
        }

        RequestId requestGameStatus() {
            return game_session_->getGameStatus(player_id_);
        }

        RequestId requestScore() {
            return game_session_->getScore(player_id_);
        }

        RequestId requestShipsCount() {
            return game_session_->getPlayerShipsCount(player_id_);
        }

        RequestId requestCurrentTurnPlayer() {
            return game_session_->getPlayerTurn(player_id_);
        }

    private:
        std::shared_ptr<IGameSessionApi> game_session_;
        PlayerType player_id_;
    }; // class GamePlayerActionImpl

    GamePlayerAction::GamePlayerAction(std::shared_ptr<IGameSessionApi> game_session,
                           const PlayerType player_id)
        : impl_(std::make_unique<GamePlayerActionImpl>(game_session, player_id)) {
        }

}   // namespace GameSession