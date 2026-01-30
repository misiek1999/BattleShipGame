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

using BoardPlayerMap = std::unordered_map<PlayerType, BoardPlayerData>;

class GameEngineImpl : public IGameEngine {
public:
    // GameEngineImpl::GameEngineImpl()
    //     : boards_() {
    // }

    Board::BoardType getBoard(const PlayerType player) const override {
        return boards_.at(player).board.get_board();
    }

    Board::BoardType getOponentBoard(const PlayerType player) const override {
        PlayerType opponent = get_opponent_player(player);
        return boards_.at(opponent).board.get_board_without_ships();
    }

    Board::ShipCountMap getPlayerShipsCount(const PlayerType player) const override {
        if (boards_.find(player) == boards_.end()) {
            LOG_E("Requested ship count for invalid player: {}", static_cast<int>(player));
            return {};
        }
        return boards_.at(player).board.get_ships_count_map();
    }

    std::pair<int, int> getScore() const override {
        if (boards_.find(PlayerType::Player_1) != boards_.end() &&
            boards_.find(PlayerType::Player_2) != boards_.end()) {
            return {boards_.at(PlayerType::Player_1).score,
                    boards_.at(PlayerType::Player_2).score};
        }
        LOG_E("Scores requested but one or both players not found");
        return {0, 0};
    }

    void resetGame() override {
        resetBoards();
        for (auto& [player, data] : boards_) {
            data.score = 0;
        }
        current_turn_player_ = PlayerType::Player_1;
        LOG_D("Game has been reset");
    }

    void resetBoards() override {
        for (auto& [player, data] : boards_) {
            data.board.reset();
        }
        game_status_ = GameStatus::PreparingBoards;
        LOG_D("Boards have been reset");
    }

    std::expected<PlayerType, GameEngineError> getCurrentTurnPlayer() const override {
        return current_turn_player_;
    }

    GameStatus getGameStatus() const override {
        return game_status_;
    }

    GameEngineError setPlayerShip(const PlayerType player, const Board::ShipType ship_type,
                               const Board::Position& position, bool is_horizontal) override {
        if (game_status_ == GameStatus::RoundInProgress) {
            LOG_W("Cannot place ship, game already in progress");
            return GameEngineError::RoundFinished;
        }

        if (game_status_ != GameStatus::PreparingBoards) {
            LOG_W("Cannot place ship, game not in preparation phase");
            return GameEngineError::GameNotStarted;
        }

        const auto is_vertical = !is_horizontal;
        auto& player_board = boards_[player].board;
        // Check if player board has all ships deployed
        if (player_board.has_all_ships_deployed()) {
            LOG_W("{} has already deployed all ships", board_player_type_to_string(player));
            return GameEngineError::BoardIsAlreadyPrepared;
        }

        auto place_result = player_board.place_ship(position, ship_type, is_vertical);
        if (place_result != Board::BoardError::Ok) {
            LOG_W("Failed to place ship for {} at position ({}, {}), vertical: {}. Error: {}",
                  board_player_type_to_string(player), position.first, position.second, is_vertical,
                  static_cast<int>(place_result));
            return GameEngineError::InvalidShipPosition;
        }
        LOG_I("{} placed a ship of type {} at position ({}, {}), vertical: {}",
            board_player_type_to_string(player), ship_type_to_string(ship_type),
            position.first, position.second, is_vertical);

        // Check if both players have deployed all ships
        if (boards_.at(PlayerType::Player_1).board.has_all_ships_deployed() &&
            boards_.at(PlayerType::Player_2).board.has_all_ships_deployed()) {
            game_status_ = GameStatus::RoundInProgress;
            LOG_I("Both players have deployed all ships. Game is starting!");
        }

        return GameEngineError::Ok;
    }

    GameEngineError setPlayerShot(const PlayerType player, const Board::Position& position, bool& was_hitted, bool& was_ship_destroyed) override {
        was_hitted = false;
        if (game_status_ == GameStatus::RoundFinished) {
            LOG_W("Cannot make a shot, game is already finished");
            return GameEngineError::RoundFinished;
        }

        if (game_status_ != GameStatus::RoundInProgress) {
            LOG_W("Cannot make a shot, game is not in progress");
            return GameEngineError::GameNotStarted;
        }

        if (player != current_turn_player_) {
            LOG_W("It's not {}'s turn", board_player_type_to_string(player));
            return GameEngineError::InvalidPlayerTurn;
        }

        PlayerType opponent = get_opponent_player(player);
        auto& opponent_board = boards_.at(opponent).board;
        auto shot_result = opponent_board.make_shot(position);
        if (!shot_result) {
            LOG_E("Shot by {} at position ({}, {}) failed: {}",
                  board_player_type_to_string(player), position.first, position.second,
                  static_cast<int>(shot_result.error()));
            return GameEngineError::InvalidShot;
        } else {
            switch (shot_result.value())
            {
            case Board::ShotResult::Hit:
                was_hitted = true;
                LOG_I("{} scored a hit at position ({}, {})! New score: {}",
                      board_player_type_to_string(player), position.first, position.second,
                      boards_.at(player).score);
                break;
            case Board::ShotResult::ShipDestroyed:
                was_hitted = true;
                was_ship_destroyed = true;
                LOG_I("{} destroyed a ship at position ({}, {})! New score: {}",
                      board_player_type_to_string(player), position.first, position.second,
                      boards_.at(player).score);
                break;
            case Board::ShotResult::Miss:
                LOG_I("{} missed at position ({}, {})",
                      board_player_type_to_string(player), position.first, position.second);
                break;
            default:
                throw std::logic_error("Unhandled ShotResult value");
                break;
            }
        }

        const auto is_winner = opponent_board.all_ships_destroyed();
        if (is_winner) {
            boards_.at(player).score += 1;
            game_status_ = GameStatus::RoundFinished;
            LOG_I("{} has won the game!", board_player_type_to_string(player));
            round_result_ = (player == PlayerType::Player_1) ? RoundResult::Player1Won : RoundResult::Player2Won;
        } else {
            current_turn_player_ = opponent;
            LOG_D("Turn changed to {}", board_player_type_to_string(current_turn_player_));
        }

        return GameEngineError::Ok;
    }

    RoundResult getRoundResult() const override {
        if (game_status_ == GameStatus::NotStarted) {
            return RoundResult::GameNotStarted;
        }
        if (game_status_ == GameStatus::PreparingBoards) {
            return RoundResult::RoundInProgress;
        }
        if (game_status_ == GameStatus::GameEnded) {
            return RoundResult::GameAborted;
        }
        return round_result_;
    }

    void stopGame() override {
        game_status_ = GameStatus::GameEnded;
    }

private:
    BoardPlayerMap boards_{
        { PlayerType::Player_1, BoardPlayerData{} },
        { PlayerType::Player_2, BoardPlayerData{} }
    };
    PlayerType current_turn_player_{PlayerType::Player_1};
    GameStatus game_status_{GameStatus::PreparingBoards};
    RoundResult round_result_{RoundResult::GameNotStarted};
};

GameEngine::GameEngine() : impl_(std::make_unique<GameEngineImpl>()) {
    impl_->resetGame();
}
}   // namespace GameEngine
