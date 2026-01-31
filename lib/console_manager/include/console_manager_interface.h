#pragma once

#include "board.h"
#include "user_interface_callback.h"
#include "game_engine.h"

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <string>

class IConsoleManager {
public:
    virtual ~IConsoleManager() = default;

    virtual void setUserInterfaceCallback(
        std::shared_ptr<UserInterface::IUserInterfaceCallback> ui_interface) = 0;

    virtual void startConsole() = 0;
    virtual void stopConsole() = 0;

    virtual void showExitConfirmation(bool confirm_was_selected) = 0;
    virtual void clearExitConfirmation() = 0;

    virtual void updateBoardPlayer(const Board::BoardType& board) = 0;
    virtual void updateBoardOponent(const Board::BoardType& board) = 0;
    virtual void updateGameStats(int host_score, int guest_score, size_t round) = 0;
    virtual void updateRoundCounter(size_t round) = 0;
    virtual void updateRoundEndMessage(const Board::BoardType& board,
                                       GameEngine::RoundResult result,
                                       size_t round) = 0;

    virtual void showRoundEndMessage() = 0;
    virtual void showRoundEndInformation() = 0;
    virtual void showPlayerTurnNotification() = 0;
    virtual void printShipPlacementInstructions() = 0;
    virtual void showMessage(const std::string& message) = 0;

    virtual void updateGameStatus(GameEngine::GameStatus game_status) = 0;
    virtual void updateShipsCount(const Board::ShipCountMap& ships_count) = 0;
    virtual void updateOponentShipsCount(const Board::ShipCountMap& ships_count) = 0;

    virtual void showMakeShotInformation() = 0;
    virtual void showShipHitInformation(bool is_hit, bool is_sunk) = 0;

    virtual void clearPlayerTurnNotification() = 0;
    virtual void resetConsoleView() = 0;
    virtual void clearConsole() = 0;

    virtual void moveCursorToPlayerBoardInput(int row, int col) = 0;
    virtual void moveCursorToShot(int row, int col) = 0;

    virtual void renderShipPlacement(Board::ShipType ship_type, bool is_vertical) = 0;
    virtual void clearRenderedShipPlacement() = 0;
};
