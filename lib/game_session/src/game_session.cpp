#include "game_session.h"
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include "log.h"
#include "board.h"
#include "game_action_event.h"
#include "game_engine.h"


namespace GameSession {

    const std::unordered_map<GameEngine::GameEngineError, RequestResult> engineErrorToRequestResultMap = {
        {GameEngine::GameEngineError::Ok, RequestResult::Ok},
        {GameEngine::GameEngineError::InvalidShipPosition, RequestResult::RequestRejected},
        {GameEngine::GameEngineError::InvalidShot, RequestResult::RequestRejected},
        {GameEngine::GameEngineError::InvalidPlayer, RequestResult::InvalidArguments},
        {GameEngine::GameEngineError::InvalidPlayerTurn, RequestResult::NotPlayerTurn},
        {GameEngine::GameEngineError::InvalidShipType, RequestResult::InvalidArguments},
        {GameEngine::GameEngineError::BoardIsAlreadyPrepared, RequestResult::OperationFailed},
        {GameEngine::GameEngineError::RoundFinished, RequestResult::RoundFinished},
        {GameEngine::GameEngineError::GameNotStarted, RequestResult::GameNotStarted},
    };

    RequestResult mapBoardErrorToRequestResult(const GameEngine::GameEngineError error) {
        if (const auto it = engineErrorToRequestResultMap.find(error); it != engineErrorToRequestResultMap.end()) {
            return it->second;
        }
        return RequestResult::OperationFailed;
    }

    class GameSessionImpl : public IGameSession {
    public:

        explicit GameSessionImpl(GameSessionCallbacks callbacks)
            : callbacks_(callbacks) {
            LOG_D("GameSessionImpl created");
            // Create game engine instance
            game_engine_ = std::make_unique<GameEngine::GameEngine>();
            LOG_D("Game engine instance created");
            // Start the game session thread
            session_thread_ = std::jthread(
                [this](std::stop_token stoken) { this->gameLoop(stoken); }
            );
            LOG_D("Game session initialization completed");
        }

        ~GameSessionImpl() {
            // Wait for the session thread to finish
            if (session_thread_.joinable()) {
                session_thread_.join();
            }
            LOG_V("GameSession destroyed");
        }

        bool addActionEvent(const GameActionEvent& event) override {
            if (!game_session_active_) {
                return false;
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                action_queue_.push(event);
            }
            cv_.notify_one();
            return true;
        }

        void stopSession() override {
            if (!game_session_active_) {
                LOG_I("Game session already stopped");
                return;
            }
            LOG_I("Stopping game session");
            // Notify players that game is finished
            game_session_active_ = false;
            sendGameFinished();
            session_thread_.request_stop();
            cv_.notify_one();
        }

    private:
        std::jthread session_thread_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> event_triggered_{false};
        std::queue<GameActionEvent> action_queue_;
        GameSessionCallbacks callbacks_;
        std::unique_ptr<GameEngine::IGameEngine> game_engine_;
        std::unordered_set<PlayerType> ready_players_;
        std::atomic<bool> game_session_active_{true};

        // Main game loop running in the session thread
        void gameLoop(std::stop_token stoken) {
            while (!stoken.stop_requested()) {
                std::unique_lock<std::mutex> lock(mutex_);
                // Wait for an event or stop request
                cv_.wait(lock, [this, &stoken]() {
                    return !action_queue_.empty() || stoken.stop_requested();
                });

                // Stop without processing further events
                if (stoken.stop_requested()) {
                    LOG_I("Game session stopping as requested");
                    break;
                }

                // Copy the queue to process events without holding the lock
                std::queue<GameActionEvent> local_queue;
                std::swap(local_queue, action_queue_);
                lock.unlock();

                // Process all events in the local queue
                while (!local_queue.empty()) {
                    GameActionEvent event = local_queue.front();
                    local_queue.pop();
                    processEvent(event);
                }
            }
        }

