/*
    Generated for Battleship AI comparison
    Fixed & merged version
*/

#include "bot_sonnet.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <queue>

// =============================================================================
// CONSTRUCTION / RESET
// =============================================================================

BotSonnet::BotSonnet(
    std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
    const PlayerType player_type)
    : PlayerBotBase(action_interface, player_type),
      gen_(rd_()) {
    resetState();
}

void BotSonnet::clearLocalBoards() {
    for (auto& r : board_) r.fill({});
    for (auto& r : oponent_board_) r.fill({});
}

void BotSonnet::resetState() {
    clearLocalBoards();

    tried_.assign(Board::kBoardSizeRow,
                  std::vector<bool>(Board::kBoardSizeCol, false));

    cell_states_.assign(Board::kBoardSizeRow,
                        std::vector<CellState>(Board::kBoardSizeCol,
                                                CellState::Unknown));

    initializeRemainingShips();

    mode_ = Mode::Hunt;
    target_queue_.clear();
    current_hit_chain_.clear();

    placed_ship_cells_.clear();
    ships_placed_this_game_ = 0;
    placement_phase_active_ = true;
}

// =============================================================================
// SHIP PLACEMENT
// =============================================================================

void BotSonnet::beginPlacementPhaseIfNeeded() {
    if (!placement_phase_active_ ||
        ships_placed_this_game_ >= Board::kTotalShipsCount) {
        resetState();
    }
}

void BotSonnet::initializeRemainingShips() {
    remaining_ship_sizes_.clear();

    for (size_t i = 0; i < Board::kNumberOfShips; ++i) {
        auto type = static_cast<Board::ShipType>(i);
        size_t count = Board::get_max_ship_count(type);
        size_t size  = Board::get_ship_size(type);
        remaining_ship_sizes_.insert(remaining_ship_sizes_.end(), count, size);
    }
}

void BotSonnet::applyPlacementToLocalBoard(Board::Position pos,
                                           Board::ShipType type,
                                           bool is_vertical) {
    size_t L = Board::get_ship_size(type);
    for (size_t i = 0; i < L; ++i) {
        int r = is_vertical ? pos.first + i : pos.first;
        int c = is_vertical ? pos.second : pos.second + i;
        board_[r][c].field = Board::BoardFieldStatus::Ship;
    }
}

RequestId BotSonnet::placeShip(const Board::ShipType ship_type) {
    beginPlacementPhaseIfNeeded();

    bool is_vertical = true;
    Board::Position pos = findBestPlacement(ship_type, is_vertical);

    applyPlacementToLocalBoard(pos, ship_type, is_vertical);

    size_t size = Board::get_ship_size(ship_type);
    for (size_t i = 0; i < size; ++i) {
        int r = is_vertical ? pos.first + i : pos.first;
        int c = is_vertical ? pos.second : pos.second + i;
        placed_ship_cells_.push_back({r, c});
    }

    ++ships_placed_this_game_;
    if (ships_placed_this_game_ >= Board::kTotalShipsCount)
        placement_phase_active_ = false;

    return action_interface_->placeShip(ship_type, pos, !is_vertical);
}

// =============================================================================
// PLACEMENT SCORING
// =============================================================================

double BotSonnet::evaluatePlacementPosition(const Board::Position& pos,
                                            size_t ship_size,
                                            bool is_vertical) {
    double score = 0.0;

    int end_r = is_vertical ? pos.first + ship_size - 1 : pos.first;
    int end_c = is_vertical ? pos.second : pos.second + ship_size - 1;

    // Edge penalty
    if (pos.first == 0 || end_r == int(Board::kBoardSizeRow) - 1)
        score -= 1.5;
    if (pos.second == 0 || end_c == int(Board::kBoardSizeCol) - 1)
        score -= 1.5;

    // Distance from existing ships
    if (!placed_ship_cells_.empty()) {
        double min_dist = std::numeric_limits<double>::max();

        for (size_t i = 0; i < ship_size; ++i) {
            int r = is_vertical ? pos.first + i : pos.first;
            int c = is_vertical ? pos.second : pos.second + i;

            for (auto& p : placed_ship_cells_) {
                double d = std::hypot(r - p.first, c - p.second);
                min_dist = std::min(min_dist, d);
            }
        }
        score += min_dist * 0.8;
    }

    // Center avoidance
    double cr = Board::kBoardSizeRow / 2.0;
    double cc = Board::kBoardSizeCol / 2.0;

    double sr = pos.first + (is_vertical ? ship_size / 2.0 : 0.0);
    double sc = pos.second + (is_vertical ? 0.0 : ship_size / 2.0);

    double center_dist = std::hypot(sr - cr, sc - cc);
    if (center_dist < 1.5) score -= 1.0;
    else if (center_dist < 4.0) score += 0.5;

    std::uniform_real_distribution<> noise(-0.3, 0.3);
    score += noise(gen_);

    return score;
}

