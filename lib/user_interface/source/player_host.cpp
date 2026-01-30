#include "player_host.h"
#include "log.h"

UserInterface::PlayerHost::PlayerHost(std::shared_ptr<UserInterface> ui) : ui_(ui) {
    if (ui_ == nullptr) {
        throw std::runtime_error("UserInterface pointer is null");
    }
    LOG_D("Create player host");
}

bool UserInterface::PlayerHost::setActionHandler(std::shared_ptr<GameSession::IGamePlayerAction> action_interface) {
    if (action_interface_ != nullptr) {
        LOG_W("Action host was already set!");
        return false;
    }
    action_interface_ = action_interface;
    LOG_D("host action handler set");
    return true;
}

bool UserInterface::PlayerHost::makeShot(const Board::Position &position, bool& was_hit, bool& was_sunk) {
    was_hit = false;
    was_sunk = false;
    if (action_interface_ == nullptr) {
        LOG_E("Action interface is not set");
        return false;
    }
    std::unique_lock<std::mutex> lock(make_shot_m_);
    shot_cond_ = false;
    std::ignore = action_interface_->makeShot(position);
    if (make_shot_cv_.wait_for(lock, std::chrono::milliseconds(100), [this]{ return shot_cond_; })) {
        was_hit = shot_result_is_hit_;
        was_sunk = shot_result_ship_sunk_;
    } else {
        LOG_E("Timeout waiting for shot result");
        return false;
    }
    return shot_result_was_correct_;
}

bool UserInterface::PlayerHost::placeShip(const Board::ShipType ship_type, const Board::Position &position, const bool is_horizontal) {
    if (action_interface_ == nullptr) {
        LOG_E("Action interface is not set");
        return false;
    }
    last_request_id_ = action_interface_->placeShip(ship_type, position, is_horizontal);
    if (!place_ship_sem_.try_acquire_for(std::chrono::milliseconds(100))) {
        LOG_E("Timeout waiting for shot result");
        return false;
    }
    return place_ship_was_correct_;
}

bool UserInterface::PlayerHost::sendMessage(const std::string &message) {
    if (action_interface_ == nullptr) {
        LOG_E("Action interface is not set");
        return false;
    }
    std::ignore = action_interface_->sendChatMessage(message);
    return true;
}

bool UserInterface::PlayerHost::notifyReady() {
    if (action_interface_ == nullptr) {
        LOG_E("Action interface is not set");
        return false;
    }
    std::ignore = action_interface_->notifyReadyForNextRound();
    return true;
}

bool UserInterface::PlayerHost::endGame() {
    if (action_interface_ == nullptr) {
        LOG_E("Action interface is not set");
        return false;
    }
    std::ignore = action_interface_->stopGame();
    return true;
}

PlayerType UserInterface::PlayerHost::getPlayerType() const {
    return player_type_;
}

bool UserInterface::PlayerHost::isReady() const {
    return true;
}

bool UserInterface::PlayerHost::isConnected() const {
    return true;
}

Board::BoardType UserInterface::PlayerHost::getBoard() const {
    return board_;
}

void UserInterface::PlayerHost::onRequestResult(const RequestId req_id, const GameSession::RequestResult result) {
    LOG_V("UserInterface::PlayerHost received request result: {} for request ID: {}", to_cstring(result), req_id);
    if (req_id == last_request_id_) {
        if (result == GameSession::RequestResult::Ok) {
            shot_result_was_correct_ = true;
            place_ship_was_correct_ = true;
        } else {
            shot_result_was_correct_ = false;
            place_ship_was_correct_ = false;
        }
        place_ship_sem_.release();
    }
}

void UserInterface::PlayerHost::onPlayerShotResult(const Board::Position& position, const bool is_hit,
        const bool is_ship_sunk) {
    if (is_hit) {
        LOG_D("UserInterface::PlayerHost's shot at ({}, {}) was a hit{}", position.first, position.second,
              is_ship_sunk ? " and sunk a ship!" : "!");
    } else {
        LOG_D("UserInterface::PlayerHost's shot at ({}, {}) was a miss.", position.first, position.second);
    }
    shot_result_is_hit_ = is_hit;
    shot_result_ship_sunk_ = is_ship_sunk;
    {
        std::unique_lock<std::mutex> lock(make_shot_m_);
        shot_cond_ = true;
    }
    make_shot_cv_.notify_one();
}

void UserInterface::PlayerHost::onOpponentShotResult(const Board::Position& position, const bool is_hit,
                              const bool is_ship_sunk) {
    if (is_hit) {
        LOG_D("Opponent's shot at ({}, {}) was a hit{}", position.first, position.second,
              is_ship_sunk ? " and sunk a ship!" : "!");
    } else {
        LOG_D("Opponent's shot at ({}, {}) was a miss.", position.first, position.second);
    }
}

void UserInterface::PlayerHost::onGameFinished() {
    ui_->onGameFinished();
}

void UserInterface::PlayerHost::onRoundEnded(const GameEngine::RoundResult round_result) {
    ui_->onRoundEnded(round_result);
}

void UserInterface::PlayerHost::onScoreUpdated(const int player_score, const int opponent_score) {
    ui_->onScoreUpdated(player_score, opponent_score);
}

void UserInterface::PlayerHost::onBoardReceived(const Board::BoardType board) {
    board_ = board;
    ui_->onBoardReceived(board);
}

void UserInterface::PlayerHost::onOponentBoardReceived(const Board::BoardType oponent_board) {
    ui_->onOponentBoardReceived(oponent_board);
}

void UserInterface::PlayerHost::onShipsCountReceived(const Board::ShipCountMap ships_count) {
    ui_->onShipsCountReceived(ships_count);
    std::lock_guard lock(mutex_);
    this->ships_count_ = ships_count;
}

void UserInterface::PlayerHost::onOponentShipsCountReceived(const Board::ShipCountMap ships_count) {
    ui_->onOponentShipsCountReceived(ships_count);
    std::lock_guard lock(mutex_);
    this->oponent_ships_count_ = ships_count;
}

void UserInterface::PlayerHost::onPlayerTurnNotify() {
    ui_->onPlayerTurnNotify();
}

void UserInterface::PlayerHost::onNewPlayerTurnReceived(const PlayerType player_turn) {
    std::ignore = player_turn;
}

void UserInterface::PlayerHost::onGameStatusReceived(const GameEngine::GameStatus game_status) {
    ui_->onGameStatusUpdated(game_status);
    std::lock_guard lock(mutex_);
    current_game_status_ = game_status;
}

void UserInterface::PlayerHost::onCallbackActivation() {
}
