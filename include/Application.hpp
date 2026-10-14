#pragma once

#include "SystemEvents.hpp"
#include "BackgroundRenderer.hpp"

namespace cellflix {

class Application final {
public:
    int run();

private:
    SystemEvents events_;
    BackgroundRenderer renderer_;
};

} // namespace cellflix
