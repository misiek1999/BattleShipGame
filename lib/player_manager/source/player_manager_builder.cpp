#include "player_manager_builder.h"
#include "bot_factory.h"
#include "player_bot.h"

PlayerManager::PlayerManager::Builder::Builder(std::shared_ptr<GameSession::GameSessionApi>& game_session)
    : game_session_(game_session) {
}

PlayerManager::PlayerManager::Builder& PlayerManager::PlayerManager::Builder::addHost(std::shared_ptr<UserInterface::PlayerHost> host) {
    if (host == nullptr) {
        throw std::runtime_error("Host argument is null");
    }
    if (host_.has_value()) {
        throw std::runtime_error("Host player already set");
    }
    std::shared_ptr<GameSession::IGamePlayerAction> player_action = std::make_shared<GameSession::GamePlayerAction>(game_session_, PlayerType::Player_1);
    host->setActionHandler(player_action);
    host_ = std::static_pointer_cast<Player::IPlayer>(host);
    return *this;
}

PlayerManager::PlayerManager::Builder& PlayerManager::PlayerManager::Builder::createGuestType(const OponentPlayerType type) {
    guest_type_ = type;
    guest_ = createGuestPlayer(type);
    LOG_V("Created oponenent for type: {}", to_cstring(type));
    return *this;
}

PlayerManager::PlayerManager::Builder &PlayerManager::PlayerManager::Builder::addBotType(const BotType type) {
    if (guest_type_ != OponentPlayerType::Bot) {
        throw std::logic_error("Please specify bot type first");
    }
    bot_type_ = type;
    return *this;
}

std::unique_ptr<PlayerManager::PlayerManager> PlayerManager::PlayerManager::Builder::build() {
    if (!guest_type_.has_value()) {
        LOG_I("Guest not specified. Use bot");
        const auto type = OponentPlayerType::Bot;
        guest_type_ = type;
        guest_ = createGuestPlayer(type);
    }

    if (!guest_.has_value()) {
        throw std::logic_error("Guest player instance must be created");
    }

    if (!host_.has_value()) {
        host_ = createHostPlayer();
    }

    return std::make_unique<PlayerManager>(game_session_, host_.value(), guest_.value(), guest_type_.value());
}

// we will use a random bot as host player when host player is not specified
std::shared_ptr<Player::IPlayer> PlayerManager::PlayerManager::Builder::createHostPlayer() {
    return createBotPlayer(PlayerType::Player_1, BotType::Random);
}

std::shared_ptr<Player::IPlayer> PlayerManager::PlayerManager::Builder::createGuestPlayer(const OponentPlayerType oponent_type) {
    std::shared_ptr<Player::IPlayer> oponent_player;
    switch (oponent_type) {
        case OponentPlayerType::Bot: {
            oponent_player = createOponentBotPlayer();
            break;
        }
        case OponentPlayerType::Human:
        [[fallthrough]];
        default: {
            throw std::logic_error("Player not supported");
        }
    }
    return oponent_player;
}

std::shared_ptr<Player::IPlayer> PlayerManager::PlayerManager::Builder::createOponentBotPlayer() {
    const auto bot_type = bot_type_.value_or(BotType::Random);
    return createBotPlayer(PlayerType::Player_2, bot_type);
}

std::shared_ptr<Player::IPlayer> PlayerManager::PlayerManager::Builder::createBotPlayer(const PlayerType player_type, const BotType bot_type) {
    std::shared_ptr<GameSession::IGamePlayerAction> player_action = std::make_shared<GameSession::GamePlayerAction>(game_session_, player_type);
    std::unique_ptr<IBotFactory> bot_factory;
    switch (bot_type) {
        case BotType::Random:
            bot_factory = std::make_unique<BotFactoryRandom>();
            break;
        case BotType::Smart:
            [[fallthrough]];
        default: {
            throw std::logic_error("Bot type not supported");
        }
    }
    std::shared_ptr<Player::IPlayer> bot = std::make_shared<Player::PlayerBot>(bot_factory, player_action, player_type);
    return bot;
}
