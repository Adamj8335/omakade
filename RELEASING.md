# Releasing Omakade

1. Update the version in `CMakeLists.txt`, AppStream metadata, and the changelog.
2. Run the release configure, build, and tests:

   ```bash
   cmake --preset release
   cmake --build --preset release
   ctest --preset release
   ```

3. Validate the desktop and AppStream files.
4. Install into an empty staging directory and inspect every installed file.
   Render visual fixtures without opening a desktop window when needed:

   ```bash
   QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
     ./build/release/omakade --render-screenshot=/tmp/omakade.png \
     --render-size=1380x880
   ```
5. Tag the reviewed commit as `vX.Y.Z`.
6. Confirm the Release workflow builds the source archive and Arch package,
   installs, launches, reinstalls, removes, and reinstalls the package, publishes
   SHA-256 checksums, and creates the GitHub release.
7. Install, upgrade, remove, and reinstall the published package in a disposable
   Omarchy environment.

Do not publish a package while the source URL or checksum is a placeholder.
