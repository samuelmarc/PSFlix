/*
* Copyright (C) 2026 samuelmarc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

    renderer_.loadBackground("/dev_hdd0/game/" DPSFlix_APP_ID "/USRDIR/background.png");

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
