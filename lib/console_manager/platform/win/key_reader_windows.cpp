#include "keycode_common.h"
#include <windows.h>
#include <stop_token>

bool init_key_reader() {
    // No special initialization needed for Windows console
    return true;
}

static KeyEvent process_key_event(const KEY_EVENT_RECORD& key) {
    switch (key.wVirtualKeyCode) {
    case VK_UP:
    case VK_NUMPAD8:
        return { Key::ArrowUp };
    case VK_DOWN:
    case VK_NUMPAD2:
        return { Key::ArrowDown };
    case VK_LEFT:
    case VK_NUMPAD4:
        return { Key::ArrowLeft };
    case VK_RIGHT:
    case VK_NUMPAD6:
        return { Key::ArrowRight };
    case VK_RETURN:
    case VK_SPACE:
        return { Key::Select };
    case VK_ESCAPE:
        return { Key::Escape };
    default:
        break;
    }

    if (key.uChar.AsciiChar != 0) {
        return { Key::Char, key.uChar.AsciiChar };
    }

    return { Key::Unknown };
}

KeyEvent read_key_with_cancel(std::stop_token stop) {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE cancelEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (cancelEvent == nullptr) {
        return { Key::Cancelled };
    }

    // Save and modify console mode
    DWORD oldMode;
    GetConsoleMode(hInput, &oldMode);
    auto newMode = (oldMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)) | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hInput, newMode);

    KeyEvent result = { Key::Cancelled };

    {
        std::stop_callback cb(stop, [cancelEvent] {
            SetEvent(cancelEvent);
        });

        HANDLE handles[2] = { cancelEvent, hInput };

        while (true) {
            DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

            if (waitResult == WAIT_OBJECT_0) {
                result = { Key::Cancelled };
                break;
            }

            if (waitResult != WAIT_OBJECT_0 + 1) {
                result = { Key::Cancelled };
                break;
            }

            INPUT_RECORD rec;
            DWORD read;
            ReadConsoleInput(hInput, &rec, 1, &read);

            if (rec.EventType != KEY_EVENT)
                continue;

            const KEY_EVENT_RECORD& key = rec.Event.KeyEvent;

            if (!key.bKeyDown)
                continue;

            result = process_key_event(key);
            break;
        }
    }

    // Cleanup
    SetConsoleMode(hInput, oldMode);
    CloseHandle(cancelEvent);
    return result;
}
