#pragma once

#include "board.h"
#include "host_action.h"
#include "user_interface_callback.h"
#include "game_engine.h"
#include "console_manager.h"

#include <thread>
#include <memory>
#include <semaphore>
#include <mutex>

namespace UserInterface {

    enum class UIState {
        Game,
        Exiting
    };

    enum class UIGameState {
        PlacingShips,
        InGame,
        RoundEnd,
    };

    // TODO: implement different UI states (game, exit confirmation, etc.)
    // TODO: implement async call to console manager to update UI
    class UserInterface : public IUserInterfaceCallback, public std::enable_shared_from_this<IUserInterfaceCallback> {
    public:
        explicit UserInterface(std::binary_semaphore& game_end_semaphore);
        ~UserInterface() override = default;

        void startInterface();

        bool setHostPlayerInterface(std::shared_ptr<IHostAction> host_player);

        void onOponentShotResult(const Board::Position& position, const bool is_hit, const bool is_ship_sunk);

        void onGameFinished();

        void onRoundEnded(const GameEngine::RoundResult round_result);

        void onScoreUpdated(const int player_score, const int opponent_score);

        void onBoardReceived(const Board::BoardType board);

        void onOponentBoardReceived(const Board::BoardType board);

        void onShipsCountReceived(const Board::ShipCountMap ships_count);

        void onOponentShipsCountReceived(const Board::ShipCountMap ships_count);

        void onPlayerTurnNotify();

        void onGameStatusUpdated(const GameEngine::GameStatus game_status);

        //  UserInterfaceCallback interface implementation
        virtual void onGameClosed() override;
        virtual void onMoveUp() override;
        virtual void onMoveDown() override;
        virtual void onMoveLeft() override;
        virtual void onMoveRight() override;
        virtual void onSelect() override;
        virtual void onCancel() override;
        virtual void onChar(const char c) override;

    private:
        std::binary_semaphore& game_end_semaphore_;
        std::shared_ptr<IHostAction> host_player_;
        std::unique_ptr<ConsoleManager> console_manager_;
        std::mutex mutex_;

        // TODO: add state machine to handle different UI states
        UIState ui_state_ = UIState::Game;
        bool exit_confirmation_selected_ = false;

        UIGameState ui_game_state_ = UIGameState::PlacingShips;
        /// @brief Horizontal position on the board (col)
        int cursor_pos_x_ = 0;
        /// @brief Vertical position on the board (row)
        int cursor_pos_y_ = 0;

        Board::ShipCountMap ships_count_{};
        bool placing_ship_horizontal_ = true;
        size_t current_ship_to_place_index_ = 0;

        std::size_t round_ {0};

        inline void moveCursorUp();
        inline void moveCursorDown();
        inline void moveCursorLeft();
        inline void moveCursorRight();

        void updateCursorPosition();

        void showExitConfirmation();
        void confirmExit();
        void cancelExit();

        void adjustCursorAfterShipChange();
        void nextShipPlacementOrientation();
        void nextShipToPlace();
        inline bool checkIsSelectedShipPossibleToPlace();
    };

} // namespace UserInterface
