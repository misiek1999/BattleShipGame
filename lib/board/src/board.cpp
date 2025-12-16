#include "board.h"
#include "log.h"
#include <ranges>
#include <algorithm>
#include <ostream>

namespace Board {

bool Board::is_full() const noexcept {
    return std::ranges::all_of(board_, [](const auto& row) {
        return std::ranges::all_of(row, [](const auto& board_field) {
            return board_field.field != BoardFieldStatus::kEmpty;
        });
    });
}

bool Board::is_winner() const noexcept {
    const auto is_winner = std::ranges::all_of(ships_count_, [](const auto& ships_count) {
        return ships_count.second == 0;
    });
    return is_winner;
}

inline bool Board::is_valid_shot(Field move) const noexcept {
    return get_shot_result(move) == BoardError::kOk;
}

BoardError Board::get_shot_result(Field move) const noexcept {
    if (move.first >= static_cast<int>(kBoardSizeCol) ||
        move.second >= static_cast<int>(kBoardSizeRow)) {
        return BoardError::kInvalidMove;
    }
    if (const auto field = get_board_field(move);
        field != BoardFieldStatus::kEmpty && field != BoardFieldStatus::kShip) {
        return BoardError::kFieldAlreadyOccupied;
    }
    return BoardError::kOk;
}

std::expected<bool, BoardError> Board::make_shot(Field move) noexcept {
    if (const auto result = get_shot_result(move); result != BoardError::kOk) {
        return std::unexpected(result);
    }
    const auto field = get_board_field(move);

    if (field == BoardFieldStatus::kShip) {
        set_board_field(move, BoardFieldStatus::kShot);
        ShipId ship_id = get_ship_id_at_field(move);
        if (ship_id == kInvalidShipId) {
            LOG_E("Invalid ship ID at field ({}, {})", move.first, move.second);
            return std::unexpected(BoardError::kInvalidShip);
        }
        ShipType ship_type = ships_info_.at(ship_id).type;
        --ships_count_[ship_type];
        LOG_V("Ship hit at field ({}, {}), ship ID: {}, remaining count of type {}: {}",
              move.first, move.second, ship_id, static_cast<int>(ship_type), ships_count_[ship_type]);
        return true;
    } else {
        set_board_field(move, BoardFieldStatus::kMiss);
        return false;
    }
}

bool Board::is_valid_ship_position(Field move, ShipType ship_type, bool is_vertical) const {
    size_t ship_size = 0;
    try {
        ship_size = get_ship_size(ship_type);
    } catch (const std::invalid_argument& e) {
        LOG_E("Exception catched: {}", e.what());
        return false; // Invalid ship type
    }
    for (size_t i = 0; i < ship_size; ++i) {
        Field current_field = is_vertical ? Field{move.first, move.second + static_cast<int>(i)}
                                          : Field{move.first + static_cast<int>(i), move.second};
        if (current_field.first >= static_cast<int>(kBoardSizeCol) ||
            current_field.second >= static_cast<int>(kBoardSizeRow)) {
            return false; // Out of bounds
        }
        if (get_board_field(current_field) != BoardFieldStatus::kEmpty) {
            return false; // Field already occupied
        }
    }
    return true;
}

std::expected<bool, BoardError> Board::place_ship(Field move, ShipType ship_type, bool is_vertical) noexcept {
    if (!is_valid_ship_position(move, ship_type, is_vertical)) {
        LOG_E("Invalid ship position at field ({}, {}) for ship type {}, vertical: {}", move.first, move.second, static_cast<int>(ship_type), is_vertical);
        return std::unexpected(BoardError::kInvalidShipPosition);
    }

    ShipId new_ship_id = generate_ship_id();
    ships_info_[new_ship_id] = ShipInfo{ship_type, {static_cast<size_t>(move.second), static_cast<size_t>(move.first)}, is_vertical, new_ship_id};
    ++ships_count_[ship_type];

    size_t ship_size = static_cast<size_t>(ship_type);
    for (size_t i = 0; i < ship_size; ++i) {
        Field current_field = is_vertical ? Field{move.first, move.second + static_cast<int>(i)}
                                          : Field{move.first + static_cast<int>(i), move.second};
        set_board_field(current_field, BoardFieldStatus::kShip);
        board_[current_field.second][current_field.first].ship_id = new_ship_id;
    }
    LOG_D("Placed ship of type {} at field ({}, {}), vertical: {}, ship ID: {}",
          static_cast<int>(ship_type), move.first, move.second, is_vertical, new_ship_id);
    return true;
}

size_t Board::get_board_size_row() const noexcept {
    return kBoardSizeRow;
}

size_t Board::get_board_size_col() const noexcept {
    return kBoardSizeCol;
}

size_t Board::get_ships_count(ShipType ship_type) const noexcept {
    return ships_count_.at(ship_type);
}

size_t Board::get_all_ships_count() const noexcept {
    size_t total = 0;
    for (const auto& [ship_type, count] : ships_count_) {
        total += count;
    }
    return total;
}

std::unordered_map<ShipType, size_t> Board::get_ships_count_map() const noexcept {
    return ships_count_;
}

bool Board::has_all_ships_deployed() const noexcept {
    size_t total_ships = 0;
    for (const auto& [ship_type, count] : ships_count_) {
        total_ships += count;
    }
    return total_ships == kTotalShipsCount;
}

BoardFieldStatus Board::get_board_field(const Field& field) const noexcept {
    return board_[field.second][field.first].field;
}

void Board::set_board_field(const Field& field, BoardFieldStatus field_type) noexcept {
    board_[field.second][field.first].field = field_type;
}

void Board::reset() {
    board_.fill({});
    ships_count_.clear();
    ships_info_.clear();
    ship_id_counter_ = 0;
}

BoardType Board::get_board() const noexcept {
    return board_;
}

BoardType Board::get_board_without_ships() const noexcept {
    BoardType board_copy = board_;
    for (auto& row : board_copy) {
        for (auto& field : row) {
            if (field.field == BoardFieldStatus::kShip) {
                field.field = BoardFieldStatus::kEmpty;
                field.ship_id = std::nullopt;
            }
        }
    }
    return board_copy;
}

ShipId Board::get_ship_id_at_field(const Field& field) const noexcept {
    const auto& board_field = board_[field.second][field.first];
    if (board_field.ship_id.has_value()) {
        return board_field.ship_id.value();
    }
    return kInvalidShipId; // or some other invalid value
}

ShipId Board::generate_ship_id() noexcept {
    return ship_id_counter_++;
}

BoardTypeAscii board_to_ascii(const BoardType &board) noexcept {
    BoardTypeAscii ascii_board;
    for (size_t row = 0; row < kBoardSizeRow; ++row) {
        for (size_t col = 0; col < kBoardSizeCol; ++col) {
            ascii_board[row][col] = get_board_field_char(board[row][col].field);
        }
    }
    return ascii_board;
}

} // namespace Board
