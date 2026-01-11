#include "player_bot.h"
#include "player_interface.h"
#include "bot_factory.h"

namespace Player
{

    class PlayerBotImpl : public Player::IPlayer
    {
    public:
        PlayerBotImpl(std::unique_ptr<IBotFactory> factory, std::shared_ptr<GameSession::IGamePlayerAction>& action_interface, const PlayerType player_type) {
            bot_algorithm_ = factory->createBot(action_interface, player_type);
        }

        ~PlayerBotImpl() = default;

        PlayerType getPlayerType() const override {
            return bot_algorithm_->getPlayerType();
        }

        bool isReady() const override {
            return bot_algorithm_->isReady();
        }

        bool isConnected() const override {
            return bot_algorithm_->isConnected();
        }

        Board::BoardType getBoard() const override {
            return bot_algorithm_->getBoard();
        }

        // IPlayerCallbacks interface implementation
        void onRequestResult(const RequestId req_id, const GameSession::RequestResult result) override {
            bot_algorithm_->onRequestResult(req_id, result);
        }

        void onPlayerShotResult(const Board::Position& position, const bool is_hit,
            const bool is_ship_sunk) override {
            bot_algorithm_->onPlayerShotResult(position, is_hit, is_ship_sunk);
        }

        void onOpponentShotResult(const Board::Position& position, const bool is_hit,
                                const bool is_ship_sunk) override {
            bot_algorithm_->onOpponentShotResult(position, is_hit, is_ship_sunk);
        }

        void onGameFinished() override {
            bot_algorithm_->onGameFinished();
        }

        void onRoundEnded(const GameEngine::RoundResult round_result) override {
            bot_algorithm_->onRoundEnded(round_result);
        }

        void onScoreUpdated(const int player_score, const int opponent_score) override {
            bot_algorithm_->onScoreUpdated(player_score, opponent_score);
        }

        void onBoardReceived(const Board::BoardType board) override {
            bot_algorithm_->onBoardReceived(board);
        }

        void onOponentBoardReceived(const Board::BoardType board) override {
            bot_algorithm_->onOponentBoardReceived(board);
        }

        void onShipsCountReceived(const Board::ShipCountMap ships_count) override {
            bot_algorithm_->onShipsCountReceived(ships_count);
        }

        void onOponentShipsCountReceived(const Board::ShipCountMap oponent_ships_count) override {
            bot_algorithm_->onOponentShipsCountReceived(oponent_ships_count);
        }

        void onPlayerTurnNotify() override {
            bot_algorithm_->onPlayerTurnNotify();
        }

        void onNewPlayerTurnReceived(const PlayerType player_turn) override {
            bot_algorithm_->onNewPlayerTurnReceived(player_turn);
        }

        void onGameStatusReceived(const GameEngine::GameStatus game_status) override {
            bot_algorithm_->onGameStatusReceived(game_status);
        }

        void onCallbackActivation() override {
            bot_algorithm_->onCallbackActivation();
        }

    private:
        std::unique_ptr<IPlayerBot> bot_algorithm_;
    };

    PlayerBot::PlayerBot(std::unique_ptr<IBotFactory>& factory, std::shared_ptr<GameSession::IGamePlayerAction>& action_interface, const PlayerType player_type):
        impl_(std::make_unique<PlayerBotImpl>(std::move(factory), action_interface, player_type)) {
    }

} // namespace Player