Board::Position BotSonnet::findBestPlacement(const Board::ShipType ship_type,
                                             bool& is_vertical) {
    size_t ship_size = Board::get_ship_size(ship_type);

    struct PlacementCandidate {
        Board::Position pos;
        bool is_vertical;
        double score;
    };

    std::vector<PlacementCandidate> candidates;

    // Evaluate all possible placements
    for (size_t r = 0; r < Board::kBoardSizeRow; ++r) {
        for (size_t c = 0; c < Board::kBoardSizeCol; ++c) {
            Board::Position pos{static_cast<int>(r), static_cast<int>(c)};

            // Vertical placement
            if (Board::Board::is_possible_to_place_ship(board_, pos, ship_type, true)) {
                double score = evaluatePlacementPosition(pos, ship_size, true);
                candidates.push_back({pos, true, score});
            }

            // Horizontal placement
            if (Board::Board::is_possible_to_place_ship(board_, pos, ship_type, false)) {
                double score = evaluatePlacementPosition(pos, ship_size, false);
                candidates.push_back({pos, false, score});
            }
        }
    }

    // If we found candidates, pick the best ones
    if (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.score > b.score; });

        size_t top_n = std::min(candidates.size(), static_cast<size_t>(10));
        std::vector<double> weights;
        for (size_t i = 0; i < top_n; ++i) weights.push_back(std::pow(0.6, static_cast<double>(i)));
        std::discrete_distribution<size_t> dist(weights.begin(), weights.end());

        auto& chosen = candidates[dist(gen_)];
        is_vertical = chosen.is_vertical;
        return chosen.pos;
    }

    // -------------------------------------------------
    // Fallback: pick the first valid position (must never fail)
    for (size_t r = 0; r < Board::kBoardSizeRow; ++r) {
        for (size_t c = 0; c < Board::kBoardSizeCol; ++c) {
            Board::Position pos{static_cast<int>(r), static_cast<int>(c)};
            if (Board::Board::is_possible_to_place_ship(board_, pos, ship_type, true)) {
                is_vertical = true;
                return pos;
            }
            if (Board::Board::is_possible_to_place_ship(board_, pos, ship_type, false)) {
                is_vertical = false;
                return pos;
            }
        }
    }

    // Absolute last resort (should be unreachable)
    is_vertical = true;
    return {0, 0};
}

// =============================================================================
// SHOOTING
// =============================================================================

RequestId BotSonnet::makeShot() {
    Board::Position pos;

    if (mode_ == Mode::Target && !target_queue_.empty()) {
        prioritizeTargetQueue();

        while (!target_queue_.empty()) {
            pos = target_queue_.front();
            target_queue_.pop_front();
            if (isValidTarget(pos)) break;
        }

        if (!isValidTarget(pos)) {
            mode_ = Mode::Hunt;
            pos = findBestHuntTarget();
        }
    } else {
        pos = findBestHuntTarget();
    }

    if (!isValidTarget(pos)) {
        auto any = findAnyValidTarget();
        if (!any)
            throw std::runtime_error("No valid shots left");
        pos = *any;
    }

    tried_[pos.first][pos.second] = true;
    return action_interface_->makeShot(pos);
}

// =============================================================================
// TARGETING HELPERS
// =============================================================================

std::pair<bool, bool> BotSonnet::detectHitOrientation() const {
    if (current_hit_chain_.size() < 2) return {false, false};

    bool same_row = true;
    bool same_col = true;

    for (auto& p : current_hit_chain_) {
        same_row &= (p.first == current_hit_chain_[0].first);
        same_col &= (p.second == current_hit_chain_[0].second);
    }
    return {same_row, same_col}; // horizontal, vertical
}

