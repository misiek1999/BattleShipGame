#include "console_manager.h"

#include "log.h"
#include "keycode_common.h"

#include <iostream>
#include <string_view>
#include <algorithm>
#include <thread>
#include <chrono>
#include <format>

// Platform-specific includes for input handling
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#endif

//TODO: remove atributes after development
static constexpr const int kHeaderLine = 1;
static constexpr const int kGameStatusLine = 2;
static constexpr const int kGameNotificationLine = 3;
static constexpr const int kMainGameTitleLine = 4;
static constexpr const int kMainGameLine = 5;
static constexpr const int kShipsCountLine = 27;
static constexpr const int kShipsOponentCountLine = 28;
static constexpr const int kRoundResultLine = 29;
static constexpr const int kGameExitConfirmationLine = 30;


static constexpr const size_t kPlayerInputStartRow = kMainGameLine + 1;
static constexpr const size_t kPlayerInputStartCol = 3;
static constexpr const size_t kPlayerShotStartRow = kPlayerInputStartRow;
static constexpr const size_t kPlayerShotStartCol = 47;
static constexpr const size_t kBoardRowSpace = 2;
static constexpr const size_t kBoardColSpace = 4;

static constexpr const char kClearLineSeq[] = "\033[2K"; // ANSI escape code to clear the entire line
static constexpr const char kClearScreenSeq[] = "\033[2J\033[1;1H"; // ANSI escape code to clear the entire screen


ConsoleManager::ConsoleManager() {
    LOG_D("ConsoleManager created");
}

ConsoleManager::~ConsoleManager() {
    clearConsole();
    LOG_D("ConsoleManager destroyed");
}

void ConsoleManager::setUserInterfaceCallback(std::shared_ptr<UserInterface::IUserInterfaceCallback> ui_interface) {
    ui_interface_ = std::move(ui_interface);
    LOG_D("UserInterface callback added to ConsoleManager");
}

void ConsoleManager::startConsole() {
    if (!ui_interface_) {
        LOG_E("UserInterface callback is not set");
        throw std::runtime_error("UserInterface callback is not set");
    }
    if (input_thread_.joinable()) {
        LOG_W("ConsoleManager input thread is already running");
        return;
    }
    input_thread_ = std::jthread(
                [this](std::stop_token stoken) { this->consoleInputThread(stoken); }
            );
}

void ConsoleManager::stopConsole() {
    input_thread_.request_stop();
    cv_.notify_one();
    if (input_thread_.joinable()) {
        input_thread_.join();
    }
    LOG_D("ConsoleManager stopped");
}

void ConsoleManager::showExitConfirmation(const bool confirm_was_selected) {
    std::string msg = "Would you like to exit the game?: ";
    if (confirm_was_selected) {
        msg += "> Yes     No";
    } else {
        msg += "  Yes   > No";
    }
    printLineAndClear(msg, kGameExitConfirmationLine);
}

void ConsoleManager::clearExitConfirmation() {
    clearLine(kGameExitConfirmationLine);
}

void ConsoleManager::updateBoardPlayer(const Board::BoardType &board) {
    board_ = board;
    printBoards(board_, oponent_board_);
}

void ConsoleManager::updateBoardOponent(const Board::BoardType &board) {
    oponent_board_ = board;
    printBoards(board_, oponent_board_);
}

void ConsoleManager::updateGameStats(const int host_score, const int guest_score, const size_t round) {
    host_score_ = host_score;
    guest_score_ = guest_score;
    round_ = round;
    printGameStatus();
}

void ConsoleManager::updateRoundCounter(const size_t round) {
    round_ = round;
    printGameStatus();
}

void ConsoleManager::updateRoundEndMessage(const Board::BoardType &board, GameEngine::RoundResult result, size_t round) {
    board_ = board;
    round_ = round;
    result_ = result;
    printGameStatus();
    showRoundEndMessage();
}

void ConsoleManager::printHeader() {
    std::string header = "===== Battleships Game by Michal D. =====";
    printLineAndClear(header, kHeaderLine);
}

