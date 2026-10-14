# Stapik K-Pop

Retro-styled desktop manager of a K-pop collection, written in C++20 with GTK4 (gtkmm) on top of
[stapik-common](https://github.com/Stapik-Group/stapik-common).

## What can be collected

| Kind | Details |
| --- | --- |
| Albums | type, format (CD, vinyl, cassette, digital, platform album, kit), edition, variant, release date, label, region, catalog number, inclusions, tracklist (title, writers, length, title track) |
| Photocards | member, origin (album, pre-order, lucky draw, fansign, event), source, available for trade |
| Merchandise | lightsticks, apparel, photobooks, season's greetings, posters, keyrings, stationery |
| Clips | type, platform, link, release date, album, watched |
| Lyrics | original text, romanization, translation (searchable) |
| Events | concert, fansign, fan meeting, listening party, pop-up store, venue, city, seat |

Every entry also has an artist, a status (owned, ordered, wishlist, sold), a condition, a number of copies,
a price in any currency, an acquisition date and place, and notes. Dates may be partial (`2021`, `2021-08`, `2021-08-23`).

Features coming from stapik-common: undo/redo of every edit, themes, Polish/English/German, atomic saving, restoring
from backups and optional synchronisation with your own Stapik Cloud. The program looks for a newer release on GitHub
once a day and offers to open its download page; nothing is downloaded or installed, and the check can be switched off
in *Settings → Settings…* (or with `update.checkEnabled` in the settings file).

## Building

Requirements: a C++20 compiler, CMake 4.2+, gtkmm-4.0, libcurl, libsodium, zlib, Ninja (optional). On Linux,
libsecret (keyring for the cloud key) and libx11 are used when present. On Ubuntu/Debian:
`libgtkmm-4.0-dev libcurl4-openssl-dev libsodium-dev zlib1g-dev libsecret-1-dev libx11-dev`.

```sh
cmake -S . -B build -G Ninja -DSTAPIK_KPOP_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/stapikkpop
```

To develop against a local checkout of stapik-common:

```sh
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_STAPIKCOMMON=../stapik-common
```

## Windows

The Windows build is a folder with the program and everything it needs (GTK, its data, the application resources),
distributed as an Inno Setup installer and as a portable zip.

* **From CI:** a pull request from `develop` to `main`/`master` builds and tests the program on Linux and Windows using
  the reusable workflows of stapik-common (`.github/workflows/pull-request.yml`). Publishing a GitHub release runs
  `release.yml`, which attaches the installer, the portable zip and a `.deb` for every supported image to that release
  (the Windows files are signed when the repository has the `WINDOWS_SIGN_PFX_*` secrets).
* **Locally**, in an MSYS2 UCRT64 shell (packages: `gcc cmake ninja pkgconf gtkmm-4.0 curl libsodium zlib
  adwaita-icon-theme hicolor-icon-theme librsvg`, prefixed `mingw-w64-ucrt-x86_64-`):

  ```sh
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build build --target stapikkpop_windows_portable
  ```

  The folder is `build/stapikkpop-windows/` and the zip is next to it. Add `-DSTAPIK_WINDOWS_CONSOLE=ON` to keep a console window.

Data lives in `%APPDATA%\stapikkpop\data\collection.json`, settings in `%APPDATA%\stapikkpop\config` and the log
(stderr of the GUI program) in `%LOCALAPPDATA%\stapikkpop\cache\stapik.log`. The program draws with the Cairo
renderer by default; set `GSK_RENDERER=gl` to use the GPU. `packaging/windows/README.txt` is the note for testers that
goes into the zip.

`--self-test` is also available on Linux; it checks icons, GSettings schemas, resources, translations and a writable
data directory, logs every check and exits with 1 when one of them fails.

To change the stapik-common version: `-DSTAPIK_COMMON_GIT_TAG=<tag, branch or commit>`.

## Linux package

The release workflow builds `stapikkpop-<version>-Linux-<image>.deb` for `ubuntu:24.04` and `debian:13` (CPack,
installed under `/opt/stapikkpop` with a launcher in `/usr/bin`, a `.desktop` entry, the AppStream metainfo and the icon).
It needs Ubuntu 24.04 or newer (or Debian 13). Install with `sudo apt install ./<file>.deb`. Locally:
`cmake --build build` and then `cd build && cpack -G DEB -D CPACK_DEBIAN_FILE_NAME=DEB-DEFAULT`.

## Layout

```
src/kpop/domain     entries, enumerations with stable storage ids, partial dates, filtering, statistics
src/kpop/document   CollectionDocument (the syncable document), JSON serializer, schema version
src/kpop/command    undoable commands
src/kpop/app        CollectionController: edits, saving, cloud sync
src/kpop/ui         GTK widgets, dialogs and the main window
resources           locales (en, pl, de) and the application part of the themes
tests               unit tests of everything that does not need a window
```

Everything except `src/kpop/ui` is part of the `kpopcore` library and is unit tested.

## Data

The collection lives in `collection.json` in the user data directory (`~/.local/share/stapikkpop/`).
Storage ids of enumerations (for example `luckyDraw`) never change once released; a file written by a newer
version is never overwritten by an older one.

## Adding a kind of entry

1. Add the value to `ItemKind` and an alternative to `ItemDetails` (same position) in `src/kpop/domain`.
2. Extend the serializer, the filter, the presenter and `createManagedDetailsForm`.
3. Add the locale keys (the locale tests tell you which ones are missing).
