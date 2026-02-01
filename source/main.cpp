#include <iostream>
#include <memory>

#include "log.h"
#include "game_manager.h"

int main(int argc, char* argv[]){
    bool dump_log_to_file {false};
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "-v") {
            dump_log_to_file = true;
        }
    }
    // Initialize logger
    init_logger(dump_log_to_file);
    try {
        GameManager::GameManager game_manager;

        game_manager.startGame();
    } catch (const std::exception& e) {
        LOG_E("Exception caught in main: {}", e.what());
        return EXIT_FAILURE;
    }
    LOG_I("Main function exit. Close game.");
    return 0;
}
