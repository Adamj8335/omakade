# Omakade

Omakade is a beautiful, local-first game library built for Omarchy.

The current build is the M2 preview. It discovers native and Flatpak Steam
libraries, indexes installed games locally, uses Steam artwork, and delegates
launching back to Steam. Run with `--demo` to explore the interface with a
deterministic fictional library.

Omakade follows the active Omarchy palette and font, updates after theme
changes, works offline, and never writes into Steam data. Favorites and hidden
games persist in a local SQLite database. Missing local artwork gets an
intentional procedural cover.

## Build

Requirements:

- CMake 3.24 or newer
- Ninja
- C++20 compiler
- Qt 6.8 or newer with Concurrent, Core, Gui, Network, Qml, Quick, Quick
  Controls, SQL, and Test
- SDL 3

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
./build/dev/omakade
```

Use `Ctrl+F` to search, arrow keys to navigate, Enter to open details, Escape
to return, and F11 to toggle fullscreen. `Ctrl+M` toggles reduced motion and
`Ctrl+D` opens diagnostics. Connected controllers use the D-pad or left stick,
the south face button to open, the east face button to go back, and the west
face button to favorite.

## Local data

- Library: `~/.local/share/omakade/library.sqlite3`
- Settings: `~/.config/omakade/config.toml`
- Future downloaded artwork: `~/.cache/omakade/`

Steam achievements and online metadata are intentionally deferred to M3. Core
library discovery, artwork, search, organization, controller navigation, and
launching require no Steam API key.

See [PLAN.md](PLAN.md) for product scope and release gates.
