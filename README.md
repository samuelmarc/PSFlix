# CellFlix

Minimal PS3 homebrew written in C++14 with PSL1GHT and Tiny3D.

## Background

Place your UI background at `pkgfiles/USRDIR` before building. Use a PNG,
preferably an opaque 16:9 image such as 1920x1080. It is stretched to fill the
active video resolution. No text, buttons, or other UI elements are drawn.

The package installs the image at
`/dev_hdd0/game/CELLFLIX1/background.png`. `pkgfiles/ICON0.PNG` remains the XMB icon.
If the background is absent or cannot be decoded, the application displays a
black screen and still processes system callbacks.

## Build and run

```sh
export PS3DEV=/usr/local/ps3dev
export PSL1GHT="$PS3DEV"
make
```

Install `build/CellFlix.pkg` on your homebrew-enabled PS3 and launch CellFlix.
After changing the image, run `make` and reinstall the package.
`make run` sends the executable through `ps3load`; the background must already
exist at the installed path above.

Hold the PS button and select the system's quit option to exit. A slot 0
`sysutils` callback receives `SYSUTIL_EXIT_GAME`, stops the main loop, and allows
the C++ destructors to finish graphics work and unregister callbacks.
Tiny3D's slot 3 callback remains active for the XMB overlay and is removed on
the exit event to prevent its immediate `exit(0)` from bypassing cleanup.

`include/SystemEvents.hpp` wraps sysutils, `include/BackgroundRenderer.hpp`
wraps Tiny3D and PNG loading, and `Application` coordinates the lifecycle.
