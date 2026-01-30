#pragma once

#include <string>

namespace UserInterface
{
    class IUserInterfaceCallback
    {
    public:
        virtual ~IUserInterfaceCallback() = default;

        virtual void onGameClosed() = 0;
        virtual void onMoveUp() =0;
        virtual void onMoveDown() =0;
        virtual void onMoveLeft() =0;
        virtual void onMoveRight() =0;
        virtual void onSelect() =0;
        virtual void onCancel() =0;
        virtual void onChar(const char c) =0;
    };
} // namespace UserInterface
