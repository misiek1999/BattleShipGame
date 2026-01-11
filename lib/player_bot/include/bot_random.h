#pragma once

#include <memory>
#include <random>
#include <utility>
#include "player_interface.h"
#include "game_player_action.h"
#include "game_engine.h"
#include "player_type.h"
#include "board.h"
#include "bot_base.h"

class BotRandom : public PlayerBotBase {
public:
    BotRandom(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
        const PlayerType player_type);
    ~BotRandom() override = default;

protected:
    RequestId placeShip(const Board::ShipType ship_type) override;

    RequestId makeShot() override;
private:
    std::shared_ptr<GameSession::IGamePlayerAction> action_interface_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> distrib_pos_col_;
    std::uniform_int_distribution<> distrib_pos_row_;
    std::uniform_int_distribution<> distrib_rot_;

    Board::Position generateRandomPos();
    bool generateRandoRot();
};
