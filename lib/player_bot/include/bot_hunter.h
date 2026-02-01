#pragma once

#include <memory>
#include <random>
#include <utility>
#include <deque>
#include "player_interface.h"
#include "game_player_action.h"
#include "game_engine.h"
#include "player_type.h"
#include "board.h"
#include "bot_base.h"

class BotHunter : public PlayerBotBase {
public:
    BotHunter(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
        const PlayerType player_type);
    ~BotHunter() override = default;

protected:
    RequestId placeShip(const Board::ShipType ship_type) override;

    RequestId makeShot() override;

    virtual void onPlayerShotResult(const Board::Position& position, const bool is_hit,
        const bool is_ship_sunk) override;
private:
    enum class Mode {
        Hunt,
        Target
    };

    Mode mode_{Mode::Hunt};
    std::deque<Board::Position> target_queue_;
    std::vector<std::vector<bool>> tried_;

    std::random_device rd_;
    std::mt19937 gen_;

    Board::Position randomPosition();
    bool isValidTarget(const Board::Position& pos) const;
    void onHit(const Board::Position& pos);
    void onShipSunk();
    void enqueueNeighbors(const Board::Position& pos);
};
