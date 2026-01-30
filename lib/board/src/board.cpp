#include "board.h"
#include "log.h"
#include <ranges>
#include <algorithm>
#include <ostream>
#include <format>

namespace Board {

bool Board::is_full() const noexcept {
    return std::ranges::all_of(board_, [](const auto& row) {
        return std::ranges::all_of(row, [](const auto& board_field) {
            return board_field.field != BoardFieldStatus::Empty;
        });
    });
}

bool Board::all_ships_destroyed() const noexcept {
    const auto all_ships_destroyed = std::ranges::all_of(ships_count_, [](const auto& ships_count) {
        return ships_count == 0;
    });
    return all_ships_destroyed;
}

inline bool Board::is_valid_shot(Position move) const noexcept {
    return get_shot_result(move) == BoardError::Ok;
}

BoardError Board::get_shot_result(Position move) const noexcept {
    if (move.first >= static_cast<int>(kBoardSizeCol) ||
        move.second >= static_cast<int>(kBoardSizeRow)) {
        return BoardError::InvalidMove;
    }
    if (const auto field = get_board_field(move);
        field != BoardFieldStatus::Empty && field != BoardFieldStatus::Ship) {
        return BoardError::FieldAlreadyOccupied;
    }
    return BoardError::Ok;
}

std::expected<ShotResult, BoardError> Board::make_shot(Position move) noexcept {
    if (const auto result = get_shot_result(move); result != BoardError::Ok) {
        return std::unexpected(result);
    }
    const auto field = get_board_field(move);
    if (field == BoardFieldStatus::Ship) {
        set_board_field(move, BoardFieldStatus::Shot);
        ShipId ship_id = get_ship_id_at_field(move);
        if (ship_id == kInvalidShipId) {
            LOG_E("Invalid ship ID at field ({}, {})", move.first, move.second);
            return std::unexpected(BoardError::InvalidShip);
        }
        ShipType ship_type = ships_info_[ship_id].type;
        ships_info_[ship_id].hits_count++;
        const auto remaining_hits = get_ship_size(ship_type) - ships_info_[ship_id].hits_count;
        LOG_V("Ship hit at field ({}, {}), ship ID: {}, remaining hit count: {}",
              move.first, move.second, ship_id, remaining_hits);
        if (remaining_hits == 0) {
            --ships_count_[static_cast<size_t>(ship_type)];
            LOG_I("Ship of type {} sunk! Ship ID: {}, remaining ships count: {}",
                   ship_type_to_string(ship_type), ship_id, ships_count_[static_cast<size_t>(ship_type)]);
            return ShotResult::ShipDestroyed;
        }
        return ShotResult::Hit;
    } else {
        LOG_V("Shot missed at field ({}, {})", move.first, move.second);
        set_board_field(move, BoardFieldStatus::Miss);
        return ShotResult::Miss;
    }
}

bool Board::is_valid_ship_position(Position move, ShipType ship_type, bool is_vertical) const {
    size_t ship_size = 0;
    // Try to get ship size, catch invalid argument exception
    try {
        ship_size = get_ship_size(ship_type);
    } catch (const std::invalid_argument& e) {
        LOG_E("Exception catched: {}", e.what());
        return false; // Invalid ship type
    }

    // Check number of ships of this type
    if (get_ships_count(ship_type) >= get_max_ship_count(ship_type)) {
        LOG_V("Maximum number of ships of type {} already placed, max allowed ships count {}",
                ship_type_to_string(ship_type), get_max_ship_count(ship_type));
        return false;
    }

    // Check if input position is non negative
    if (move.first < 0 || move.second < 0) {
        LOG_V("Negative ship coordinates are invalid: ({}, {})", move.first, move.second);
        return false;
    }

    // Check if ship fits in the board and doesn't overlap with existing ships
    for (size_t i = 0; i < ship_size; ++i) {
        Position current_field = !is_vertical ? Position{move.first, move.second + static_cast<int>(i)}
                                          : Position{move.first + static_cast<int>(i), move.second};
        if (current_field.first >= static_cast<int>(kBoardSizeCol) ||
            current_field.second >= static_cast<int>(kBoardSizeRow)) {
            LOG_V("Ship of type {} does not fit on the board at position ({}, {}), vertical: {}",
                  ship_type_to_string(ship_type), current_field.first, current_field.second, is_vertical);
            return false; // Out of bounds
        }
        const auto board_field = get_board_field(current_field);
        if (board_field != BoardFieldStatus::Empty) {
            LOG_V("Position ({}, {}) is already occupied {}, cannot place ship of type {}",
                  current_field.first, current_field.second, static_cast<int>(board_field), ship_type_to_string(ship_type));
            return false; // Position already occupied
        }

        // ensure surrounding cells (including diagonals) are empty so ships don't touch
        const int col = current_field.first;
        const int row = current_field.second;
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue; // skip current
                int ncol = col + dc;
                int nrow = row + dr;
                if (ncol < 0 || nrow < 0 ||
                    ncol >= static_cast<int>(kBoardSizeCol) || nrow >= static_cast<int>(kBoardSizeRow)) {
                    continue;
                }
                if (get_board_field(Position{ncol, nrow}) == BoardFieldStatus::Ship) {
                    LOG_V("Adjacent ship found at ({}, {}) when placing ship of type {} at ({}, {}), invalid position",
                          ncol, nrow, ship_type_to_string(ship_type), move.first, move.second);
                    return false; // adjacent ship found — invalid position
                }
            }
        }
    }
    return true;
}

