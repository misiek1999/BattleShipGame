#pragma once

#include <stop_token>

enum class Key {
    Char,

    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,

    Select,
    Escape,
    Cancelled,

    Exit,
    Unknown
};

struct KeyEvent {
    Key type;
    char ch{};   // valid if type == Char
};

constexpr char kEscKey = 27; // ESC key
constexpr char kExtendedKeyArrowUp = 72;
constexpr char kExtendedKeyArrowDown = 80;
constexpr char kExtendedKeyArrowLeft = 75;
constexpr char kExtendedKeyArrowRight = 77;

bool init_key_reader();

KeyEvent read_key_with_cancel(std::stop_token stop);
