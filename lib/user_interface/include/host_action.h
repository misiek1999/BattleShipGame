#pragma once

#include "board.h"

namespace UserInterface {
    class IHostAction {
        public:
            virtual ~IHostAction() = default;
            virtual bool makeShot(const Board::Position& position, bool& was_hit, bool& was_sunk) = 0;
            virtual bool placeShip(const Board::ShipType ship_type, const Board::Position& position, const bool is_horizontal) = 0;
            virtual bool sendMessage(const std::string& message) = 0;
            virtual bool notifyReady() = 0;
            virtual bool endGame() = 0;
    };
}   // namespace UserInterface
