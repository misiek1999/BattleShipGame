#pragma once

#include <memory>
#include <utility>
#include <expected>
#include "board.h"
#include "player_type.h"

namespace GameEngine {

enum class GameEngineError {
    Ok,
    InvalidShipPosition,
    InvalidShot,
    InvalidPlayer,
    InvalidPlayerTurn,
    InvalidShipType,
    BoardIsAlreadyPrepared,
    GameFinished,
    GameNotStarted
};

enum class GameStatus {
    NotStarted,
    PreparingBoards,
    GameInProgress,
    GameFinished
};

enum class RoundResult {
    Player1Won,
    Player2Won,
    Draw,
    GameInProgress,
    GameNotStarted,
    GameAborted
};

class IGameEngine {
public:
    virtual ~IGameEngine() = default;

    /// @brief Get the current state of the board
    virtual void resetGame() = 0;

    /// @brief Reset the board to its initial state
    virtual void resetBoards() = 0;

    /// @brief Get the current state of the board
    /// @param player The player for whom to get the board state
    /// @return The board state as a 2D array
    virtual Board::BoardType getBoard(const PlayerType player) const = 0;

    /// @brief Get the opponent's board state
    /// @param player The player whose opponent's board to get
    /// @return The opponent's board state as a 2D array
    virtual Board::BoardType getOponentBoard(const PlayerType player) const = 0;

    /// @brief Get the count of remaining ships for the specified player
    /// @param player The player whose ships count to get
    /// @return A map containing the ship type as key and the count of remaining ships as value
    virtual Board::ShipCountMap getPlayerShipsCount(const PlayerType player) const = 0;

    /// @brief Get the current score of both players
    /// @return A pair containing the scores of Player 1 and Player 2
    virtual std::pair<int, int> getScore() const  = 0;

    /// @brief Get the player whose turn it is currently
    /// @return The type of the player whose turn it is
    virtual std::expected<PlayerType, GameEngineError> getCurrentTurnPlayer() const = 0;

    /// @brief Get the current status of the game
    /// @return The current game status
    virtual GameStatus getGameStatus() const = 0;

    /// @brief Place a ship for the specified player at the given position
    /// @param player The player placing the ship
    /// @param ship_type The type of ship to place
    /// @param position The position where the ship should be placed
    /// @param is_horizontal Whether the ship is placed horizontally or vertically
    virtual GameEngineError setPlayerShip(const PlayerType player, const Board::ShipType ship_type,
                               const Board::Position& position, bool is_horizontal) = 0;

    /// @brief Make a shot for the specified player at the given position
    /// @param player The player making the shot
    /// @param position The position where the shot is made
    /// @param was_hitted Reference to a boolean indicating if the shot hit a ship
    /// @param was_ship_destroyed Reference to a boolean indicating if the shot destroyed a ship
    /// @return The result of the game processing
    virtual GameEngineError setPlayerShot(const PlayerType player, const Board::Position& position, bool& was_hitted, bool& was_ship_destroyed) = 0;

    /// @brief Get the result of the current round
    /// @return The result of the round
    virtual RoundResult getRoundResult() const = 0;
};

class GameEngineImpl;

class GameEngine : public IGameEngine {

public:
    GameEngine();
    ~GameEngine() = default;

    Board::BoardType getBoard(const PlayerType player) const override {
        return impl_->getBoard(player);
    }

    Board::BoardType getOponentBoard(const PlayerType player) const override {
        return impl_->getOponentBoard(player);
    }

    Board::ShipCountMap getPlayerShipsCount(const PlayerType player) const override {
        return impl_->getPlayerShipsCount(player);
    }

    std::pair<int, int> getScore() const override {
        return impl_->getScore();
    }

    void resetGame() override {
        impl_->resetGame();
    }

    void resetBoards() override {
        impl_->resetBoards();
    }

    std::expected<PlayerType, GameEngineError> getCurrentTurnPlayer() const override {
        return impl_->getCurrentTurnPlayer();
    }

    GameStatus getGameStatus() const override {
        return impl_->getGameStatus();
    }

    GameEngineError setPlayerShip(const PlayerType player, const Board::ShipType ship_type,
                               const Board::Position& position, bool is_horizontal) override {
        return impl_->setPlayerShip(player, ship_type, position, is_horizontal);
    }

    GameEngineError setPlayerShot(const PlayerType player, const Board::Position& position, bool& was_hitted, bool& was_ship_destroyed) override {
        return impl_->setPlayerShot(player, position, was_hitted, was_ship_destroyed);
    }

    RoundResult getRoundResult() const override {
        return impl_->getRoundResult();
    }

private:
    std::unique_ptr<IGameEngine> impl_;
};


// Helper functions
constexpr const char* to_cstring(const GameEngineError error)
{
    switch (error)
    {
        case GameEngineError::Ok:                     return "Ok";
        case GameEngineError::InvalidShipPosition:    return "InvalidShipPosition";
        case GameEngineError::InvalidShot:            return "InvalidShot";
        case GameEngineError::InvalidPlayer:          return "InvalidPlayer";
        case GameEngineError::InvalidPlayerTurn:      return "InvalidPlayerTurn";
        case GameEngineError::InvalidShipType:        return "InvalidShipType";
        case GameEngineError::BoardIsAlreadyPrepared: return "BoardIsAlreadyPrepared";
        case GameEngineError::GameFinished:           return "GameFinished";
        case GameEngineError::GameNotStarted:         return "GameNotStarted";
        default:
            break;
    }
    assert(false && "Unknown GameActionType");
    return "Unknown";   // We should enter here in runtime
}

} // namespace GameEngine