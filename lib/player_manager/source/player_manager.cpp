#include "player_manager.h"
#include "log.h"

namespace PlayerManager
{

    PlayerManager::PlayerManager(std::shared_ptr<GameSession::GameSessionApi> game_session,
                      std::shared_ptr<Player::IPlayer> host,
                      std::shared_ptr<Player::IPlayer> guest,
                      OponentPlayerType guest_type)
            : game_session_(game_session)
            , host_(host)
            , guest_(guest)
            , guest_type_(guest_type) {
        LOG_V("Create player manager");
        game_session_->registerPlayer(PlayerType::Player_1, host_);
        game_session_->registerPlayer(PlayerType::Player_2, guest_);

    };

    PlayerManager::~PlayerManager() {
        game_session_->unregisterPlayer(PlayerType::Player_1);
        game_session_->unregisterPlayer(PlayerType::Player_2);
    }

    std::shared_ptr<Player::IPlayer> PlayerManager::getHostClient()
    {
        return host_;
    }

    std::shared_ptr<Player::IPlayer> PlayerManager::getGuestClient() {
        return guest_;
    }

    OponentPlayerType PlayerManager::getGuestPlayerType() {
        return guest_type_;
    }

} // namespace Player
