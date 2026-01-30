#pragma once

#include "player_type.h"
#include "player_interface.h"
#include "player_manager.h"
#include "game_session_api.h"
#include "player_host.h"
#include "player_host.h"
#include "user_interface.h"

#include <memory>
#include <semaphore>

namespace GameManager {

    class GameManager {
    public:
        GameManager();
        ~GameManager();

        bool startGame();
    private:
        std::shared_ptr<UserInterface::UserInterface> user_interface_;
        std::shared_ptr<UserInterface::PlayerHost> player_host_;
        std::shared_ptr<GameSession::GameSessionApi> game_session_;
        std::unique_ptr<PlayerManager::PlayerManager> player_manager_;
        std::binary_semaphore game_end_semaphore_{0};
    };

} // namespace GameManager
