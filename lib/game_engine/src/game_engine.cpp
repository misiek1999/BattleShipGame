#include "game_engine.h"
#include "log.h"
#include "board.h"
#include "player_type.h"
#include <expected>
#include <memory>
#include <utility>
#include <unordered_map>

namespace GameEngine {

struct BoardPlayerData {
    Board::Board board;
    int score = 0;
};

using BoardPlayerMap = std::unordered_map<BoardPlayerType, BoardPlayerData>;

class GameEngineImpl : public IGameEngine {
public:

    Board::BoardType getBoard(const BoardPlayerType player) const override {
        return boards_.at(player).board.get_board();
    }

    Board::BoardType getOponentBoard(const BoardPlayerType player) const override {
        BoardPlayerType opponent = get_opponent_player(player);
        return boards_.at(opponent).board.get_board_without_ships();
    }

    Board::ShipCountMap getPlayerShipsCount(const BoardPlayerType player) const override {
        if (boards_.find(player) == boards_.end()) {
            LOG_E("Requested ship count for invalid player: {}", static_cast<int>(player));
            return {};
        }
        return boards_.at(player).board.get_ships_count_map();
    }

    std::pair<int, int> getScore() const override {
        if (boards_.find(BoardPlayerType::Player_1) != boards_.end() &&
            boards_.find(BoardPlayerType::Player_2) != boards_.end()) {
            return {boards_.at(BoardPlayerType::Player_1).score,
                    boards_.at(BoardPlayerType::Player_2).score};
        }
        LOG_E("Scores requested but one or both players not found");
        return {0, 0};
    }

    void resetGame() override {
        for (auto& [player, data] : boards_) {
            data.board.reset();
            data.score = 0;
        }
        current_turn_player_ = BoardPlayerType::Player_1;
        game_status_ = GameStatus::kPreparingBoards;
        LOG_D("Game has been reset");
    }

    void resetBoards() override {
        for (auto& [player, data] : boards_) {
            data.board.reset();
        }
        LOG_D("Boards have been reset");
    }

    std::expected<BoardPlayerType, GameEngineError> getCurrentTurnPlayer() const override {
        return current_turn_player_;
    }

    GameStatus getGameStatus() const override {
        return game_status_;
    }

    GameEngineError setPlayerShip(const BoardPlayerType player, const Board::ShipType ship_type,
                               const Board::Field& position, bool is_horizontal) override {
        if (game_status_ == GameStatus::kGameInProgress) {
            LOG_W("Cannot place ship, game already in progress");
            return GameEngineError::kGameFinished;
        }

        if (game_status_ != GameStatus::kPreparingBoards) {
            LOG_W("Cannot place ship, game not in preparation phase");
            return GameEngineError::kGameNotStarted;
        }

        auto& player_board = boards_[player].board;
        // Check if player board has all ships deployed
        if (player_board.has_all_ships_deployed()) {
            LOG_W("{} has already deployed all ships", board_player_type_to_string(player));
            return GameEngineError::kBoardIsAlreadyPrepared;
        }

        auto place_result = player_board.place_ship(position, ship_type, !is_horizontal);
        if (place_result != Board::BoardError::kOk) {
            LOG_W("Failed to place ship for {} at position ({}, {}), horizontal: {}. Error: {}",
                  board_player_type_to_string(player), position.first, position.second, is_horizontal,
                  static_cast<int>(place_result));
            return GameEngineError::kInvalidShipPosition;
        }
        LOG_I("{} placed a ship of type {} at position ({}, {}), horizontal: {}",
            board_player_type_to_string(player), ship_type_to_string(ship_type),
            position.first, position.second, is_horizontal);

        // Check if both players have deployed all ships
        if (boards_.at(BoardPlayerType::Player_1).board.has_all_ships_deployed() &&
            boards_.at(BoardPlayerType::Player_2).board.has_all_ships_deployed()) {
            game_status_ = GameStatus::kGameInProgress;
            LOG_I("Both players have deployed all ships. Game is starting!");
        }

        return GameEngineError::kOK;
    }

    GameEngineError setPlayerShot(const BoardPlayerType player, const Board::Field& position) override {
        if (game_status_ == GameStatus::kGameFinished) {
            LOG_W("Cannot make a shot, game is already finished");
            return GameEngineError::kGameFinished;
        }

        if (game_status_ != GameStatus::kGameInProgress) {
            LOG_W("Cannot make a shot, game is not in progress");
            return GameEngineError::kGameNotStarted;
        }

        if (player != current_turn_player_) {
            LOG_W("It's not {}'s turn", board_player_type_to_string(player));
            return GameEngineError::kInvalidPlayerTurn;
        }

        BoardPlayerType opponent = get_opponent_player(player);
        auto& opponent_board = boards_.at(opponent).board;
        auto shot_result = opponent_board.make_shot(position);
        if (!shot_result) {
            LOG_E("Shot by {} at position ({}, {}) failed: {}",
                  board_player_type_to_string(player), position.first, position.second,
                  static_cast<int>(shot_result.error()));
            return GameEngineError::kInvalidShot;
        } else {
            if (shot_result.value()) {
                boards_.at(player).score += 1;
                LOG_I("{} scored a hit at position ({}, {})! New score: {}",
                      board_player_type_to_string(player), position.first, position.second,
                      boards_.at(player).score);
            } else {
                LOG_I("{} missed at position ({}, {})",
                      board_player_type_to_string(player), position.first, position.second);
            }
        }

        const auto is_winner = opponent_board.is_winner();
        if (is_winner) {
            game_status_ = GameStatus::kGameFinished;
            LOG_I("{} has won the game!", board_player_type_to_string(player));
        } else {
            current_turn_player_ = opponent;
            LOG_D("Turn changed to {}", board_player_type_to_string(current_turn_player_));
        }

        return GameEngineError::kOK;
    }


private:
    BoardPlayerMap boards_;
    BoardPlayerType current_turn_player_{BoardPlayerType::Player_1};
    GameStatus game_status_{GameStatus::kPreparingBoards};
};

GameEngine::GameEngine() : impl_(std::make_unique<GameEngineImpl>()) {
    impl_->resetGame();
}
}   // namespace GameEngine
