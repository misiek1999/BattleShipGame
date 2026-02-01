#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <algorithm>

#include "log.h"
#include "game_manager.h"

namespace {

void print_usage(const char* exe) {
    std::cout
        << "Usage: " << exe << " [options]\n"
        << "Options:\n"
        << "  -v                 Dump log to file\n"
        << "  -b, --bot <type>    Bot type: random | hunter | gpt | claude\n"
        << "  --bot=<type>        Same as above\n"
        << "  -h, --help          Show this help\n";
}

std::string to_lower(std::string_view sv) {
    std::string s(sv);
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

std::optional<BotType> parse_bot_type(std::string_view sv) {
    const std::string s = to_lower(sv);
    if (s == "random") return BotType::Random;
    if (s == "hunter") return BotType::Hunter;
    if (s == "gpt")    return BotType::GPT;
    if (s == "claude") return BotType::Claude;
    return std::nullopt;
}

} // namespace

int main(int argc, char* argv[]) {
    bool dump_log_to_file{false};
    std::optional<BotType> bot_type; // not set => GameManager uses its default

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (arg == "-v") {
            dump_log_to_file = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (arg == "-b" || arg == "--bot") {
            if (i + 1 >= argc) {
                std::cerr << "Error: missing value for " << arg << "\n";
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            auto parsed = parse_bot_type(argv[++i]);
            if (!parsed) {
                std::cerr << "Error: invalid bot type: " << argv[i] << "\n";
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            bot_type = *parsed;
        } else if (arg.rfind("--bot=", 0) == 0) {
            auto value = arg.substr(std::string_view("--bot=").size());
            auto parsed = parse_bot_type(value);
            if (!parsed) {
                std::cerr << "Error: invalid bot type: " << value << "\n";
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            bot_type = *parsed;
        } else {
            std::cerr << "Error: unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    init_logger(dump_log_to_file);

    try {
        GameManager::GameManager game_manager(bot_type.value_or(BotType::GPT));
        game_manager.startGame();
    } catch (const std::exception& e) {
        LOG_E("Exception caught in main: {}", e.what());
        return EXIT_FAILURE;
    }

    LOG_I("Main function exit. Close game.");
    return 0;
}