void BotSonnet::prioritizeTargetQueue() {
    if (current_hit_chain_.size() < 2) return;

    auto [is_horizontal, is_vertical] = detectHitOrientation();
    if (!is_horizontal && !is_vertical) return;

    auto rmm = std::minmax_element(
        current_hit_chain_.begin(), current_hit_chain_.end(),
        [](auto& a, auto& b) { return a.first < b.first; });

    auto cmm = std::minmax_element(
        current_hit_chain_.begin(), current_hit_chain_.end(),
        [](auto& a, auto& b) { return a.second < b.second; });

    std::deque<Board::Position> filtered;

    for (auto& p : target_queue_) {
        if (!isValidTarget(p)) continue;

        if (is_horizontal &&
            p.first == rmm.first->first &&
            (p.second == cmm.first->second - 1 ||
             p.second == cmm.second->second + 1))
            filtered.push_back(p);

        if (is_vertical &&
            p.second == cmm.first->second &&
            (p.first == rmm.first->first - 1 ||
             p.first == rmm.second->first + 1))
            filtered.push_back(p);
    }

    if (!filtered.empty())
        target_queue_ = std::move(filtered);
}

// =============================================================================
// EVENTS
// =============================================================================

void BotSonnet::onPlayerShotResult(const Board::Position& pos,
                                   bool is_hit,
                                   bool is_ship_sunk) {
    bool hit = is_hit || is_ship_sunk;
    cell_states_[pos.first][pos.second] =
        hit ? CellState::Hit : CellState::Miss;

    if (!hit) return;

    if (is_ship_sunk)
        onShipSunk(collectHitClusterFrom(pos));
    else
        onHit(pos);
}

void BotSonnet::onHit(const Board::Position& pos) {
    mode_ = Mode::Target;
    current_hit_chain_.push_back(pos);
    enqueueNeighbors(pos);
}

void BotSonnet::onShipSunk(const std::vector<Board::Position>& sunk) {
    auto it = std::find(remaining_ship_sizes_.begin(),
                        remaining_ship_sizes_.end(), sunk.size());
    if (it != remaining_ship_sizes_.end())
        remaining_ship_sizes_.erase(it);

    for (auto& p : sunk)
        cell_states_[p.first][p.second] = CellState::Sunk;

    mode_ = Mode::Hunt;
    target_queue_.clear();
    current_hit_chain_.clear();
}

// =============================================================================
// UTILS
// =============================================================================

bool BotSonnet::isValidTarget(const Board::Position& pos) const {
    if (pos.first < 0 || pos.second < 0) return false;
    if ((size_t)pos.first >= Board::kBoardSizeRow ||
        (size_t)pos.second >= Board::kBoardSizeCol)
        return false;

    return !tried_[pos.first][pos.second] &&
           cell_states_[pos.first][pos.second] == CellState::Unknown;
}

void BotSonnet::enqueueNeighbors(const Board::Position& pos) {
    static const int dr[] = {-1, 1, 0, 0};
    static const int dc[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; ++i) {
        Board::Position next{pos.first + dr[i], pos.second + dc[i]};

        if (!isValidTarget(next))
            continue;

        bool already_queued = std::any_of(
            target_queue_.begin(),
            target_queue_.end(),
            [&next](const Board::Position& p) {
                return p.first == next.first && p.second == next.second;
            });

        if (!already_queued) {
            target_queue_.push_back(next);
        }
    }
}

std::optional<Board::Position> BotSonnet::findAnyValidTarget() const {
    for (int r = 0; r < static_cast<int>(Board::kBoardSizeRow); ++r) {
        for (int c = 0; c < static_cast<int>(Board::kBoardSizeCol); ++c) {
            Board::Position p{r, c};
            if (isValidTarget(p)) {
                return p;
            }
        }
    }
    return std::nullopt;
}