        // Event handler, executed in the session thread
        void processEvent(const GameActionEvent& event) {
            // Check event type
            const auto& action_type = event.action_type;

            LOG_V("Processing event: Request ID: {}, Player ID: {}, Action Type: {}",
                  event.request_id,
                  to_cstring(event.player_id),
                  to_cstring(event.action_type));

            auto result = RequestResult::Ok;

            // Process the event based on its type
            switch (action_type) {
                case GameActionType::PlaceShip:
                    result = placeShip(event);
                    break;
                case GameActionType::MakeShot:
                    result = makeShot(event);
                    break;
                case GameActionType::NotifyReadyForNextRound:
                    result = notifyPlayerReadyForNextRound(event);
                    break;
                case GameActionType::StartNewGame:
                    result = startNewGame(event.player_id);
                    break;
                case GameActionType::RestartGame:
                    result = resetGame();
                    break;
                case GameActionType::RequestPlayerBoard:
                    result = getBoard(event.player_id);
                    break;
                case GameActionType::RequestOpponentBoard:
                    result = requestOponentBoard(event.player_id);
                    break;
                case GameActionType::RequestGameStatus:
                    result = requestGameStatus();
                    break;
                case GameActionType::RequestScore:
                    result = requestScore(event.player_id);
                    break;
                case GameActionType::RequestShipsCount:
                    result = requestShipsCount(event.player_id);
                    break;
                case GameActionType::RequestCurrentTurnPlayer:
                    result = requestCurrentTurnPlayer(event.player_id);
                    break;
                    case GameActionType::StopGame:
                    result = stopGame();
                    break;
                default:
                    LOG_W("Action type {} not implemented yet", static_cast<uint8_t>(action_type));
                    result = RequestResult::InvalidRequest;
                    break;
            }

            LOG_V("Event processed: Request ID: {}, Player ID: {}, Action Type: {} with result: {}",
                  event.request_id,
                  to_cstring(event.player_id),
                  to_cstring(event.action_type),
                  to_cstring(result));
            sendRequestResult(event.player_id, event.request_id, result);
        }

