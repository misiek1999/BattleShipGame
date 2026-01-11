#include "bot_base.h"
#include "log.h"

PlayerBotBase::PlayerBotBase(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type):
        action_interface_(action_interface),
        player_type_(player_type) {
    if (!action_interface_) {
        LOG_E("PlayerBotBase action interface is null");
        throw std::invalid_argument("Action interface cannot be null");
    }
    LOG_V("PlayerBotBase created");
}

PlayerBotBase::~PlayerBotBase() {
    LOG_V("PlayerBotBase destroyed");
}

PlayerType PlayerBotBase::getPlayerType() const {
    return player_type_;
}

bool PlayerBotBase::isReady() const {
    return true;
}

bool PlayerBotBase::isConnected() const {
    return true;
}

Board::BoardType PlayerBotBase::getBoard() const {
    return board_;
}

void PlayerBotBase::onRequestResult(const RequestId req_id, const GameSession::RequestResult result) {
    LOG_D("PlayerBotBase received request result: {} for request ID: {}", to_cstring(result), req_id);
    std::unique_lock lock(mutex_);
    // Ignore unknown requestId
    const auto itr = request_map_.find(req_id);
    if (itr != request_map_.end()) {
        lock.unlock();
        const auto req_type = itr->second;
        // Retry in case of actionerror
        if (result != GameSession::RequestResult::Ok) {
            switch (req_type) {
                case RequesBotType::MakeShot: {
                    LOG_D("Retry bot shot");
                    makeShotHelper();
                    break;
                }
                case RequesBotType::PlaceShip: {
                    LOG_D("Retry bot ship place");
                    placeShipsHelper();
                    break;
                }
                default: {
                    LOG_E("Unknown request type: {}", static_cast<int>(req_type));
                    break;
                }
            }
        }
        lock.lock();
        request_map_.erase(req_id);
    }
}

void PlayerBotBase::onPlayerShotResult(const Board::Position& position, const bool is_hit,
        const bool is_ship_sunk) {
    if (is_hit) {
        LOG_D("PlayerBotBase's shot at ({}, {}) was a hit{}", position.first, position.second,
              is_ship_sunk ? " and sunk a ship!" : "!");
    } else {
        LOG_D("PlayerBotBase's shot at ({}, {}) was a miss.", position.first, position.second);
    }
}

void PlayerBotBase::onOpponentShotResult(const Board::Position& position, const bool is_hit,
                              const bool is_ship_sunk) {
    if (is_hit) {
        LOG_D("Opponent's shot at ({}, {}) was a hit{}", position.first, position.second,
              is_ship_sunk ? " and sunk a ship!" : "!");
    } else {
        LOG_D("Opponent's shot at ({}, {}) was a miss.", position.first, position.second);
    }
}

void PlayerBotBase::onGameFinished() {
    LOG_D("PlayerBotBase received game finished notification.");
}

void PlayerBotBase::onRoundEnded(const GameEngine::RoundResult round_result) {
    LOG_D("PlayerBotBase received round ended notification with result: {}", static_cast<int>(round_result));
    std::unique_lock lock(mutex_);
    request_map_.clear();
}

void PlayerBotBase::onScoreUpdated(const int player_score, const int opponent_score) {
    LOG_D("PlayerBotBase's score updated: Player Score = {}, Opponent Score = {}", player_score, opponent_score);
}

void PlayerBotBase::onBoardReceived(const Board::BoardType board) {
    std::unique_lock lock(mutex_);
    board_ = board;
    lock.unlock();
    LOG_D("PlayerBotBase received its game board.");
    const auto ascii_board = board_to_ascii_string(board);
    LOG_D("Board ASCII representation:\n{}", ascii_board);
}

void PlayerBotBase::onOponentBoardReceived(const Board::BoardType oponent_board) {
    LOG_D("PlayerBotBase received opponent's game board.");
    const auto ascii_board = board_to_ascii_string(oponent_board);
    LOG_D("Board ASCII representation:\n{}", ascii_board);
}

void PlayerBotBase::onShipsCountReceived(const Board::ShipCountMap ships_count) {
    LOG_V("Receive bot ship count: {}", Board::ship_count_to_string(ships_count).c_str());
    std::lock_guard lock(mutex_);
    this->bot_ships_count_ = ships_count;
}

void PlayerBotBase::onOponentShipsCountReceived(const Board::ShipCountMap ships_count) {
    std::lock_guard lock(mutex_);
    this->oponent_ships_count_ = ships_count;
}

void PlayerBotBase::onPlayerTurnNotify() {
    LOG_D("PlayerBotBase received notification that action is needed");
    std::unique_lock lock(mutex_);
    switch (current_game_status_) {
        case GameEngine::GameStatus::GameInProgress:
            makeShotHelper();
            break;
        case GameEngine::GameStatus::PreparingBoards:
            placeShipsHelper();
            break;
        default:
            LOG_W("PlayerBotBase received turn notification in unexpected game status: {}", static_cast<int>(current_game_status_));
            break;
    }
}

void PlayerBotBase::placeShipsHelper() {
    if (current_game_status_ != GameEngine::GameStatus::PreparingBoards) {
        LOG_W("Unable to place ship, when game is in state: {}", int(current_game_status_));
    }
    //update number of available ships to place
    for (std::size_t i = 0; i < available_bot_ships_.size(); ++i) {
        available_bot_ships_[i] = Board::kMaxAvailableShipsArray[i] - bot_ships_count_[i];
    }
    ShipType ship_type = Board::ShipType::Destro
    const auto req_id = placeShip(ship_type);
    request_map_.insert({req_id, RequesBotType::PlaceShip});
}

void PlayerBotBase::makeShotHelper() {
    if (current_game_status_ != GameEngine::GameStatus::GameInProgress) {
        LOG_W("Unable to shot ship, when game is in state: {}", int(current_game_status_));
    }
    const auto req_id = makeShot();
    request_map_.insert({req_id, RequesBotType::MakeShot});
}

void PlayerBotBase::onNewPlayerTurnReceived(const PlayerType player_turn) {
    std::ignore = player_turn;
}

void PlayerBotBase::onGameStatusReceived(const GameEngine::GameStatus game_status) {
    std::lock_guard lock(mutex_);
    current_game_status_ = game_status;
}

void PlayerBotBase::onCallbackActivation() {
}
