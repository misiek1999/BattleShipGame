#include <iostream>
#include <memory>

#include "log.h"
#include "game_manager.h"

int main(int, char**){
    // Initialize logger
    init_logger();
    try {
        GameManager::GameManager game_manager;

        game_manager.startGame();
    } catch (const std::exception& e) {
        LOG_E("Exception caught in main: {}", e.what());
        return EXIT_FAILURE;
    }
    return 0;
}
