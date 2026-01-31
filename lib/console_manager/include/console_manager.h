#pragma once

#include "board.h"
#include "user_interface_callback.h"
#include "game_engine.h"

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <chrono>

class ConsoleManager {
public:
    explicit ConsoleManager();
    ~ConsoleManager();

    void setUserInterfaceCallback(std::shared_ptr<UserInterface::IUserInterfaceCallback> ui_interface);

    void startConsole();
    void stopConsole();

    void showExitConfirmation(const bool confirm_was_selected);
    void clearExitConfirmation();

    void updateBoardPlayer(const Board::BoardType& board);
    void updateBoardOponent(const Board::BoardType& board);
    void updateGameStats(int host_score, int guest_score, size_t round);
    void updateRoundEndMessage(const Board::BoardType& board, GameEngine::RoundResult result, size_t round);
    void showRoundEndMessage();
    void showPlayerTurnNotification();
    void printShipPlacementInstructions();
    void showMessage(const std::string& message);
    void updateGameStatus(GameEngine::GameStatus game_status);
    void updateShipsCount(const Board::ShipCountMap& ships_count);
    void updateOponentShipsCount(const Board::ShipCountMap& ships_count);
    void showMakeShotInformation();
    void showShipHitInformation(const bool is_hit, const bool is_sunk);

    void clearPlayerTurnNotification();

    void resetConsoleView();
    void clearConsole();

    void moveCursorToPlayerBoardInput(const int row, const int col);
    void moveCursorToShot(const int row, const int col);

    void renderShipPlacement(const Board::ShipType ship_type, const bool is_vertical);
    void clearRenderedShipPlacement();

private:
    void consoleInputThread(std::stop_token stoken);

    void printHeader();
    void printBoards(const Board::BoardType& player_board, const Board::BoardType& oponent_board);
    void printGameStatus();
    void printShipsCount();
    void printOponentShipsCount();

    void moveCursorToPosition(const int row, const int col);
    void moveCursorToInputLine(const int line_num = 0);
    void clearLine();
    void clearLine(const int line_num);
    void printLineAndClear(const std::string& line);
    void printLineAndClear(const std::string& line, const int line_num);

    void toggleCursorMoveBackAfterInput(const bool enable);
    void restoreCursorPosition();

    static constexpr const std::chrono::milliseconds kInputSleep = std::chrono::milliseconds(20);

    std::shared_ptr<UserInterface::IUserInterfaceCallback> ui_interface_;
    std::jthread input_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;

    Board::BoardType board_;
    Board::BoardType oponent_board_;
    int host_score_ = 0;
    int guest_score_ = 0;
    size_t round_ = 0;
    GameEngine::GameStatus game_status_ = GameEngine::GameStatus::NotStarted;
    Board::ShipCountMap ships_count_{};
    Board::ShipCountMap oponent_ships_count_{};
    GameEngine::RoundResult result_ {GameEngine::RoundResult::GameNotStarted};

    bool cursor_move_back_after_input_ = true;
    int last_point_cursor_row_ = 0;
    int last_point_cursor_col_ = 0;
};