void ConsoleManager::printBoards(const Board::BoardType& player_board, const Board::BoardType& oponent_board)
{
    static constexpr const char kTopBorder[]    = "+---+---+---+---+---+---+---+---+---+---+";
    static constexpr const char kMiddleBorder[] = "+---+---+---+---+---+---+---+---+---+---+";
    static constexpr const char kBottomBorder[] = "+---+---+---+---+---+---+---+---+---+---+";
    static constexpr const char kColumnSeparator[] = "|";
    const std::size_t rows = player_board.size();

    // Titles
    std::string title = "               YOUR BOARD"
                      + std::string(sizeof(kTopBorder) - 12, ' ')
                      + "OPONENT BOARD";
    printLineAndClear(title, kMainGameTitleLine);
    printLineAndClear(std::format("{}   {}", kTopBorder, kTopBorder), kMainGameLine);

    // Top borders
    for (std::size_t row = 0; row < rows; ++row) {
        const size_t row_line_num = kMainGameLine + static_cast<int>(row * 2) + 1;
        std::string row_str = kColumnSeparator;
        // ---- Player board row ----
        for (std::size_t col = 0; col < player_board[row].size(); ++col) {
            const char cell =
                Board::get_board_field_char(player_board[row][col].field);
            row_str += " " + std::string(1, cell) + " " + kColumnSeparator;
        }

        row_str += "   ";

        // ---- Opponent board row ----
        row_str += kColumnSeparator;
        for (std::size_t col = 0; col < oponent_board[row].size(); ++col) {
            const char cell =
                Board::get_board_field_char(oponent_board[row][col].field);
            row_str += " " + std::string(1, cell) + " " + kColumnSeparator;
        }

        printLineAndClear(row_str, row_line_num);

        // Middle borders (except last row)
        if (row < rows - 1) {
            std::string middle_border = std::format("{}   {}", kMiddleBorder, kMiddleBorder);
            printLineAndClear(middle_border, row_line_num + 1);
        }
    }

    // Bottom borders
    std::string bottom_border = std::format("{}   {}", kBottomBorder, kBottomBorder);
    printLineAndClear(bottom_border, kMainGameLine + static_cast<int>(rows * 2));
}

void ConsoleManager::printGameStatus() {
    std::string status_str;
    switch (game_status_) {
        case GameEngine::GameStatus::NotStarted:
            status_str = "Not Started";
            break;
        case GameEngine::GameStatus::PreparingBoards:
            status_str = "Preparing Boards";
            printShipPlacementInstructions();
            break;
        case GameEngine::GameStatus::RoundInProgress:
            status_str = "Round In Progress";
            break;
        case GameEngine::GameStatus::RoundFinished:
            status_str = "Round Finished";
            break;
        case GameEngine::GameStatus::GameEnded:
            status_str = "Game Ended";
            break;
        default:
            status_str = "Unknown";
            break;
    }
    std::string str = std::format("Round: {}, Host Score: {}, Guest Score: {}, Game phase: {}", round_, host_score_, guest_score_, status_str);
    printLineAndClear(str, kGameStatusLine);
}

void ConsoleManager::printShipsCount() {
    const auto ship_str = Board::ship_count_to_string(ships_count_);
    printLineAndClear(std::format("Your's  ship count: {}", ship_str), kShipsCountLine);
}

void ConsoleManager::printOponentShipsCount() {
    const auto ship_str = Board::ship_count_to_string(oponent_ships_count_);
    printLineAndClear(std::format("Oponent ship count: {}", ship_str), kShipsOponentCountLine);
}

void ConsoleManager::printShipPlacementInstructions() {
    std::string msg = "Place your ships on the board using arrow keys and 'R' to select orientation. 'N' to change ship type. Press 'Enter' to confirm placement.";
    printLineAndClear(msg, kGameNotificationLine);
}

void ConsoleManager::moveCursorToPosition(const int row, const int col) {
    last_point_cursor_col_ = col;
    last_point_cursor_row_ = row;
    std::cout << "\033[" << row << ";" << col << "H" << std::flush;
}

void ConsoleManager::moveCursorToInputLine(const int line_num) {
    if (line_num > 0) {
        std::cout << "\033[" << line_num << ";1H" << std::flush;
    }
    std::cout << "\r" << std::flush; // move to column 0
}

void ConsoleManager::clearLine() {
    std::cout << kClearLineSeq << std::flush;
}

void ConsoleManager::clearLine(const int line_num) {
    moveCursorToInputLine(line_num);
    clearLine();
}

void ConsoleManager::printLineAndClear(const std::string &line) {
    std::cout << "\r";
    clearLine();
    std::cout << line;
    std::cout.flush();
}

void ConsoleManager::printLineAndClear(const std::string &line, const int line_num) {
    moveCursorToInputLine(line_num);
    printLineAndClear(line);
    restoreCursorPosition();
}

void ConsoleManager::toggleCursorMoveBackAfterInput(const bool enable) {
    cursor_move_back_after_input_ = enable;
}

void ConsoleManager::restoreCursorPosition() {
    if (cursor_move_back_after_input_) {
        moveCursorToPosition(last_point_cursor_row_, last_point_cursor_col_);
    }
}

void ConsoleManager::showRoundEndMessage() {

    std::string result_str = "Round result: ";
    switch (result_) {
        case GameEngine::RoundResult::Player1Won:
            result_str += "Player 1 (Host) Won!";
            break;
        case GameEngine::RoundResult::Player2Won:
            result_str += "Player 2 (Guest) Won!";
            break;
        case GameEngine::RoundResult::Draw:
            result_str += "It's a Draw!";
            break;
        case GameEngine::RoundResult::GameAborted:
            result_str += "Game was Aborted";
            break;
        default:
            result_str = "Unknown Result";
            break;
    }

    printLineAndClear(result_str, kRoundResultLine);
}

