#include "Application.hpp"

#include <cstdio>

namespace cellflix {

int Application::run() {
    if (!events_.initialize()) {
        std::fprintf(stderr, "Could not register the system callback.\n");
        return 1;
    }
    if (!renderer_.initialize()) {
        std::fprintf(stderr, "Could not initialize Tiny3D.\n");
        return 1;
    }

    renderer_.loadBackground("/dev_hdd0/game/" CELLFLIX_APP_ID "/USRDIR/background.png");

    while (!events_.exitRequested()) {
        if (!events_.poll()) {
            std::fprintf(stderr, "Could not poll system callbacks.\n");
            return 1;
        }
        if (!events_.exitRequested()) {
            renderer_.render();
        }
    }
    return 0;
}

} // namespace cellflix