Board::Position BotSonnet::findBestHuntTarget() {
    auto prob_map = calculateProbabilityMap();

    double max_prob = -1.0;
    std::vector<Board::Position> best;

    for (size_t r = 0; r < Board::kBoardSizeRow; ++r) {
        for (size_t c = 0; c < Board::kBoardSizeCol; ++c) {
            Board::Position p{static_cast<int>(r), static_cast<int>(c)};
            if (!isValidTarget(p)) continue;

            double prob = prob_map[r][c];
            if (prob > max_prob) {
                max_prob = prob;
                best.clear();
                best.push_back(p);
            } else if (std::abs(prob - max_prob) < 1e-6) {
                best.push_back(p);
            }
        }
    }

    if (best.empty()) {
        if (auto any = findAnyValidTarget(); any.has_value())
            return *any;

        throw std::runtime_error("BotSonnet: no valid hunt targets");
    }

    std::uniform_int_distribution<size_t> pick(0, best.size() - 1);
    return best[pick(gen_)];
}

std::vector<Board::Position>
BotSonnet::collectHitClusterFrom(const Board::Position& start) const {
    std::vector<Board::Position> cluster;

    if (!isValidTarget(start) &&
        cell_states_[start.first][start.second] != CellState::Hit)
        return cluster;

    const int R = static_cast<int>(Board::kBoardSizeRow);
    const int C = static_cast<int>(Board::kBoardSizeCol);

    std::vector<std::vector<bool>> visited(
        Board::kBoardSizeRow,
        std::vector<bool>(Board::kBoardSizeCol, false));

    std::queue<Board::Position> q;
    q.push(start);
    visited[start.first][start.second] = true;

    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    while (!q.empty()) {
        auto p = q.front();
        q.pop();
        cluster.push_back(p);

        for (int i = 0; i < 4; ++i) {
            int nr = p.first + dr[i];
            int nc = p.second + dc[i];

            if (nr < 0 || nc < 0 || nr >= R || nc >= C) continue;
            if (visited[nr][nc]) continue;
            if (cell_states_[nr][nc] != CellState::Hit) continue;

            visited[nr][nc] = true;
            q.push({nr, nc});
        }
    }

    return cluster;
}

std::vector<std::vector<double>>
BotSonnet::calculateProbabilityMap() const {
    std::vector<std::vector<double>> prob_map(
        Board::kBoardSizeRow,
        std::vector<double>(Board::kBoardSizeCol, 0.0));

    for (size_t ship_size : remaining_ship_sizes_) {
        if (ship_size == 0) continue;

        // Horizontal placements
        for (size_t r = 0; r < Board::kBoardSizeRow; ++r) {
            for (size_t c = 0; c + ship_size <= Board::kBoardSizeCol; ++c) {
                bool fits = true;

                for (size_t i = 0; i < ship_size; ++i) {
                    auto state = cell_states_[r][c + i];
                    if (state == CellState::Miss ||
                        state == CellState::Sunk) {
                        fits = false;
                        break;
                    }
                }

                if (!fits) continue;

                for (size_t i = 0; i < ship_size; ++i)
                    prob_map[r][c + i] += 1.0;
            }
        }

        // Vertical placements
        if (ship_size > 1) {
            for (size_t r = 0; r + ship_size <= Board::kBoardSizeRow; ++r) {
                for (size_t c = 0; c < Board::kBoardSizeCol; ++c) {
                    bool fits = true;

                    for (size_t i = 0; i < ship_size; ++i) {
                        auto state = cell_states_[r + i][c];
                        if (state == CellState::Miss ||
                            state == CellState::Sunk) {
                            fits = false;
                            break;
                        }
                    }

                    if (!fits) continue;

                    for (size_t i = 0; i < ship_size; ++i)
                        prob_map[r + i][c] += 1.0;
                }
            }
        }
    }

    // Boost neighbors of known hits
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    for (size_t r = 0; r < Board::kBoardSizeRow; ++r) {
        for (size_t c = 0; c < Board::kBoardSizeCol; ++c) {
            if (cell_states_[r][c] == CellState::Hit) {
                for (int i = 0; i < 4; ++i) {
                    int nr = int(r) + dr[i];
                    int nc = int(c) + dc[i];

                    if (nr >= 0 && nc >= 0 &&
                        nr < int(Board::kBoardSizeRow) &&
                        nc < int(Board::kBoardSizeCol) &&
                        cell_states_[nr][nc] == CellState::Unknown) {
                        prob_map[nr][nc] *= 2.0;
                    }
                }
            }
        }
    }

    return prob_map;
}
