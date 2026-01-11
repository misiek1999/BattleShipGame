#pragma once

#include <variant>
#include <cassert>
#include "game_types.h"
#include "board.h"

namespace GameSession {

    enum class RequestResult {
        Ok,
        InvalidRequest,
        InvalidArguments,
        RequestRejected,
        NotPlayerTurn,
        GameNotStarted,
        RoundFinished,
        OperationFailed
    };

    enum class GameActionType : uint8_t {
        PlaceShip = 0,
        MakeShot,
        NotifyReadyForNextRound,
        Disconnect,
        StopGame,
        StartNewGame,
        RestartGame,
        SendChatMessage,
        Surrender,
        PauseGame,
        ResumeGame,
        RequestPlayerBoard,
        RequestOpponentBoard,
        RequestGameStatus,
        RequestScore,
        RequestShipsCount,
        RequestOponentShipsCount,
        RequestCurrentTurnPlayer,
        LastActionType = RequestCurrentTurnPlayer
    };

    struct PlaceShipActionArg {
        Board::ShipType ship_type;
        Board::Position position;
        bool is_horizontal;
    };

    struct MakeShotActionArg {
        Board::Position position;
    };

    using GameActionArg = std::variant<
        PlaceShipActionArg,
        MakeShotActionArg
    >;

    struct GameActionEvent {
        RequestId request_id = {};
        PlayerType player_id = {};
        GameActionType action_type = {};
        GameActionArg action_arg = {};
    };

        // Helper functions
    constexpr const char* to_cstring(const GameActionType action) noexcept {
        switch (action) {
            case GameActionType::PlaceShip:                return "PlaceShip";
            case GameActionType::MakeShot:                 return "MakeShot";
            case GameActionType::NotifyReadyForNextRound:  return "NotifyReadyForNextRound";
            case GameActionType::Disconnect:               return "Disconnect";
            case GameActionType::StopGame:                 return "StopGame";
            case GameActionType::StartNewGame:             return "StartNewGame";
            case GameActionType::RestartGame:              return "RestartGame";
            case GameActionType::SendChatMessage:          return "SendChatMessage";
            case GameActionType::Surrender:                return "Surrender";
            case GameActionType::PauseGame:                return "PauseGame";
            case GameActionType::ResumeGame:               return "ResumeGame";
            case GameActionType::RequestPlayerBoard:       return "RequestPlayerBoard";
            case GameActionType::RequestOpponentBoard:     return "RequestOpponentBoard";
            case GameActionType::RequestGameStatus:        return "RequestGameStatus";
            case GameActionType::RequestScore:             return "RequestScore";
            case GameActionType::RequestShipsCount:        return "RequestShipsCount";
            case GameActionType::RequestOponentShipsCount: return "RequestOponentShipsCount";
            case GameActionType::RequestCurrentTurnPlayer: return "RequestCurrentTurnPlayer";
        }

        assert(false && "Unknown GameActionType");
        return "Unknown";
    }

    constexpr const char* to_cstring(const RequestResult result) noexcept {
        switch (result) {
            case RequestResult::Ok:                 return "OK";
            case RequestResult::InvalidRequest:     return "InvalidRequest";
            case RequestResult::RequestRejected:    return "RequestRejected";
            case RequestResult::InvalidArguments:   return "InvalidArguments";
            case RequestResult::NotPlayerTurn:      return "NotPlayerTurn";
            case RequestResult::GameNotStarted:     return "GameNotStarted";
            case RequestResult::RoundFinished:       return "RoundFinished";
            case RequestResult::OperationFailed:    return "OperationFailed";
        }

        assert(false && "Unknown RequestResult");
        return "Unknown";
    }

}   // namespace GameSession
