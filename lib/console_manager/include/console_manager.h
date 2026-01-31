#pragma once

#include "board.h"
#include "user_interface_callback.h"
#include "game_engine.h"
#include "console_manager_interface.h"

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <chrono>

class ConsoleManager : public IConsoleManager {
public:
    ConsoleManager();
    ~ConsoleManager() override;

    void setUserInterfaceCallback(
        std::shared_ptr<UserInterface::IUserInterfaceCallback> ui_interface) override;

    void startConsole() override;
    void stopConsole() override;

    void showExitConfirmation(bool confirm_was_selected) override;
    void clearExitConfirmation() override;

    void updateBoardPlayer(const Board::BoardType& board) override;
    void updateBoardOponent(const Board::BoardType& board) override;

    void updateGameStats(int host_score, int guest_score, size_t round) override;
    void updateRoundCounter(size_t round) override;

    void updateRoundEndMessage(const Board::BoardType& board,
                               GameEngine::RoundResult result,
                               size_t round) override;

    void showRoundEndMessage() override;
    void showRoundEndInformation() override;
    void showPlayerTurnNotification() override;

    void printShipPlacementInstructions() override;
    void showMessage(const std::string& message) override;

    void updateGameStatus(GameEngine::GameStatus game_status) override;
    void updateShipsCount(const Board::ShipCountMap& ships_count) override;
    void updateOponentShipsCount(const Board::ShipCountMap& ships_count) override;

    void showMakeShotInformation() override;
    void showShipHitInformation(bool is_hit, bool is_sunk) override;

    void clearPlayerTurnNotification() override;
    void resetConsoleView() override;
    void clearConsole() override;

    void moveCursorToPlayerBoardInput(int row, int col) override;
    void moveCursorToShot(int row, int col) override;

    void renderShipPlacement(Board::ShipType ship_type, bool is_vertical) override;
    void clearRenderedShipPlacement() override;

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
    size_t round_ = 1;
    GameEngine::GameStatus game_status_ = GameEngine::GameStatus::NotStarted;
    Board::ShipCountMap ships_count_{};
    Board::ShipCountMap oponent_ships_count_{};
    GameEngine::RoundResult result_ {GameEngine::RoundResult::GameNotStarted};

    bool cursor_move_back_after_input_ = true;
    int last_point_cursor_row_ = 0;
    int last_point_cursor_col_ = 0;
};
