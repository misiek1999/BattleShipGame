#include "keycode_common.h"
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <stop_token>
#include <atomic>

static std::atomic<bool> g_sigint{false};

static void sigint_handler(int) {
    g_sigint.store(true);
}

bool init_key_reader() {
    signal(SIGINT, sigint_handler);
    return true;
}

class RawTerminal {
    termios old_{};
public:
    RawTerminal() {
        tcgetattr(STDIN_FILENO, &old_);
        termios raw = old_;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }
    ~RawTerminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_);
    }
};

KeyEvent read_key_with_cancel(std::stop_token stop) {
    RawTerminal rt;

    int cancel_pipe[2];
    pipe(cancel_pipe);

    std::stop_callback cb(stop, [&] {
        write(cancel_pipe[1], "x", 1);
    });

    pollfd fds[2] = {
        { STDIN_FILENO, POLLIN, 0 },
        { cancel_pipe[0], POLLIN, 0 }
    };

    while (true) {
        poll(fds, 2, -1);

        if (fds[1].revents & POLLIN)
            return { Key::Exit };

        if (fds[0].revents & POLLIN) {
            char buf[3];
            int n = read(STDIN_FILENO, buf, sizeof(buf));

            if (g_sigint.exchange(false)) {
                return { Key::Exit };
            }

            if (n == 1) {
                switch (buf[0]) {
                    case 0x03:
                        return { Key::Exit };

                    case '\n':
                    case '\r':
                    case ' ':
                        return { Key::Select };

                    case 0x1b:
                        return { Key::Escape };

                    default:
                        return { Key::Char, buf[0] };
                }
            }

            if (n == 3 && buf[0] == 0x1b && buf[1] == '[') {
                switch (buf[2]) {
                    case 'A': return { Key::ArrowUp };
                    case 'B': return { Key::ArrowDown };
                    case 'C': return { Key::ArrowRight };
                    case 'D': return { Key::ArrowLeft };
                }
            }
            return { Key::Unknown };
        }
    }
}