void ConsoleManager::showPlayerTurnNotification() {
    printLineAndClear("It's your turn!", kGameNotificationLine);
}

void ConsoleManager::showMessage(const std::string &message) {
    (void) message;
    // TODO: implement
}

void ConsoleManager::updateGameStatus(GameEngine::GameStatus game_status) {
    game_status_ = game_status;
    printGameStatus();
}

void ConsoleManager::updateShipsCount(const Board::ShipCountMap &ships_count) {
    ships_count_ = ships_count;
    printShipsCount();
}

void ConsoleManager::updateOponentShipsCount(const Board::ShipCountMap &ships_count) {
    oponent_ships_count_ = ships_count;
    printOponentShipsCount();
}

void ConsoleManager::showMakeShotInformation() {
    printLineAndClear("Use arrow to navigate and space to shot", kGameNotificationLine);
}

void ConsoleManager::showShipHitInformation(const bool is_hit, const bool is_sunk) {
    if (is_hit) {
        if (is_sunk) {
            printLineAndClear("Ship was hit and sunk!", kGameNotificationLine);
        } else {
            printLineAndClear("Ship was hit!", kGameNotificationLine);
        }
    } else {
        printLineAndClear("Shot miss", kGameNotificationLine);
    }
}

void ConsoleManager::showRoundEndInformation() {
    printLineAndClear("Press space to start new game", kGameNotificationLine);
}

void ConsoleManager::clearPlayerTurnNotification() {
    clearLine(kGameNotificationLine);
}

void ConsoleManager::resetConsoleView() {
    clearConsole();
    printHeader();
    printGameStatus();
    printBoards(board_, oponent_board_);
    printShipsCount();
    printOponentShipsCount();
}

void ConsoleManager::clearConsole() {
    // ANSI escape code for clearing screen
    std::cout << kClearScreenSeq << std::flush;
}

void ConsoleManager::moveCursorToPlayerBoardInput(const int row, const int col) {
    const auto cur_row = kPlayerInputStartRow + row * kBoardRowSpace;
    const auto cur_col = kPlayerInputStartCol + col * kBoardColSpace;
    moveCursorToPosition(cur_row, cur_col);
}

void ConsoleManager::moveCursorToShot(const int row, const int col) {
    const auto cur_row = kPlayerShotStartRow + row * kBoardRowSpace;
    const auto cur_col = kPlayerShotStartCol + col * kBoardColSpace;
    moveCursorToPosition(cur_row, cur_col);
}

void ConsoleManager::renderShipPlacement(const Board::ShipType ship_type, const bool is_vertical) {
    const auto ship_size = Board::get_ship_size(ship_type);
    const auto symbol = Board::ship_type_to_symbol(ship_type);

    int prv_row = last_point_cursor_row_;
    int prv_col = last_point_cursor_col_;
    int row = last_point_cursor_row_;
    int col = last_point_cursor_col_;
    for (size_t i = 0; i < ship_size; ++i) {
        moveCursorToPosition(row, col);
        std::cout << symbol << std::flush;
        if (is_vertical) {
            row += kBoardRowSpace;
        } else {
            col += kBoardColSpace;
        }
    }
    moveCursorToPosition(prv_row, prv_col);
}

void ConsoleManager::clearRenderedShipPlacement() {
    // int prv_row = last_point_cursor_row_;
    // int prv_col = last_point_cursor_col_;
    // for (size_t col = 0; col < Board::kBoardSizeCol; ++col) {
    //     for (size_t row = 0; row < Board::kBoardSizeRow; ++row) {
    //         int cur_row = kPlayerInputStartRow + static_cast<int>(row * 2);
    //         int cur_col = kPlayerInputStartCol + static_cast<int>(col * 4);
    //         moveCursorToPosition(cur_row, cur_col);
    //         std::cout << " ";
    //     }
    // }
    // moveCursorToPosition(prv_row, prv_col);
    printBoards(board_, oponent_board_);
}

void ConsoleManager::consoleInputThread(std::stop_token stoken) {
    LOG_D("Console input thread started");
    bool stopped = false;
    while (!stopped && !stoken.stop_requested()) {
        auto ev = read_key_with_cancel(stoken);
        switch (ev.type) {
            case Key::Exit:
                stopped = true;
                break;
            case Key::Char:
                ui_interface_->onChar(ev.ch);
                break;
            case Key::ArrowUp:
                ui_interface_->onMoveUp();
                break;
            case Key::ArrowDown:
                ui_interface_->onMoveDown();
                break;
            case Key::ArrowLeft:
                ui_interface_->onMoveLeft();
                break;
            case Key::ArrowRight:
                ui_interface_->onMoveRight();
                break;
            case Key::Select:
                ui_interface_->onSelect();
                break;
            case Key::Cancelled:
                ui_interface_->onCancel();
                break;
            case Key::Escape:
                ui_interface_->onCancel();
                break;
            default:
                break;
        }
    }

    LOG_I("Game session stopping as requested");
}