BoardError Board::place_ship(Position move, ShipType ship_type, bool is_vertical) noexcept {
    if (!is_valid_ship_position(move, ship_type, is_vertical)) {
        LOG_E("Invalid ship position at field ({}, {}) for ship type {}, vertical: {}", move.first, move.second, ship_type_to_string(ship_type), is_vertical);
        return BoardError::InvalidShipPosition;
    }

    ShipId new_ship_id = generate_ship_id();
    ships_info_[new_ship_id] = ShipInfo{ship_type, {static_cast<size_t>(move.first), static_cast<size_t>(move.second)}, is_vertical, new_ship_id};
    ++ships_count_[static_cast<size_t>(ship_type)];

    size_t ship_size = get_ship_size(ship_type);
    for (size_t i = 0; i < ship_size; ++i) {
        Position current_field = !is_vertical ? Position{move.first, move.second + static_cast<int>(i)}
                                          : Position{move.first + static_cast<int>(i), move.second};
        set_board_field(current_field, BoardFieldStatus::Ship);
        board_[current_field.first][current_field.second].ship_id = new_ship_id;
    }
    LOG_D("Placed ship of type {} at field ({}, {}), vertical: {}, ship ID: {}",
          ship_type_to_string(ship_type), move.first, move.second, is_vertical, new_ship_id);
    return BoardError::Ok;
}

size_t Board::get_board_size_row() const noexcept {
    return kBoardSizeRow;
}

size_t Board::get_board_size_col() const noexcept {
    return kBoardSizeCol;
}

size_t Board::get_ships_count(ShipType ship_type) const noexcept {
    return ships_count_[static_cast<size_t>(ship_type)];
}

size_t Board::get_all_ships_count() const noexcept {
    size_t total = 0;
    for (const auto& count : ships_count_) {
        total += count;
    }
    return total;
}

ShipCountMap Board::get_ships_count_map() const noexcept {
    return ships_count_;
}

bool Board::has_all_ships_deployed() const noexcept {
    size_t total_ships = 0;
    for (const auto& count : ships_count_) {
        total_ships += count;
    }
    return total_ships == kTotalShipsCount;
}

BoardFieldStatus Board::get_board_field(const Position& field) const noexcept {
    return board_[field.first][field.second].field;
}

void Board::set_board_field(const Position& field, BoardFieldStatus field_type) noexcept {
    board_[field.first][field.second].field = field_type;
}

void Board::reset() {
    board_.fill({});
    ships_count_.fill(0);
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
            if (field.field == BoardFieldStatus::Ship) {
                field.field = BoardFieldStatus::Empty;
                field.ship_id = std::nullopt;
            }
        }
    }
    return board_copy;
}

ShipId Board::get_ship_id_at_field(const Position& field) const noexcept {
    const auto& board_field = board_[field.first][field.second];
    if (board_field.ship_id.has_value()) {
        return board_field.ship_id.value();
    }
    return kInvalidShipId; // or some other invalid value
}

ShipId Board::generate_ship_id() noexcept {
    return ship_id_counter_++;
}

BoardTypeAscii board_to_ascii_array(const BoardType &board) noexcept {
    BoardTypeAscii ascii_board;
    for (size_t row = 0; row < kBoardSizeRow; ++row) {
        for (size_t col = 0; col < kBoardSizeCol; ++col) {
            ascii_board[row][col] = get_board_field_char(board[row][col].field);
        }
    }
    return ascii_board;
}

std::string board_to_ascii_string(const BoardType &board) noexcept {
    std::string ascii_board;
    for (size_t row = 0; row < kBoardSizeRow; ++row) {
        for (size_t col = 0; col < kBoardSizeCol; ++col) {
            ascii_board += get_board_field_char(board[row][col].field);
            ascii_board += " ";
        }
        ascii_board += '\n';
    }
    return ascii_board;
}

std::string ship_count_to_string(const ShipCountMap &ship_count) {
    std::string result;
    bool first = true;

    for (size_t i = 0; i < ship_count.size(); ++i) {
        if (!first) {
            result += ", ";
        }
        first = false;

        const auto type = static_cast<ShipType>(i);
        result += std::format("{}: {}",
                              ship_type_to_string(type),
                              ship_count[i]);
    }

    return result;
}

} // namespace Board
