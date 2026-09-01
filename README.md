# Omakade

[![CI](https://github.com/tsouth89/omakade/actions/workflows/ci.yml/badge.svg)](https://github.com/tsouth89/omakade/actions/workflows/ci.yml)
[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-8cd3cb.svg)](LICENSE)

**Your games, beautifully together.**

Omakade is a fast, local-first game library built for Omarchy. It brings
installed Steam, Lutris, Epic, GOG, and Amazon games into one quiet,
cover-focused home that follows the active Omarchy theme.

[Project homepage](https://tsouth89.github.io/omakade/) ·
[Roadmap](PLAN.md) · [Support](SUPPORT.md)

> Omakade is an independent community project. It is not an official Omarchy
> application.

## Current preview

The 0.5 preview includes:

- Native and Flatpak Steam, Lutris, and Heroic discovery
- One-click details and delegated launching through the owning platform
- Omarchy palette, font, transparency, and live theme updates
- Search, favorites, hidden games, sorting, and source filters
- Local Steam achievements plus optional Web API enrichment
- Local, downloaded, and user-selected cover artwork
- Explicit linking for games installed through multiple sources
- Keyboard, mouse, and controller navigation

Omakade reads launcher data without modifying it. Core discovery, browsing,
artwork, and launching work offline. Run `omakade --demo` to explore the UI
with a deterministic fictional library.

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
`Ctrl+D` opens diagnostics.

## Local data

- Library: `~/.local/share/omakade/library.sqlite3`
- Settings: `~/.config/omakade/config.toml`
- Downloaded artwork: `~/.cache/omakade/`
- Selected custom covers: `~/.local/share/omakade/artwork/`

Core library discovery, local achievements, artwork, search, organization,
controller navigation, and launching require no Steam API key or network
connection. Optional Steam credentials are stored by Secret Service.

See [PRIVACY.md](PRIVACY.md) for retained data and external requests,
[CHANGELOG.md](CHANGELOG.md) for release notes, and the current
[compatibility report](docs/COMPATIBILITY.md) for tested platform layouts.
