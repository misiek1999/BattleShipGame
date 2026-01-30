#include "user_interface.h"
#include "log.h"

namespace UserInterface
{

    UserInterface::UserInterface(std::binary_semaphore& game_end_semaphore)
        : game_end_semaphore_(game_end_semaphore) {
        console_manager_ = std::make_unique<ConsoleManager>();
        LOG_D("UserInterface created");
    }

    void UserInterface::startInterface() {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->setUserInterfaceCallback(shared_from_this());
        console_manager_->startConsole();
        console_manager_->resetConsoleView();
        LOG_I("User interface started");
    }

    bool UserInterface::setHostPlayerInterface(std::shared_ptr<IHostAction> host_player) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (host_player_ != nullptr) {
            LOG_W("Host player interface was already set!");
            return false;
        }
        if (host_player == nullptr) {
            LOG_E("Host player interface is null");
            return false;
        }
        host_player_ = host_player;
        return true;
    }

    void UserInterface::onOponentShotResult(const Board::Position &position, const bool is_hit, const bool is_ship_sunk) {
        std::lock_guard<std::mutex> lock(mutex_);
        (void) position;
        (void) is_hit;
        (void) is_ship_sunk;
    }

    void UserInterface::onGameFinished() {
        std::lock_guard<std::mutex> lock(mutex_);
        LOG_I("Game finished, releasing semaphore.");
        game_end_semaphore_.release();
    }

    void UserInterface::onRoundEnded(const GameEngine::RoundResult round_result) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateGameEndMessage({}, round_result, -1);
    }

    void UserInterface::onScoreUpdated(const int player_score, const int opponent_score) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateGameStats(player_score, opponent_score, -1);
    }

    void UserInterface::onBoardReceived(const Board::BoardType board) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateBoardPlayer(board);
    }

    void UserInterface::onOponentBoardReceived(const Board::BoardType board) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateBoardOponent(board);
    }

    void UserInterface::onShipsCountReceived(const Board::ShipCountMap ships_count) {
        ships_count_ = ships_count;
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateShipsCount(ships_count);
    }

    void UserInterface::onOponentShipsCountReceived(const Board::ShipCountMap ships_count) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateOponentShipsCount(ships_count);
    }

    void UserInterface::onPlayerTurnNotify() {
        if (ui_state_ == UIState::Exiting || ui_game_state_ == UIGameState::PlacingShips) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->showPlayerTurnNotification();
    }

    void UserInterface::onGameStatusUpdated(const GameEngine::GameStatus game_status) {
        if (game_status == GameEngine::GameStatus::RoundInProgress) {
            ui_game_state_ = UIGameState::InGame;
            std::lock_guard<std::mutex> lock(mutex_);
            console_manager_->moveCursorToShot(cursor_pos_y_, cursor_pos_x_);
        }
        if (game_status == GameEngine::GameStatus::PreparingBoards) {
            ui_game_state_ = UIGameState::PlacingShips;
            std::unique_lock<std::mutex> lock(mutex_);
            console_manager_->moveCursorToPlayerBoardInput(cursor_pos_y_, cursor_pos_x_);
            console_manager_->printShipPlacementInstructions();
            lock.unlock();
            updateCursorPosition();

        }
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->updateGameStatus(game_status);
    }

    void UserInterface::onGameClosed() {
        LOG_I("Game closed by user.");
        game_end_semaphore_.release();
    }

    void UserInterface::onMoveUp() {
        LOG_D("Move Up action received");
        if (ui_state_ == UIState::Exiting) {
            return;
        }
        moveCursorUp();

    }

    void UserInterface::onMoveDown() {
        LOG_D("Move Down action received");
        if (ui_state_ == UIState::Exiting) {
            return;
        }
        moveCursorDown();
    }

    void UserInterface::onMoveLeft() {
        LOG_D("Move Left action received");
        if (ui_state_ == UIState::Exiting) {
            exit_confirmation_selected_ = true;
            std::lock_guard<std::mutex> lock(mutex_);
            console_manager_->showExitConfirmation(exit_confirmation_selected_);
            return;
        }
        moveCursorLeft();
    }

    void UserInterface::onMoveRight() {
        LOG_D("Move Right action received");
        if (ui_state_ == UIState::Exiting) {
            exit_confirmation_selected_ = false;
            std::lock_guard<std::mutex> lock(mutex_);
            console_manager_->showExitConfirmation(exit_confirmation_selected_);
            return;
        }
        moveCursorRight();
    }

    void UserInterface::onSelect() {
        LOG_D("Select action received");
        if (ui_state_ == UIState::Exiting) {
            if (exit_confirmation_selected_) {
                confirmExit();
            } else {
                cancelExit();
            }
            return;
        }
        if (host_player_ != nullptr) {
            if (ui_game_state_ == UIGameState::PlacingShips) {
                Board::ShipType ship_type = static_cast<Board::ShipType>(current_ship_to_place_index_);
                bool placed = host_player_->placeShip(ship_type, {cursor_pos_y_, cursor_pos_x_}, placing_ship_horizontal_);
                if (placed) {
                    std::unique_lock<std::mutex> lock(mutex_);
                    console_manager_->clearRenderedShipPlacement();
                    lock.unlock();
                    if (!checkIsSelectedShipPossibleToPlace()) {
                        nextShipToPlace();
                    }
                    updateCursorPosition();
                } else {
                    LOG_W("Failed to place ship at position ({}, {})", cursor_pos_y_, cursor_pos_x_);
                }
            }
            if (ui_game_state_ == UIGameState::InGame) {
                bool was_hit = false;
                bool was_sunk = false;
                bool success = host_player_->makeShot({cursor_pos_y_, cursor_pos_x_}, was_hit, was_sunk);
                if (!success) {
                    LOG_W("Failed to place ship at position ({}, {})", cursor_pos_y_, cursor_pos_x_);
                }
            }
        }
    }

    void UserInterface::onCancel() {
        LOG_D("Cancel action received");
        if (ui_state_ == UIState::Exiting) {
            cancelExit();
        } else {
            showExitConfirmation();
        }
        return;
    }

    void UserInterface::onChar(const char c) {
        LOG_D("Char action received: {}", c);
        if (ui_state_ == UIState::Exiting) {
            return;
        }
        if (ui_game_state_ == UIGameState::PlacingShips) {
            if (c == 'r' || c == 'R') {
                nextShipPlacementOrientation();
                updateCursorPosition();
            } else if (c == 'n' || c == 'N') {
                nextShipToPlace();
                updateCursorPosition();
            }
        }
    }

    void UserInterface::moveCursorUp() {
        if (cursor_pos_y_ > 0) {
            --cursor_pos_y_ ;
        }
        updateCursorPosition();
    }

    void UserInterface::moveCursorDown() {
        size_t y = cursor_pos_y_ + 1;
        if (y < Board::kBoardSizeRow) {
            cursor_pos_y_ = y;
        }
        updateCursorPosition();
    }

    void UserInterface::moveCursorLeft() {
        if (cursor_pos_x_ > 0) {
            --cursor_pos_x_;
        }
        updateCursorPosition();
    }

    void UserInterface::moveCursorRight() {
        size_t x = cursor_pos_x_ + 1;
        if (x < Board::kBoardSizeCol) {
            cursor_pos_x_ = x;
        }
        updateCursorPosition();
    }

    void UserInterface::updateCursorPosition() {
        adjustCursorAfterShipChange();
        if (ui_game_state_ == UIGameState::PlacingShips) {
            std::lock_guard<std::mutex> lock(mutex_);
            console_manager_->moveCursorToPlayerBoardInput(cursor_pos_y_, cursor_pos_x_);
            console_manager_->clearRenderedShipPlacement();
            console_manager_->renderShipPlacement(static_cast<Board::ShipType>(current_ship_to_place_index_),
                                               !placing_ship_horizontal_);
        } else if (ui_game_state_ == UIGameState::InGame) {
            std::lock_guard<std::mutex> lock(mutex_);
            console_manager_->moveCursorToShot(cursor_pos_y_, cursor_pos_x_);
        }
    }

    void UserInterface::showExitConfirmation()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->showExitConfirmation(exit_confirmation_selected_);
        ui_state_ = UIState::Exiting;
    }

    void UserInterface::confirmExit() {
        LOG_I("User confirmed exit.");
        game_end_semaphore_.release();
    }

    void UserInterface::cancelExit() {
        LOG_I("User canceled exit.");
        ui_state_ = UIState::Game;
        std::lock_guard<std::mutex> lock(mutex_);
        console_manager_->clearExitConfirmation();
    }

    void UserInterface::adjustCursorAfterShipChange() {
        if (ui_game_state_ != UIGameState::PlacingShips) {
            return;
        }
        const auto ship_size = Board::get_ship_size(static_cast<Board::ShipType>(current_ship_to_place_index_));
        if (placing_ship_horizontal_) {
            if (cursor_pos_x_ + ship_size > Board::kBoardSizeCol) {
                cursor_pos_x_ = Board::kBoardSizeCol - ship_size;
            }
        } else {
            if (cursor_pos_y_ + ship_size > Board::kBoardSizeRow) {
                cursor_pos_y_ = Board::kBoardSizeRow - ship_size;
            }
        }
    }

    void UserInterface::nextShipPlacementOrientation() {
        placing_ship_horizontal_ = !placing_ship_horizontal_;
        adjustCursorAfterShipChange();
    }

    void UserInterface::nextShipToPlace() {
        for (size_t i = 0; i < Board::kNumberOfShips; ++i) {
            // Avoid index out of bounds and loop back to start
            if (++current_ship_to_place_index_ >= Board::kNumberOfShips) {
                current_ship_to_place_index_ = 0;
            }
            // Check if there are remaining ships of this type to place
            if (checkIsSelectedShipPossibleToPlace()) {
                break;
            }
        }
        adjustCursorAfterShipChange();
    }
    bool UserInterface::checkIsSelectedShipPossibleToPlace() {
        if (ships_count_[current_ship_to_place_index_] < Board::get_max_ship_count(static_cast<Board::ShipType>(current_ship_to_place_index_))) {
            return true;
        }
        return false;
    }
} // namespace UserInterface
