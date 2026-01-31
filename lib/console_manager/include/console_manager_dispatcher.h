#pragma once

#include "console_manager_interface.h"
#include "console_manager.h"


#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
class ConsoleManagerDispatcher final : public IConsoleManager {
public:
    ConsoleManagerDispatcher()
        : console_(std::make_unique<ConsoleManager>())
    {
        worker_ = std::thread(&ConsoleManagerDispatcher::worker, this);
    }

    ~ConsoleManagerDispatcher() override {
        running_ = false;
        cv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    // ---------------- lifecycle ----------------
    void setUserInterfaceCallback(
        std::shared_ptr<UserInterface::IUserInterfaceCallback> ui) override
    {
            console_->setUserInterfaceCallback(ui);
    }

    void startConsole() override {
            console_->startConsole();
    }

    void stopConsole() override {
        dispatch([](ConsoleManager& cm) {
            cm.stopConsole();
        });
    }

    // ---------------- exit ----------------
    void showExitConfirmation(bool confirm) override {
        dispatch([confirm](ConsoleManager& cm) {
            cm.showExitConfirmation(confirm);
        });
    }

    void clearExitConfirmation() override {
        dispatch([](ConsoleManager& cm) {
            cm.clearExitConfirmation();
        });
    }

    // ---------------- board / game ----------------
    void updateBoardPlayer(const Board::BoardType& board) override {
        dispatch([board](ConsoleManager& cm) {
            cm.updateBoardPlayer(board);
        });
    }

    void updateBoardOponent(const Board::BoardType& board) override {
        dispatch([board](ConsoleManager& cm) {
            cm.updateBoardOponent(board);
        });
    }

    void updateGameStats(int host, int guest, size_t round) override {
        dispatch([host, guest, round](ConsoleManager& cm) {
            cm.updateGameStats(host, guest, round);
        });
    }

    void updateRoundCounter(size_t round) override {
        dispatch([round](ConsoleManager& cm) {
            cm.updateRoundCounter(round);
        });
    }

    void updateRoundEndMessage(const Board::BoardType& board,
                               GameEngine::RoundResult result,
                               size_t round) override
    {
        dispatch([board, result, round](ConsoleManager& cm) {
            cm.updateRoundEndMessage(board, result, round);
        });
    }

    // ---------------- notifications ----------------
    void showRoundEndMessage() override {
        dispatch([](ConsoleManager& cm) {
            cm.showRoundEndMessage();
        });
    }

    void showRoundEndInformation() override {
        dispatch([](ConsoleManager& cm) {
            cm.showRoundEndInformation();
        });
    }

    void showPlayerTurnNotification() override {
        dispatch([](ConsoleManager& cm) {
            cm.showPlayerTurnNotification();
        });
    }

    void printShipPlacementInstructions() override {
        dispatch([](ConsoleManager& cm) {
            cm.printShipPlacementInstructions();
        });
    }

    void showMessage(const std::string& msg) override {
        dispatch([msg](ConsoleManager& cm) {
            cm.showMessage(msg);
        });
    }

    void showMakeShotInformation() override {
        dispatch([](ConsoleManager& cm) {
            cm.showMakeShotInformation();
        });
    }

    void showShipHitInformation(bool is_hit, bool is_sunk) override {
        dispatch([is_hit, is_sunk](ConsoleManager& cm) {
            cm.showShipHitInformation(is_hit, is_sunk);
        });
    }

    // ---------------- status & counts ----------------
    void updateGameStatus(GameEngine::GameStatus status) override {
        dispatch([status](ConsoleManager& cm) {
            cm.updateGameStatus(status);
        });
    }

    void updateShipsCount(const Board::ShipCountMap& ships) override {
        dispatch([ships](ConsoleManager& cm) {
            cm.updateShipsCount(ships);
        });
    }

    void updateOponentShipsCount(const Board::ShipCountMap& ships) override {
        dispatch([ships](ConsoleManager& cm) {
            cm.updateOponentShipsCount(ships);
        });
    }

    // ---------------- misc ----------------
    void clearPlayerTurnNotification() override {
        dispatch([](ConsoleManager& cm) {
            cm.clearPlayerTurnNotification();
        });
    }

    void resetConsoleView() override {
        dispatch([](ConsoleManager& cm) {
            cm.resetConsoleView();
        });
    }

    void clearConsole() override {
        dispatch([](ConsoleManager& cm) {
            cm.clearConsole();
        });
    }

    // ---------------- cursor / rendering ----------------
    void moveCursorToPlayerBoardInput(int row, int col) override {
        dispatch([row, col](ConsoleManager& cm) {
            cm.moveCursorToPlayerBoardInput(row, col);
        });
    }

    void moveCursorToShot(int row, int col) override {
        dispatch([row, col](ConsoleManager& cm) {
            cm.moveCursorToShot(row, col);
        });
    }

    void renderShipPlacement(Board::ShipType type, bool vertical) override {
        dispatch([type, vertical](ConsoleManager& cm) {
            cm.renderShipPlacement(type, vertical);
        });
    }

    void clearRenderedShipPlacement() override {
        dispatch([](ConsoleManager& cm) {
            cm.clearRenderedShipPlacement();
        });
    }

private:
    void dispatch(std::function<void(ConsoleManager&)> fn) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(fn));
        }
        cv_.notify_one();
    }

    void worker() {
        while (running_) {
            std::function<void(ConsoleManager&)> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [&] {
                    return !queue_.empty() || !running_;
                });

                if (!running_ && queue_.empty()) {
                    return;
                }

                task = std::move(queue_.front());
                queue_.pop();
            }

            task(*console_);
        }
    }

private:
    std::unique_ptr<ConsoleManager> console_;
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::function<void(ConsoleManager&)>> queue_;
    std::atomic<bool> running_{true};
};