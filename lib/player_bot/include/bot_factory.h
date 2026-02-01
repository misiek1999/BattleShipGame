#pragma once

#include "board.h"
#include "bot_interface.h"
#include "game_player_action.h"
#include "bot_interface.h"
#include "bot_random.h"
#include "bot_hunter.h"
#include "bot_gpt.h"
#include "bot_sonnet.h"

#include <memory>

class IBotFactory {
public:
    virtual ~IBotFactory() = default;
    virtual std::unique_ptr<IPlayerBot> createBot(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type) = 0;
};

class BotFactoryRandom : public IBotFactory {
public:
    BotFactoryRandom() = default;
    ~BotFactoryRandom() override = default;
    inline virtual std::unique_ptr<IPlayerBot> createBot(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type) override {
        return std::make_unique<BotRandom>(action_interface, player_type);
    }
};

class BotFactoryHunter : public IBotFactory {
public:
    BotFactoryHunter() = default;
    inline virtual std::unique_ptr<IPlayerBot> createBot(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type) override {
        return std::make_unique<BotHunter>(action_interface, player_type);
    }
};

class BotFactoryGpt : public IBotFactory {
public:
    BotFactoryGpt() = default;
    inline virtual std::unique_ptr<IPlayerBot> createBot(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type) override {
        return std::make_unique<BotGpt>(action_interface, player_type);
    }
};

class BotFactorySonnet : public IBotFactory {
public:
    BotFactorySonnet() = default;
    inline virtual std::unique_ptr<IPlayerBot> createBot(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
              const PlayerType player_type) override {
        return std::make_unique<BotSonnet>(action_interface, player_type);
    }
};
