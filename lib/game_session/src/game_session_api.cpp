#include "game_session_api.h"
#include  <memory>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <queue>
#include "log.h"
#include "player_interface_callbacks.h"
#include "player_type.h"
#include "game_action_event.h"
#include "game_session.h"

namespace GameSession {

    class GameSessionApiImpl : public IGameSessionApi {
    public:
        GameSessionApiImpl() {
            LOG_V("GameSessionApiImpl created");
            // TODO: change to shared/weak ptr impl and move to separate factory
            // Create and start the game session
            struct GameSessionCallbacks callbacks = {
                .onRequestResult = [this] (const PlayerType player, const RequestId req_id, const RequestResult result) {
                    this->sendRequestResultToPlayer(player, req_id, result);
                },
                .onPlayerShotResult = [this] (const PlayerType player, const Board::Position& position, const bool is_hit, const bool is_ship_sunk) {
                    this->sendPlayerShotResultToPlayer(player, position, is_hit, is_ship_sunk);
                },
                .onOpponentShotResult = [this] (const PlayerType opponent_player, const Board::Position& position, const bool is_hit, const bool is_ship_sunk) {
                    this->sendOpponentShotResultToPlayer(opponent_player, position, is_hit, is_ship_sunk);
                },
                .onGameFinished = [this]() {
                    this->sendGameFinishedToAllPlayers();
                },
                .onRoundEnded = [this](const GameEngine::RoundResult round_result) {
                    this->sendRoundEndedToAllPlayers(round_result);
                },
                .onScoreUpdated = [this] (const int player_score, const int opponent_score) {
                    this->sendScoreUpdatedToAllPlayers(player_score, opponent_score);
                },
                .onBoardReceived = [this] (const PlayerType player, const Board::BoardType& board) {
                    this->sendBoardToPlayer(player, board);
                },
                .onOponentBoardReceived = [this] (const PlayerType player, const Board::BoardType& oponent_board) {
                    this->sendOponentBoardTpPlayer(player, oponent_board);
                },
                .onShipsCountReceived = [this] (const PlayerType player, const Board::ShipCountMap& ships_count) {
                    this->sendShipsCountToPlayer(player, ships_count);
                },
                .onOponentShipsCountReceived = [this] (const PlayerType player, const Board::ShipCountMap& oponent_ships_count) {
                    this->sendOponentShipsCountToPlayer(player, oponent_ships_count);
                },
                .onPlayerTurnNotify = [this] (const PlayerType player_turn) {
                    this->sendPlayerTurnNotify(player_turn);
                },
                .onBroadcastPlayerNewTurn = [this] (const PlayerType player_turn) {
                    this->sendPlayerTurnBroadcast(player_turn);
                },
                .onGameStatusReceived = [this] (const GameEngine::GameStatus game_status) {
                    this->sendGameStatusToAllPlayers(game_status);
                }
            };
               game_session_ = std::make_unique<GameSession>(callbacks);
            LOG_V("Game session started");
        }

        ~GameSessionApiImpl() {
            LOG_V("GameSessionApiImpl destroyed");
            game_session_->stopSession();
        }

        bool registerPlayer(const PlayerType player,
                            std::shared_ptr<Player::IPlayerCallbacks> player_callback) override {
            if (!player_callback) {
                LOG_W("Attempted to register null player for player type: {}", board_player_type_to_string(player));
                return false;
            }
            std::unique_lock<std::mutex> lock(callback_mutex_);
            // Check if player_type is already registered
            if (players_callbacks_.find(player) != players_callbacks_.end()) {
                LOG_W("Player {} is already registered", board_player_type_to_string(player));
                return false;
            }
            players_callbacks_[player] = player_callback;
            LOG_D("Player {} registered", board_player_type_to_string(player));
            lock.unlock();

            sendCallbackActivationConfirmation(player);
            addReadyPlayerToServer(player);
            return true;
        }


        bool unregisterPlayer(const PlayerType player) override {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player);
            if (it != players_callbacks_.end()) {
                players_callbacks_.erase(it);
                LOG_D("Player: {} unregistered", board_player_type_to_string(player));
                return true;
            }
            LOG_W("Attempted to unregister non-existing player: {}", board_player_type_to_string(player));
            return false;
        }

