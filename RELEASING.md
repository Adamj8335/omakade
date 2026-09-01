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
6. Create a source archive from that tag and publish its SHA-256 checksum.
7. Replace the checksum placeholder in the Arch package template, rename it to
   `PKGBUILD`, and build it with a clean Arch environment.
8. Publish short release notes from `CHANGELOG.md`.

Do not publish a package while the source URL or checksum is a placeholder.
