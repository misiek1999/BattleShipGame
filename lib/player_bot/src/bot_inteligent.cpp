#include "bot_inteligent.h"
#include "board.h"

BotInteligent::BotInteligent(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
        const PlayerType player_type)
    : PlayerBotBase(action_interface, player_type),
      gen_(rd_()) {
    LOG_V("Create a smart bot");
    tried_.resize(Board::kBoardSizeRow,
                  std::vector<bool>(Board::kBoardSizeCol, false));
}

RequestId BotInteligent::placeShip(const Board::ShipType ship_type) {
    std::uniform_int_distribution<> row(0, Board::kBoardSizeRow - 1);
    std::uniform_int_distribution<> col(0, Board::kBoardSizeCol - 1);
    std::uniform_int_distribution<> rot(0, 1);

    Board::Position pos;
    bool horizontal;

    do {
        pos = {row(gen_), col(gen_)};
        horizontal = rot(gen_);
    } while (!Board::Board::is_possible_to_place_ship(
        board_, pos, ship_type, !horizontal));

    return action_interface_->placeShip(ship_type, pos, horizontal);
}

RequestId BotInteligent::makeShot() {
    Board::Position pos;

    if (mode_ == Mode::Target && !target_queue_.empty()) {
        pos = target_queue_.front();
        target_queue_.pop_front();
    } else {
        mode_ = Mode::Hunt;
        do {
            pos = randomPosition();
        } while (!isValidTarget(pos));
    }

    tried_[pos.first][pos.second] = true;
    return action_interface_->makeShot(pos);
}

void BotInteligent::onPlayerShotResult(const Board::Position &position, const bool is_hit, const bool is_ship_sunk) {
    if (is_ship_sunk) {
        onShipSunk();
        return;
    }
    if (is_hit) {
        onHit(position);
    }
}

Board::Position BotInteligent::randomPosition() {
    std::uniform_int_distribution<> row(0, Board::kBoardSizeRow - 1);
    std::uniform_int_distribution<> col(0, Board::kBoardSizeCol - 1);

    return {row(gen_), col(gen_)};
}

bool BotInteligent::isValidTarget(const Board::Position& pos) const {
    if (pos.first < 0 || pos.second < 0) return false;
    if (static_cast<size_t>(pos.first) >= Board::kBoardSizeRow ||
        static_cast<size_t>(pos.second) >= Board::kBoardSizeCol)
        return false;
    return !tried_[pos.first][pos.second] &&
           Board::Board::is_valid_shot(oponent_board_, pos);
}

void BotInteligent::onHit(const Board::Position& pos) {
    mode_ = Mode::Target;
    enqueueNeighbors(pos);
}

void BotInteligent::onShipSunk() {
    mode_ = Mode::Hunt;
    target_queue_.clear();
}

void BotInteligent::enqueueNeighbors(const Board::Position& pos) {
    static const int dr[] = { -1, 1, 0, 0 };
    static const int dc[] = { 0, 0, -1, 1 };

    for (int i = 0; i < 4; ++i) {
        Board::Position next{
            pos.first + dr[i],
            pos.second + dc[i]
        };

        if (isValidTarget(next)) {
            target_queue_.push_back(next);
        }
    }
}