        // Event processing methods
        RequestResult placeShip(const GameActionEvent& event) {
            try {
                const auto& arg = std::get<PlaceShipActionArg>(event.action_arg);
                const auto player = event.player_id;
                const auto result = game_engine_->setPlayerShip(player, arg.ship_type, arg.position, arg.is_horizontal);
                if (result != GameEngine::GameEngineError::Ok) {
                    LOG_W("Failed to place ship for player {}: {}", board_player_type_to_string(player), static_cast<int>(result));
                    return RequestResult::OperationFailed;
                }
                //notify how many ship player need to place
                sendShipsCountToPlayer(player);
                // send updated map with ships
                sendBoardToPlayer(player, game_engine_->getBoard(player));

                // Check all ships were placed, if yes move to game state
                const auto game_status = game_engine_->getGameStatus();
                if (game_status == GameEngine::GameStatus::RoundInProgress) {
                    startRoundShots();
                }
            } catch (const std::bad_variant_access& e) {
                LOG_E("Invalid action argument for PlaceShip: {}", e.what());
                return RequestResult::InvalidArguments;
            } catch (const std::exception& e) {
                LOG_E("Exception during PlaceShip processing: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult makeShot(const GameActionEvent& event) {
            const auto& player = event.player_id;
            try {
                const auto& arg = std::get<MakeShotActionArg>(event.action_arg);

                bool was_hitted = false;
                bool was_ship_destroyed = false;
                const auto result = game_engine_->setPlayerShot(event.player_id, arg.position, was_hitted, was_ship_destroyed);
                if (result != GameEngine::GameEngineError::Ok) {
                    LOG_W("Make shot failed for player: {}, error code: {}", board_player_type_to_string(event.player_id), to_cstring(result));
                    return RequestResult::OperationFailed;
                }
                // Notify both players about the shot result
                sendShotResult(player, arg.position, was_hitted, was_ship_destroyed);

                // In case of sunk ship, update the ships count
                if (was_ship_destroyed) {
                    sendShipsCountToPlayer(get_opponent_player(player));
                    sendBoardToPlayer(get_opponent_player(player), game_engine_->getBoard(get_opponent_player(player)));
                }

                // Notify about next player's turn
                sendPlayerTurnNotify();
                broadcastToAllPlayersCurrentTurnNotify();

                // Check if the round has finished
                const auto game_status = game_engine_->getGameStatus();
                if (game_status == GameEngine::GameStatus::RoundFinished) {
                    startRoundFinish();
                }
            } catch (const std::bad_variant_access& e) {
                LOG_E("Invalid action argument for MakeShot: {}", e.what());
                return RequestResult::InvalidArguments;
            } catch (const std::exception& e) {
                LOG_E("Exception during MakeShot processing: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult notifyPlayerReadyForNextRound(const GameActionEvent& event) {
            const auto player = event.player_id;
            const auto game_status = game_engine_->getGameStatus();
            if (game_status == GameEngine::GameStatus::RoundInProgress ||
                game_status == GameEngine::GameStatus::PreparingBoards) {
                    LOG_W("Cannot start new round, game status is {}", static_cast<int>(game_status));
                    return RequestResult::InvalidRequest;
                }
            ready_players_.insert(player);
            LOG_D("Player {} want to start new round", board_player_type_to_string(player));
            if (ready_players_.size() == static_cast<size_t>(PlayerType::NumberOfPlayers)) {
                game_engine_->resetBoards();
                ready_players_.clear();
                LOG_I("All players are ready. Round is started!");
                startRoundPraparation();
            }
            return RequestResult::Ok;
        }

        RequestResult startNewGame(const PlayerType player) {
            // We can reuse read_players set at the game start
            ready_players_.insert(player);
            if (ready_players_.size() == static_cast<size_t>(PlayerType::NumberOfPlayers)) {
                game_engine_->resetBoards();
                ready_players_.clear();
                LOG_I("All players are ready. Game is started!");
            }
            startRoundPraparation();
            return RequestResult::Ok;
        }

        RequestResult resetGame() {
            if (game_engine_->getGameStatus() == GameEngine::GameStatus::NotStarted) {
                LOG_W("Cannot reset game, game has not started yet");
                return RequestResult::GameNotStarted;
            }
            game_engine_->resetGame();
            return RequestResult::Ok;
        }

        RequestResult getBoard(const PlayerType player) {
            try {
                const auto board_opt = game_engine_->getBoard(player);
                sendBoardToPlayer(player, board_opt);
            } catch (const std::exception& e) {
                LOG_E("Failed to create board instance: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult requestOponentBoard(const PlayerType player) {
            try {
                const auto board_opt = game_engine_->getOponentBoard(player);
                sendBoardToPlayer(player, board_opt);
            } catch (const std::exception& e) {
                LOG_E("Failed to create opponent board instance: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult requestGameStatus() {
            try {
                broadcastGameStatus();
            } catch (const std::exception& e) {
                LOG_E("Failed to get game status: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult requestScore(const PlayerType player) {
            std::ignore = player;
            try {
                sendScoreUpdated();
            } catch (const std::exception& e) {
                LOG_E("Failed to get score: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult requestShipsCount(const PlayerType player) {
            try {
                sendShipsCountToPlayer(player);
                sendOponentShipsCountToPlayer(player);
            } catch (const std::exception& e) {
                LOG_E("Failed to get score: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult requestCurrentTurnPlayer(const PlayerType player) {
            std::ignore = player;   //TODO: use player info if needed
            try {
                broadcastToAllPlayersCurrentTurnNotify();
            } catch (const std::exception& e) {
                LOG_E("Failed to get current turn player: {}", e.what());
                return RequestResult::OperationFailed;
            }
            return RequestResult::Ok;
        }

        RequestResult stopGame() {
            game_engine_->stopGame();
            return RequestResult::Ok;
        }

        // Methods use to manage game between different states

        void startRoundPraparation() {
            if (game_engine_->getGameStatus() != GameEngine::GameStatus::PreparingBoards) {
                LOG_W("Unable to start game preparation when game engine is not in preparation phase");
            }
            // Broadcast to all players events to place a ships
            broadcastGameStatus();
            sendShipCountsToAllPlayers();
            notifyPlayerToPlaceShips();
        }

        void startRoundShots() {
            if (game_engine_->getGameStatus() != GameEngine::GameStatus::RoundInProgress) {
                LOG_W("Unable to start game when game engine is not in game phase");
            }
            // Broadcast to all players events to start shots
            broadcastGameStatus();
            sendPlayerTurnNotify();
        }

        void startRoundFinish() {
            if (game_engine_->getGameStatus() != GameEngine::GameStatus::RoundFinished) {
                LOG_W("Unable to start finish preparation when game engine is not in finish phase");
            }
            broadcastGameStatus();
            sendRoundEnded();
            sendScoreUpdated();
        }

        // helper functions

        void sendShipCountsToAllPlayers() {
            for (const auto player : kPlayerArray) {
                sendShipsCountToPlayer(player);
            }
        }

        // callbacks invokers

        void sendRequestResult(const PlayerType player,
                               const RequestId req_id,
                               const RequestResult result) {
            if (callbacks_.onRequestResult) {
                callbacks_.onRequestResult(player, req_id, result);
            }
        }

        void sendGameFinished() {
            if (callbacks_.onGameFinished) {
                callbacks_.onGameFinished();
            }
        }

        void sendShotResult(const PlayerType player,
                            const Board::Position& position,
                            const bool is_hit,
                            const bool is_ship_sunk) {
            sendPlayerShotResult(player, position, is_hit, is_ship_sunk);
            const auto opponent_player = get_opponent_player(player);
            sendOpponentShotResult(opponent_player, position, is_hit, is_ship_sunk);
        }

        void sendPlayerShotResult(const PlayerType player,
                                  const Board::Position& position,
                                  const bool is_hit,
                                  const bool is_ship_sunk) {
            if (callbacks_.onPlayerShotResult) {
                callbacks_.onPlayerShotResult(player, position, is_hit, is_ship_sunk);
            }
        }

        void sendOpponentShotResult(const PlayerType player,
                                    const Board::Position& position,
                                    const bool is_hit,
                                    const bool is_ship_sunk) {
            if (callbacks_.onOpponentShotResult) {
                callbacks_.onOpponentShotResult(player, position, is_hit, is_ship_sunk);
            }
        }

        void sendRoundEnded() {
            const auto round_result = game_engine_->getRoundResult();
            if (callbacks_.onRoundEnded) {
                callbacks_.onRoundEnded(round_result);
            }
        }

        void sendScoreUpdated() {
            const auto [player_1, player_2] = game_engine_->getScore();
            if (callbacks_.onScoreUpdated) {
                callbacks_.onScoreUpdated(player_1, player_2);
            }
        }

        void sendBoardToPlayer(const PlayerType player, const Board::BoardType& board) {
            if (callbacks_.onBoardReceived) {
                callbacks_.onBoardReceived(player, board);
            }
        }

        void sendOponentBoardToPlayer(const PlayerType player, const Board::BoardType& oponent_board) {
            if (callbacks_.onOponentBoardReceived) {
                callbacks_.onOponentBoardReceived(player, oponent_board);
            }
        }

        void sendShipsCountToPlayer(const PlayerType player) {
            const auto ships_count = game_engine_->getPlayerShipsCount(player);
            if (callbacks_.onShipsCountReceived) {
                callbacks_.onShipsCountReceived(player, ships_count);
            }
        }
        void sendOponentShipsCountToPlayer(const PlayerType player) {
            const auto oponent_player = get_opponent_player(player);
            const auto oponent_ships_count = game_engine_->getPlayerShipsCount(oponent_player);
            if (callbacks_.onOponentShipsCountReceived) {
                callbacks_.onOponentShipsCountReceived(player, oponent_ships_count);
            }
        }

        /// @brief Ask players to place ships on board in prepareation board state
        /// Players can place ships at the same time
        void notifyPlayerToPlaceShips() {
            if (callbacks_.onPlayerTurnNotify) {
                callbacks_.onPlayerTurnNotify(PlayerType::Player_1);
                callbacks_.onPlayerTurnNotify(PlayerType::Player_2);
            }
        }

        /// @brief This function should be used to notify player that this is his turn to make a shot
        void sendPlayerTurnNotify() {
            const auto current_player_opt = game_engine_->getCurrentTurnPlayer();
            if (callbacks_.onPlayerTurnNotify) {
                callbacks_.onPlayerTurnNotify(current_player_opt.value());
            }
        }

        /// @brief Notify all players which player should take a shot
        void broadcastToAllPlayersCurrentTurnNotify() {
            const auto current_player_opt = game_engine_->getCurrentTurnPlayer();
            if (current_player_opt) {
                callbacks_.onBroadcastPlayerNewTurn(current_player_opt.value());
            } else {
                LOG_W("Failed to get current turn player: {}", static_cast<int>(current_player_opt.error()));
            }
        }

        void broadcastGameStatus() {
            const auto game_status = game_engine_->getGameStatus();
            if (callbacks_.onGameStatusReceived) {
                callbacks_.onGameStatusReceived(game_status);
            }
        }
    };

    GameSession::GameSession::GameSession(GameSessionCallbacks callbacks)
        : impl_(std::make_unique<GameSessionImpl>(callbacks)) {}
}   // namespace GameSession
