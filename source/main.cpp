#include <iostream>

#include "log.h"
#include "board.h"

int main(int, char**){
    std::cout << "Hello, from tictactoe!\n";
    // Initialize logger
    init_logger();

    Board::Board board;


    LOG_V("Hello, from tictactoe!");
    return 0;
}
