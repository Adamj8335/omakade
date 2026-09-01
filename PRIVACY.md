# Omakade privacy

Omakade is local-first. It has no analytics, advertising, telemetry, account
system, or required online service.

## Local data

Omakade reads Steam library manifests, artwork caches, playtime, recent-play
state, and achievement caches. It also reads installed-game manifests and
cached artwork from Lutris and Heroic. It never writes into Steam, Lutris, or
Heroic directories.

Omakade retains:

- Library, source records, favorites, hidden state, and achievements in
  `$XDG_DATA_HOME/omakade/library.sqlite3`
- User-created links between duplicate installations in the same database
- Steam ID, cache limit, and reduced-motion preference in
  `$XDG_CONFIG_HOME/omakade/config.toml`
- Downloaded covers and achievement icons in `$XDG_CACHE_HOME/omakade/`
- Copies of covers selected by the user in `$XDG_DATA_HOME/omakade/artwork/`

The Steam ID is an account identifier, not a credential. A Steam Web API key
is stored only through the desktop Secret Service under
`io.github.tsouth89.Omakade.Steam`. Older preview keys stored as
`io.github.omakade.Steam` remain readable. The key is never written to Omakade's config, database,
logs, or process arguments.

## Network requests

Omakade may request missing covers and achievement icons from Steam's public
HTTPS artwork hosts. Responses are size-limited and the artwork cache is
bounded by the configured limit.

Steam Web API requests occur only after the user stores a key and explicitly
selects Refresh Steam for a game. Omakade requests player achievements, the
game's achievement schema, and global rarity from Valve's documented HTTPS
endpoints. Failed requests do not remove cached data.

## Removal

The diagnostics panel can clear downloaded achievement art and remove the API
key from Secret Service. Removing Omakade does not remove its XDG data by
default, so users can preserve settings across reinstallations.
Resetting a custom cover removes Omakade's private copy and restores the
source-provided artwork. It does not change the original selected image.
