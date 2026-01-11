#include <iostream>
#include <memory>

#include "log.h"
#include "board.h"
#include "game_engine.h"
#include "player_interface.h"
#include "game_session_api.h"
#include "game_player_action.h"
#include "bot_factory.h"
#include "player_bot.h"

int main(int, char**){
    // Initialize logger
    init_logger();

    std::shared_ptr<GameSession::GameSessionApi> game_session = std::make_shared<GameSession::GameSessionApi>();

    GameSession::GamePlayerAction player1_action(game_session, PlayerType::Player_1);
    std::shared_ptr<GameSession::IGamePlayerAction> player2_action = std::make_shared<GameSession::GamePlayerAction>(game_session, PlayerType::Player_2);

    player1_action.placeShip(Board::ShipType::Destroyer, Board::Position{0,0}, true);

    std::unique_ptr<IBotFactory> bot_factory = std::make_unique<BotFactoryRandom>();
    std::shared_ptr<Player::IPlayer> bot = std::make_shared<Player::PlayerBot>(bot_factory, player2_action, PlayerType::Player_2);

    game_session->registerPlayer(PlayerType::Player_2, bot);

    game_session->resetSession();
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    return 0;
}
