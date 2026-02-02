#pragma once

#include "bot_base.h"
#include "board.h"
#include <random>
#include <vector>
#include <deque>

class BotSonnet : public PlayerBotBase {
public:
    BotSonnet(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              PlayerType player_type);

    RequestId placeShip(Board::ShipType ship_type) override;
    RequestId makeShot() override;
    void onPlayerShotResult(const Board::Position& position,
                            bool is_hit, bool is_ship_sunk) override;

private:
    // FIX: Added Sunk state
    enum class CellState { Unknown, Miss, Hit, Sunk };
    enum class Mode { Hunt, Target };

    std::random_device rd_;
    mutable std::mt19937 gen_;  // FIX: Made mutable for const methods

    std::vector<std::vector<bool>> tried_;
    std::vector<std::vector<CellState>> cell_states_;
    std::vector<size_t> remaining_ship_sizes_;
    std::vector<Board::Position> placed_ship_cells_;
    std::vector<Board::Position> current_hit_chain_;
    std::deque<Board::Position> target_queue_;
    Mode mode_ = Mode::Hunt;

    void initializeRemainingShips();

    // Placement
    Board::Position findBestPlacement(Board::ShipType ship_type, bool& is_vertical);
    double evaluatePlacementPosition(const Board::Position& pos,
                                                size_t ship_size,
                                                bool is_vertical);
    // Shooting
    Board::Position findBestHuntTarget();
    std::vector<std::vector<double>> calculateProbabilityMap() const;
    Board::Position randomPosition();  // FIX: removed const
    bool isValidTarget(const Board::Position& pos) const;

    // Targeting
    void prioritizeTargetQueue();
    std::pair<bool, bool> detectHitOrientation() const;

    // Events
    void onHit(const Board::Position& pos);

    // State management
    void resetState();

    // Safe fallback for shooting
    std::optional<Board::Position> findAnyValidTarget() const;

    // Hit cluster detection for proper sunk handling
    std::vector<Board::Position> collectHitClusterFrom(const Board::Position& start) const;

    // Updated onShipSunk signature
    void onShipSunk(const std::vector<Board::Position>& sunk_cells);
    // ... existing members ...

    size_t ships_placed_this_game_ = 0;
    bool placement_phase_active_ = false;

    void clearLocalBoards();
    void beginPlacementPhaseIfNeeded();
    void applyPlacementToLocalBoard(Board::Position pos, Board::ShipType type, bool is_vertical);

    void enqueueNeighbors(const Board::Position& pos);
};
