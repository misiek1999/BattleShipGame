#include <iostream>

#include "log.h"
#include "board.h"
#include "game_engine.h"

int main(int, char**){
    // Initialize logger
    init_logger();

    GameEngine::GameEngine game_engine;

    game_engine.setPlayerShip(BoardPlayerType::Player_1, Board::ShipType::kDestroyer, {0, 0}, true);
    game_engine.setPlayerShip(BoardPlayerType::Player_2, Board::ShipType::kDestroyer, {0, 0}, true);

    return 0;
}
