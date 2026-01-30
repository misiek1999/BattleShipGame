#pragma once

#include <utility>
#include <memory>

#include "player_type.h"
#include "player_interface.h"
#include "game_session_api.h"

namespace PlayerManager {

    class IPlayerManager {
    public:
        virtual ~IPlayerManager() = default;
        // Get host and guest clients instances
        virtual std::shared_ptr<Player::IPlayer> getHostClient() = 0;
        virtual std::shared_ptr<Player::IPlayer> getGuestClient() = 0;
        // get player type
        virtual OponentPlayerType getGuestPlayerType() = 0;
    };

    class PlayerManager : public IPlayerManager {
    public:
        class Builder;
        ~PlayerManager();
        // Get host and guest clients instances
        std::shared_ptr<Player::IPlayer> getHostClient() override;
        std::shared_ptr<Player::IPlayer> getGuestClient() override;

        OponentPlayerType getGuestPlayerType() override;

        PlayerManager(std::shared_ptr<GameSession::GameSessionApi> game_session,
                      std::shared_ptr<Player::IPlayer> host,
                      std::shared_ptr<Player::IPlayer> guest,
                      OponentPlayerType guest_type);

    private:
        std::shared_ptr<GameSession::GameSessionApi> game_session_;
        std::shared_ptr<Player::IPlayer> host_;
        std::shared_ptr<Player::IPlayer> guest_;
        OponentPlayerType guest_type_;
    };


}   // namespace PlayerManager
