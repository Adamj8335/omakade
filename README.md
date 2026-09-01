# Omakade

Omakade is a beautiful, local-first game library built for Omarchy.

The current build is the 0.4 unified-library preview. It discovers native and
Flatpak Steam and Lutris libraries, indexes installed games locally, and
delegates launching back to the owning platform. Run with `--demo` to explore
the interface with a deterministic fictional library.

Omakade follows the active Omarchy palette and font, updates after theme
changes, works offline, and never writes into Steam or Lutris data. Favorites,
hidden games, achievement details, and downloaded artwork persist in bounded
local caches. Missing Steam covers are resolved from Steam's public CDN when
available, with an intentional procedural fallback.

Steam Web API enrichment is optional. Omakade uses the Steam client's local
achievement cache without setup. A user may add a Steam Web API key through
the diagnostics panel to refresh complete schemas, unlock dates, and rarity.
The key is stored by Secret Service and never written to Omakade's config or
database.

Lutris support reads installed entries from its local `pga.db` in read-only
mode, reuses local cover art, and supports both native and Flatpak launches.
Omakade never edits Lutris game records or runner configuration.

## Build

Requirements:

- CMake 3.24 or newer
- Ninja
- C++20 compiler
- Qt 6.8 or newer with Concurrent, Core, Gui, Network, Qml, Quick, Quick
  Controls, SQL, and Test
- SDL 3
- libsecret

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
- Downloaded artwork: `~/.cache/omakade/`

Core library discovery, local achievements, artwork, search, organization,
controller navigation, and launching require no Steam API key or network
connection. See [PRIVACY.md](PRIVACY.md) for retained data and external
requests.

See [PLAN.md](PLAN.md) for product scope and release gates.