        bool resetSession() override {
            LOG_V("Game session reset");
            const auto request_id = generateRequestId();
            struct GameActionEvent event = {
                .request_id = request_id,
                .action_type = GameActionType::RestartGame,
            };
            game_session_->addActionEvent(event);
            return true;
        }

        bool resetBoards() override {
            // Implementation of boards reset
            LOG_D("Game boards reset");
            return true;
        }

        RequestId getBoard(const PlayerType player) override {
            LOG_D("Get board requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::RequestPlayerBoard,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId getOponentBoard(const PlayerType player) override {
            LOG_D("Get opponent board requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::RequestOpponentBoard,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId getPlayerShipsCount(const PlayerType player) override {
            LOG_D("Get player ships count requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::RequestShipsCount,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId getScore(const PlayerType player) override {
            LOG_D("Get score requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::RequestScore,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId getPlayerTurn(const PlayerType player) override {
            LOG_D("Get player turn requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::RequestCurrentTurnPlayer,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId getGameStatus(const PlayerType player) override {
            LOG_D("Get game status requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::RequestGameStatus,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId setPlayerShip(const PlayerType player,
                                       const Board::ShipType ship_type,
                                       const Board::Position& position,
                                       bool is_horizontal) override {
            LOG_D("Set ship requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const GameActionArg arg = PlaceShipActionArg {
                .ship_type = ship_type,
                .position = position,
                .is_horizontal = is_horizontal,
            };
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::PlaceShip,
                .action_arg = arg,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId setPlayerShot(const PlayerType player,
                                       const Board::Position& position) override {
            LOG_D("Set shot requested by player: {}", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const GameActionArg arg = MakeShotActionArg {
                .position = position,
            };
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::MakeShot,
                .action_arg = arg,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId notifyPlayerReadyForNextRound(const PlayerType player) override {
            LOG_D("Player: {} is ready", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::NotifyReadyForNextRound,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId notifyPlayerDisconnected(const PlayerType player) override {
            LOG_D("Player: {} has disconnected", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::Disconnect,
            };
            game_session_->addActionEvent(event);
            return request_id;
        }

        RequestId stopGame() override {
            LOG_D("Request game stop");
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = PlayerType::System,
                .action_type = GameActionType::StopGame,
            };
            game_session_->addActionEvent(event);
            game_session_->stopSession();
            return request_id;
        }

    private:
        std::unique_ptr<GameSession::IGameSession> game_session_;
        std::mutex callback_mutex_;
        std::unordered_map<PlayerType, std::shared_ptr<Player::IPlayerCallbacks>> players_callbacks_;
        std::atomic<RequestId> next_request_id_{0};

        RequestId generateRequestId() {
            static std::atomic<RequestId> next_request_id{0};
            return next_request_id++;
        }

        void sendRequestResultToPlayer(const PlayerType player, const RequestId req_id, const RequestResult result) {
            if (player == PlayerType::System) {
                return;
            }
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player);
            if (it != players_callbacks_.end()) {
                it->second->onRequestResult(req_id, result);
                LOG_V("Sent request id({}) with result: {} to player ID: {}", req_id, to_cstring(result), board_player_type_to_string(player));
            } else {
                LOG_W("Attempted to send request result to non-existing player ID: {}", board_player_type_to_string(player));
            }
        }

        void sendPlayerShotResultToPlayer(const PlayerType player,
                                        const Board::Position& position,
                                        const bool is_hit,
                                        const bool is_ship_sunk) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player);
            if (it != players_callbacks_.end()) {
                it->second->onPlayerShotResult(position, is_hit, is_ship_sunk);
                LOG_V("Sent shot result to player ID: {}", board_player_type_to_string(player));
            } else {
                LOG_W("Attempted to send shot result to non-existing player ID: {}", board_player_type_to_string(player));
            }
        }

        void sendOpponentShotResultToPlayer(const PlayerType opponent_player,
                                          const Board::Position& position,
                                          const bool is_hit,
                                          const bool is_ship_sunk) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(opponent_player);
            if (it != players_callbacks_.end()) {
                it->second->onOpponentShotResult(position, is_hit, is_ship_sunk);
                LOG_V("Sent opponent shot result to player ID: {}", board_player_type_to_string(opponent_player));
            } else {
                LOG_W("Attempted to send opponent shot result to non-existing player ID: {}", board_player_type_to_string(opponent_player));
            }
        }

        void sendGameFinishedToAllPlayers() {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            for (const auto& [player_type, callbacks] : players_callbacks_) {
                callbacks->onGameFinished();
                LOG_V("Sent game finished notification to player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendRoundEndedToAllPlayers(const GameEngine::RoundResult round_result) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            for (const auto& [player_type, callbacks] : players_callbacks_) {
                callbacks->onRoundEnded(round_result);
                LOG_V("Sent round ended notification to player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendScoreUpdatedToAllPlayers(const int player_score, const int opponent_score) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            for (const auto& [player_type, callbacks] : players_callbacks_) {
                callbacks->onScoreUpdated(player_score, opponent_score);
                LOG_V("Sent score updated notification to player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendPlayerTurnNotify(const PlayerType player_turn) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player_turn);
            if (it != players_callbacks_.end()) {
                it->second->onPlayerTurnNotify();
            } else {
                LOG_W("Attempted to send player turn notification to non-existing player ID: {}", board_player_type_to_string(player_turn));
            }
            // Notify all players about the turn change
            for (const auto& [player_type, callbacks] : players_callbacks_) {
                callbacks->onNewPlayerTurnReceived(player_turn);
            }
        }

        void sendBoardToPlayer(const PlayerType player_type, const Board::BoardType& board) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player_type);
            if (it != players_callbacks_.end()) {
                it->second->onBoardReceived(board);
                LOG_D("Sent board to player ID: {}", board_player_type_to_string(player_type));
            } else {
                LOG_W("Attempted to send board to non-existing player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendOponentBoardTpPlayer(const PlayerType player, const Board::BoardType& oponent_board) {
            std::lock_guard<std::mutex> lock(callback_mutex_);

            auto it = players_callbacks_.find(player);
            if (it != players_callbacks_.end()) {
                it->second->onOponentBoardReceived(oponent_board);
                LOG_D("Sent opponent board to player ID: {}", board_player_type_to_string(player));
            } else {
                LOG_W("Attempted to send opponent board to non-existing player ID: {}", board_player_type_to_string(player));
            }
        }

        void sendShipsCountToPlayer(const PlayerType player_type, const Board::ShipCountMap& ships_count) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player_type);
            if (it != players_callbacks_.end()) {
                it->second->onShipsCountReceived(ships_count);
                LOG_D("Sent ships count to player ID: {}", board_player_type_to_string(player_type));
            } else {
                LOG_W("Attempted to send ships count to non-existing player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendOponentShipsCountToPlayer(const PlayerType player, const Board::ShipCountMap& oponent_ships_count) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player);
            if (it != players_callbacks_.end()) {
                it->second->onOponentShipsCountReceived(oponent_ships_count);
                LOG_D("Sent opponent ships count to player ID: {}", board_player_type_to_string(player));
            } else {
                LOG_W("Attempted to send ships count to non-existing player ID: {}", board_player_type_to_string(player));
            }
        }

        void sendPlayerTurnBroadcast(const PlayerType player_turn) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            for (const auto& [player_type, callbacks] : players_callbacks_) {
                callbacks->onNewPlayerTurnReceived(player_turn);
                LOG_V("Sent new player turn to player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendGameStatusToAllPlayers(const GameEngine::GameStatus game_status) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            for (const auto& [player_type, callbacks] : players_callbacks_) {
                callbacks->onGameStatusReceived(game_status);
                LOG_V("Sent game status to player ID: {}", board_player_type_to_string(player_type));
            }
        }

        void sendCallbackActivationConfirmation(const PlayerType player) {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            auto it = players_callbacks_.find(player);
            if (it != players_callbacks_.end()) {
                it->second->onCallbackActivation();
            } else {
                LOG_W("Attempted to send ships count to non-existing player ID: {}", board_player_type_to_string(player));
            }
        }

        void addReadyPlayerToServer(const PlayerType player) {
            LOG_D("Player: {} wants to start a new game", board_player_type_to_string(player));
            const auto request_id = generateRequestId();
            const struct GameActionEvent event = {
                .request_id = request_id,
                .player_id = player,
                .action_type = GameActionType::StartNewGame,
            };
            game_session_->addActionEvent(event);
        }
    };

    GameSessionApi::GameSessionApi()
        : impl_(std::make_unique<GameSessionApiImpl>()) {}

}   // namespace GameSession
