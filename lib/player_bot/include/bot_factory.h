#pragma once

#include "board.h"
#include "bot_interface.h"
#include "game_player_action.h"
#include "bot_interface.h"
#include "bot_random.h"
// #include "bot_algorithm.h"

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

//TODO: uncomment when bot algorithm will be implemented
// class BotFactoryAlgorithm : public IBotFactory {
// public:
//     BotFactoryAlgorithm() = default;
//     inline virtual std::unique_ptr<IPlayerBot> createBot(std::shared_ptr<GameSession::IGamePlayerAction> action_interface,
//               const PlayerType player_type) override {
//         return std::make_unique<BotAlgorithm>(action_interface, player_type);
//     }
// };
