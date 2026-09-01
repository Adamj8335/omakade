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
5. Tag the reviewed commit as `vX.Y.Z`.
6. Confirm the Release workflow builds the source archive and Arch package,
   publishes their SHA-256 checksums, and creates the GitHub release.
7. Install, upgrade, remove, and reinstall the published package in a disposable
   Omarchy environment.

Do not publish a package while the source URL or checksum is a placeholder.
