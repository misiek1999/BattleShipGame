#include "game_manager.h"
#include "log.h"
#include "player_manager_builder.h"

GameManager::GameManager::GameManager() {
    user_interface_ = std::make_shared<UserInterface::UserInterface>(game_end_semaphore_);
    player_host_ = std::make_shared<UserInterface::PlayerHost>(user_interface_);
    game_session_ = std::make_shared<GameSession::GameSessionApi>();
    player_manager_ = PlayerManager::PlayerManager::Builder(game_session_)
                                        .addHost(player_host_)
                                        .createGuestType(OponentPlayerType::Bot)
                                        .addBotType(BotType::Hunter)
                                        .build();
    user_interface_->setHostPlayerInterface(player_host_);
    user_interface_->startInterface();
    LOG_D("GameManager created with host player");
}

GameManager::GameManager::~GameManager() {
    game_session_->stopGame();
    LOG_D("GameManager destroyed, game session stopped.");
}

bool GameManager::GameManager::startGame() {
    LOG_I("Starting game...");
    game_end_semaphore_.acquire();
    // We are leaving here at game end
    return true;
}