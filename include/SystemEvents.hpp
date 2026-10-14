#pragma once

#include <sysutil/sysutil.h>

namespace cellflix {

class SystemEvents final {
public:
    SystemEvents() = default;
    SystemEvents(const SystemEvents&) = delete;
    SystemEvents& operator=(const SystemEvents&) = delete;

    ~SystemEvents() {
        if (registered_) {
            sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
        }
    }

    bool initialize() {
        registered_ = sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, callback, this) == SYSUTIL_OK;
        return registered_;
    }

    bool poll() {
        return sysUtilCheckCallback() == SYSUTIL_OK;
    }

    bool exitRequested() const { return exitRequested_; }

private:
    static void callback(u64 status, u64, void* userData) {
        auto& events = *static_cast<SystemEvents*>(userData);
        if (status == SYSUTIL_EXIT_GAME) {
            events.exitRequested_ = true;
            // Tiny3D registers an exit(0) callback in slot 3. Slot 0 runs first,
            // so remove it on exit to let the application unwind normally.
            // Keep it for other events so Tiny3D still handles the XMB overlay.
            sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT3);
        }
    }

    bool registered_ = false;
    bool exitRequested_ = false;
};

} // namespace cellflix
