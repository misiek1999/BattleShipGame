#include "bot_random.h"
#include "log.h"
#include "board.h"

BotRandom::BotRandom(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
        const PlayerType player_type)
        : PlayerBotBase(action_interface, player_type),
          action_interface_(action_interface),
          gen_(rd_()) {
    LOG_V("Create a random bot");
    const size_t kMinGeneratedNumber= 0U;
    const size_t kMaxGeneratedNumberRot= 1U;

    distrib_pos_col_ = std::uniform_int_distribution<>(kMinGeneratedNumber, Board::kBoardSizeCol);
    distrib_pos_row_ = std::uniform_int_distribution<>(kMinGeneratedNumber, Board::kBoardSizeRow);
    distrib_rot_ = std::uniform_int_distribution<>(kMinGeneratedNumber, kMaxGeneratedNumberRot);
}

RequestId BotRandom::placeShip(const Board::ShipType ship_type) {
    Board::Position pos {0, 0};
    bool rot {false};
    do {
        pos = generateRandomPos();
        rot = generateRandoRot();
    } while (!Board::Board::is_possible_to_place_ship(board_, pos, ship_type, !rot));
    return action_interface_->placeShip(ship_type, pos, rot);
}

RequestId BotRandom::makeShot() {
    Board::Position pos = {0, 0};
    do {
        pos = generateRandomPos();
    } while (!Board::Board::is_valid_shot(oponent_board_, pos));
    return action_interface_->makeShot(pos);
}

Board::Position BotRandom::generateRandomPos() {
    return Board::Position(distrib_pos_row_(gen_), distrib_pos_col_(gen_));
}

bool BotRandom::generateRandoRot() {
    return distrib_rot_(gen_);
}
