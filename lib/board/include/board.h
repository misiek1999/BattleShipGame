#pragma once

#include <array>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <expected>
#include <utility>
#include <stdexcept>
#include <optional>
#include <string>
#include <functional>

namespace Board {
    // Size of the board
    constexpr size_t kBoardSizeRow = 10U;
    constexpr size_t kBoardSizeCol = 10U;

    // Types of fields in the board
    enum class BoardFieldStatus : uint8_t {
        Empty = 0,  // Empty field
        Ship = 1,   // Player ship
        Shot = 2,   // Shot fired
        Miss = 3,   // Missed shot
    };

    // Types of ships
    enum class ShipType : uint8_t {
        Destroyer  = 0,    // 1 cells
        Submarine  = 1,    // 2 cells
        Cruiser    = 2,    // 3 cells
        Battleship = 3,    // 4 cells
        NumberOfShips,
        Undefined
    };

    constexpr const auto kNumberOfShips = static_cast<size_t>(ShipType::NumberOfShips);

    inline constexpr const char* ship_type_to_string(const ShipType ship_type) {
        switch (ship_type) {
            case ShipType::Destroyer:  return "Destroyer";
            case ShipType::Submarine:  return "Submarine";
            case ShipType::Cruiser:    return "Cruiser";
            case ShipType::Battleship: return "Battleship";
            default: return "Unknown";
        }
    }

    inline constexpr const char* ship_type_to_symbol(const ShipType ship_type) {
        switch (ship_type) {
            case ShipType::Destroyer:  return "D";
            case ShipType::Submarine:  return "S";
            case ShipType::Cruiser:    return "C";
            case ShipType::Battleship: return "B";
            default: return "U";
        }
    }

    // Count of ships
    constexpr size_t kDestroyerMaxCount = 4U;
    constexpr size_t kSubmarineMaxCount = 3U;
    constexpr size_t kCruiserMaxCount   = 2U;
    constexpr size_t kBattleshipMaxCount= 1U;
    constexpr size_t kTotalShipsCount = kDestroyerMaxCount + kSubmarineMaxCount + kCruiserMaxCount + kBattleshipMaxCount;

    constexpr const std::array kMaxAvailableShipsArray {kDestroyerMaxCount, kSubmarineMaxCount, kCruiserMaxCount, kBattleshipMaxCount};

    // Get the count of ships of a specific type
    constexpr size_t get_max_ship_count(ShipType ship_type) {
        switch (ship_type) {
            case ShipType::Destroyer:  return kDestroyerMaxCount;
            case ShipType::Submarine:  return kSubmarineMaxCount;
            case ShipType::Cruiser:    return kCruiserMaxCount;
            case ShipType::Battleship: return kBattleshipMaxCount;
            default: break;
        }
        // Throw an exception if the ship type is not found
        throw std::invalid_argument("Invalid ship type");
    }

    constexpr size_t get_ship_size(ShipType ship_type) {
        switch (ship_type) {
            case ShipType::Destroyer:  return 1U;
            case ShipType::Submarine:  return 2U;
            case ShipType::Cruiser:    return 3U;
            case ShipType::Battleship: return 4U;
            default: break;
        }
        // Throw an exception if the ship type is not found
        throw std::invalid_argument("Invalid ship type");
    }

    // Map of ship types to their char characters, this is used for debug printing
    constexpr char get_board_field_char(BoardFieldStatus field) noexcept {
        switch (field) {
            case BoardFieldStatus::Empty:return ' ';
            case BoardFieldStatus::Ship: return 'P';
            case BoardFieldStatus::Shot: return 'O';
            case BoardFieldStatus::Miss: return 'X';
            default: return '?';
        }
    }

    // Define a move as a pair of integers (ROW, COL)
    using Position = std::pair<int, int>;

    // Constant for invalid move
    constexpr Position InvalidMove = {-1, -1};

    // Error codes for the board
    enum class BoardError {
        Ok = 0,
        InvalidMove,
        InvalidPlayer,
        InvalidShip,
        InvalidShipCount,
        InvalidShipPosition,
        InvalidShipOrientation,
        FieldAlreadyOccupied,
    };

    enum class ShotResult {
        Miss = 0,
        Hit,
        ShipDestroyed
    };

