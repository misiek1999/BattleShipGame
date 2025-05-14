#pragma once

#include <array>
#include <expected>
#include <memory>
#include <utility>

#include "player_type.h"

namespace Board {
    // Size of the board
    constexpr size_t kBoardSize = 10U;

    // Types of fields in the board
    enum class BoardField : uint8_t {
        EMPTY = 0,
        X = 1,
        O = 2
    };

    // Convert player type to board field
    constexpr BoardField convertPlayerTypeToBoardField(BoardPlayerType player) {
        auto boardField = BoardField::EMPTY;
        switch (player) {
        case BoardPlayerType::X:
            boardField = BoardField::X;
            break;
        case BoardPlayerType::O:
            boardField = BoardField::O;
            break;
        default:
            break;
        }
        return boardField;
    }

    constexpr char convertBoardFieldToChar(BoardField field) {
        char c = ' ';
        switch (field) {
        case BoardField::X:
            c = 'X';
            break;
        case BoardField::O:
            c = 'O';
            break;
        default:
            break;
        }
        return c;
    }

    // Constant for invalid move
    constexpr std::pair<int, int> kInvalidMove = {-1, -1};

    // Type of the board 1. ROW, 2. COL
    using BoardType = std::array<std::array<BoardField, kBoardSize>, kBoardSize>;

    enum class BoardError {
        INVALID_MOVE,
        INVALID_PLAYER,
        INVALID_STATE,
        INVALID_COORDINATES,
        NONE
    };

    class IBoard {
    public:
        virtual ~IBoard() = default;
        virtual BoardType get_board() const = 0;
        virtual bool is_full() const = 0;
        virtual bool is_winner(BoardPlayerType player) const = 0;
        virtual bool is_valid_move(int row, int col) const = 0;
        virtual std::expected<bool, BoardError> make_move(int row, int col, BoardPlayerType player) = 0;
        virtual void reset() = 0;

    };

    // Forward declaration
    class BoardImpl;

    // Board class
    class Board{
    public:
        Board();
        explicit Board(const BoardType& board);
        ~Board() = default;

        BoardType get_board() const {
            return board_impl_->get_board();
        }

        bool is_full() const {
            return board_impl_->is_full();
        }

        bool is_winner(BoardPlayerType player) const {
            return board_impl_->is_winner(player);
        }

        bool is_valid_move(int row, int col) const {
            return board_impl_->is_valid_move(row, col);
        }

        std::expected<bool, BoardError> make_move(int row, int col, BoardPlayerType player) {
            return board_impl_->make_move(row, col, player);
        }

        void reset() {
            board_impl_->reset();
        }

    private:
        std::unique_ptr<IBoard> board_impl_;
    };
}
