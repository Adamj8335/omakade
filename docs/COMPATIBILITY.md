# Compatibility report

## Reference Omarchy system

Verified on August 31, 2026:

| Component | Version | Result |
| --- | --- | --- |
| Omarchy | 4.0.0.r1979.gb686ed8-1 | Pass |
| Hyprland | 0.56.2 | Pass |
| Linux | 7.1.11-arch1-1 | Pass |
| Qt | 6.11.2 | Pass |
| SDL | 3.4.14 | Pass |
| Native Steam | 1.0.0.87-3 | Library, artwork, launch delegation, and local achievements pass |

The reference library contains 45 installed Steam games. Theme colors, font,
launcher transparency, one-click details, keyboard navigation, and the
controller input path have been exercised on this system.

## Contract-tested sources

Lutris native and Flatpak discovery, Heroic native and Flatpak discovery, and
Epic, GOG, and Amazon manifests are covered by repeatable local fixtures. These
paths still need reports from users with those launchers installed before the
stable release gate can close.

## Still needed

- A clean Omarchy installation
- A second display scale
- Native and Flatpak Lutris libraries from real users
- Native and Flatpak Heroic libraries from real users
- Steam Flatpak from a real user
- Blur disabled and a light Omarchy theme

Reports should follow [SUPPORT.md](../SUPPORT.md) and must not include secrets.
