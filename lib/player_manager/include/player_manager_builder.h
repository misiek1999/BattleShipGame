#pragma once

#include <memory>
#include <optional>
#include "player_type.h"
#include "player_interface.h"
#include "player_manager.h"

namespace PlayerManager {

    class PlayerManager::Builder {
    public:
        explicit Builder(std::shared_ptr<GameSession::GameSessionApi> game_session);
        Builder& addHost(std::shared_ptr<Player::IPlayer> host);

        Builder& addGuest(std::shared_ptr<Player::IPlayer> guest, const OponentPlayerType type);

        Builder& createGuestType(const OponentPlayerType type);

        Builder& addBotType(const BotType type);

        PlayerManager build();
    private:
        std::shared_ptr<GameSession::GameSessionApi> game_session_;
        std::optional<std::shared_ptr<Player::IPlayer>> guest_;
        std::optional<std::shared_ptr<Player::IPlayer>> host_;
        std::optional<OponentPlayerType> guest_type_;
        std::optional<BotType> bot_type_;

        std::shared_ptr<Player::IPlayer> createHostPlayer();
        std::shared_ptr<Player::IPlayer> createGuestPlayer(const OponentPlayerType oponent_type);
        std::shared_ptr<Player::IPlayer> createOponentBotPlayer();
        std::shared_ptr<Player::IPlayer> createBotPlayer(const PlayerType player_type, const BotType bot_type);
    };
}   // namespace PlayerManager
