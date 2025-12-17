#pragma once

#include <memory>
#include <utility>
#include <expected>
#include "board.h"
#include "player_type.h"

namespace GameEngine {

enum class GameEngineError {
    kOK,
    kInvalidShipPosition,
    kInvalidShot,
    kInvalidPlayer,
    kInvalidPlayerTurn,
    kInvalidShipType,
    kBoardIsAlreadyPrepared,
    kGameFinished,
    kGameNotStarted
};

enum class GameStatus {
    kNotStarted,
    kPreparingBoards,
    kGameInProgress,
    kGameFinished
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
    virtual Board::BoardType getBoard(const BoardPlayerType player) const = 0;

    /// @brief Get the opponent's board state
    /// @param player The player whose opponent's board to get
    /// @return The opponent's board state as a 2D array
    virtual Board::BoardType getOponentBoard(const BoardPlayerType player) const = 0;

    /// @brief Get the count of remaining ships for the specified player
    /// @param player The player whose ships count to get
    /// @return A map containing the ship type as key and the count of remaining ships as value
    virtual Board::ShipCountMap getPlayerShipsCount(const BoardPlayerType player) const = 0;

    /// @brief Get the current score of both players
    /// @return A pair containing the scores of Player 1 and Player 2
    virtual std::pair<int, int> getScore() const  = 0;

    /// @brief Get the player whose turn it is currently
    /// @return The type of the player whose turn it is
    virtual std::expected<BoardPlayerType, GameEngineError> getCurrentTurnPlayer() const = 0;

    /// @brief Get the current status of the game
    /// @return The current game status
    virtual GameStatus getGameStatus() const = 0;

    /// @brief Place a ship for the specified player at the given position
    /// @param player The player placing the ship
    /// @param ship_type The type of ship to place
    /// @param position The position where the ship should be placed
    /// @param is_horizontal Whether the ship is placed horizontally or vertically
    virtual GameEngineError setPlayerShip(const BoardPlayerType player, const Board::ShipType ship_type,
                               const Board::Field& position, bool is_horizontal) = 0;

    /// @brief Make a shot for the specified player at the given position
    /// @param player The player making the shot
    /// @param row The row index of the shot
    /// @param col The column index of the shot
    /// @return The result of the game processing
    virtual GameEngineError setPlayerShot(const BoardPlayerType player, const Board::Field& position) = 0;
};

class GameEngineImpl;

class GameEngine : public IGameEngine {

public:
    GameEngine();
    ~GameEngine() = default;

    Board::BoardType getBoard(const BoardPlayerType player) const override {
        return impl_->getBoard(player);
    }

    Board::BoardType getOponentBoard(const BoardPlayerType player) const override {
        return impl_->getOponentBoard(player);
    }

    Board::ShipCountMap getPlayerShipsCount(const BoardPlayerType player) const override {
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

    std::expected<BoardPlayerType, GameEngineError> getCurrentTurnPlayer() const override {
        return impl_->getCurrentTurnPlayer();
    }

    GameStatus getGameStatus() const override {
        return impl_->getGameStatus();
    }

    GameEngineError setPlayerShip(const BoardPlayerType player, const Board::ShipType ship_type,
                               const Board::Field& position, bool is_horizontal) override {
        return impl_->setPlayerShip(player, ship_type, position, is_horizontal);
    }

    GameEngineError setPlayerShot(const BoardPlayerType player, const Board::Field& position) override {
        return impl_->setPlayerShot(player, position);
    }

private:
    std::unique_ptr<IGameEngine> impl_;
};

} // namespace GameEngine