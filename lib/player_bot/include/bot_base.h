#pragma once

#include <unordered_map>
#include <mutex>
#include "player_interface.h"
#include "game_player_action.h"
#include "game_engine.h"
#include "player_type.h"
#include "board.h"
#include "bot_interface.h"


/// @brief Base class for Player Bots
/// @note All bot classes should inherit from this class and implement specific bot logic
class PlayerBotBase : public IPlayerBot {
protected:
    PlayerBotBase(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type);
public:
    virtual ~PlayerBotBase() override;

    // IPlayer interface implementation
    virtual PlayerType getPlayerType() const override;

    virtual bool isReady() const override;

    virtual bool isConnected() const override;

    virtual Board::BoardType getBoard() const override;

    // IPlayerCallbacks interface implementation
    virtual void onRequestResult(const RequestId req_id, const GameSession::RequestResult result) override;

    virtual void onPlayerShotResult(const Board::Position& position, const bool is_hit,
        const bool is_ship_sunk) override;

    virtual void onOpponentShotResult(const Board::Position& position, const bool is_hit,
                              const bool is_ship_sunk) override;

    virtual void onGameFinished() override;

    virtual void onRoundEnded(const GameEngine::RoundResult round_result) override;

    virtual void onScoreUpdated(const int player_score, const int opponent_score) override;

    virtual void onBoardReceived(const Board::BoardType board) override;

    virtual void onOponentBoardReceived(const Board::BoardType board) override;

    virtual void onShipsCountReceived(const Board::ShipCountMap ships_count) override;

    virtual void onOponentShipsCountReceived(const Board::ShipCountMap oponent_ships_count) override;

    virtual void onPlayerTurnNotify() override;

    virtual void onNewPlayerTurnReceived(const PlayerType player_turn) override;

    virtual void onGameStatusReceived(const GameEngine::GameStatus game_status) override;

    virtual void onCallbackActivation() override;
protected:
    enum class RequesBotType {
        PlaceShip,
        MakeShot,
    };
    std::shared_ptr<GameSession::IGamePlayerAction> action_interface_;
    PlayerType player_type_;
    Board::BoardType board_ = {};
    GameEngine::GameStatus current_game_status_ = GameEngine::GameStatus::NotStarted;
    Board::ShipCountMap bot_ships_count_ = {};
    Board::ShipCountMap oponent_ships_count_ = {};
    Board::ShipCountMap available_bot_ships_ = {};
    std::mutex mutex_;
    std::unordered_map<RequestId, RequesBotType> request_map_ = {};

    virtual void placeShipsHelper();

    virtual void makeShotHelper();

    /// @brief This function should be implemented in child for place ship
    /// @param ship_type ship which should be placed by bot
    /// @return request id returned from the server for specific request
    virtual RequestId placeShip(const Board::ShipType ship_type) = 0;

    /// @brief This function should be implemented in child for make a shot
    /// @return request id returned from the server for specific request
    virtual RequestId makeShot() = 0;
};