    using ShipId = size_t;

    constexpr ShipId kInvalidShipId = static_cast<ShipId>(-1);

    // Information about a ship on the board
    struct ShipInfo {
        ShipType type;
        std::pair<size_t, size_t> pos; // row, col
        bool is_vertical = true;
        ShipId ship_id;
        size_t hits_count = 0;
    };

    struct BoardField {
        BoardFieldStatus field = BoardFieldStatus::Empty;
        std::optional<ShipId> ship_id = std::nullopt;
    };

    // Type of the board 1. ROW, 2. COL
    using BoardType = std::array<std::array<BoardField, kBoardSizeCol>, kBoardSizeRow>;
    using BoardTypeAscii = std::array<std::array<char, kBoardSizeCol>, kBoardSizeRow>;

    using ShipCountMap = std::array<size_t, kNumberOfShips>;

    BoardTypeAscii board_to_ascii_array(const BoardType& board) noexcept;
    std::string board_to_ascii_string(const BoardType& board) noexcept;
    std::string ship_count_to_string(const ShipCountMap& ship_count);

    class Board{
    public:
        Board() = default;
        ~Board() = default;

        /// @brief Check if the board is full
        /// @return True if the board is full, false otherwise
        bool is_full() const noexcept;

        /// @param player The player to check
        /// @return True if all ships are sunk, false otherwise
        bool all_ships_destroyed() const noexcept;

        /// @brief Check if the move is valid
        /// @param move The move to check
        /// @return True if the shot is valid, false otherwise
        bool is_valid_shot(Position move) const noexcept;

        /// @brief Make a shot on the board
        /// @param move The move to make
        /// @return Shot result if the shot was successful, error code otherwise
        std::expected<ShotResult, BoardError> make_shot(Position move) noexcept;

        /// @brief Check if the ship can be placed on the board
        /// @param move The move to make
        /// @param ship_type The type of the ship
        /// @param is_vertical True if the ship is vertical, false otherwise
        bool is_valid_ship_position(Position move, ShipType ship_type, bool is_vertical) const;

        /// @brief Place a ship on the board
        /// @param move The move to make
        /// @param ship_type The type of the ship
        /// @param is_vertical True if the ship is vertical, false otherwise
        /// @return Error code, Ok if the ship was placed successfully
        BoardError place_ship(Position move, ShipType ship_type, bool is_vertical) noexcept;

        /// @brief Get the board row size
        /// @return The row size of the board
        size_t get_board_size_row() const noexcept;

        /// @brief Get the board column size
        /// @return The column size of the board
        size_t get_board_size_col() const noexcept;

        /// @brief Get the count of ships of a specific type
        /// @param ship_type The type of the ship
        /// @return The count of ships of the specified type
        size_t get_ships_count(ShipType ship_type) const noexcept;

        /// @brief Get the count of all ships on the board
        /// @return The count of all ships on the board
        size_t get_all_ships_count() const noexcept;

        /// @brief Get the map of ships remaining count
        /// @return The map of ships remaining count
        /// @note The map contains the ship type as key and the count of remaining ships as value
        ShipCountMap get_ships_count_map() const noexcept;

        /// @brief Has if all ships are deployed
        /// @return True if all ships are deployed, false otherwise
        bool has_all_ships_deployed() const noexcept;

        /// @brief Reset the board to its initial state
        void reset();

        /// @brief Get the board
        /// @return The board
        BoardType get_board() const noexcept;

        /// @brief Get the board without ships
        /// @return The board without ships
        BoardType get_board_without_ships() const noexcept;

    private:
        BoardType board_;
        ShipCountMap ships_count_ {};
        std::unordered_map<ShipId, ShipInfo> ships_info_;

        ShipId ship_id_counter_ = 0;

        BoardFieldStatus get_board_field(const Position& field) const noexcept;

        void set_board_field(const Position& field, BoardFieldStatus field_type) noexcept;

        ShipId get_ship_id_at_field(const Position& field) const noexcept;

        ShipId generate_ship_id() noexcept;

        /// @brief Check if the move is valid
        /// @param move The move to check
        /// @return Return error code, Ok if the move is valid
        BoardError get_shot_result(Position move) const noexcept;

        void blockAreaAroundSunkShip(const ShipId ship_id);
    };
}
