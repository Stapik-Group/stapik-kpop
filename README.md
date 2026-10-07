# Stapik K-Pop

Retro-styled desktop manager of a K-pop collection, written in C++20 with GTK4 (gtkmm) on top of
[stapik-common](https://github.com/Stapik-Group/stapik-common).

## What can be collected

| Kind | Details |
| --- | --- |
| Albums | type, format (CD, vinyl, cassette, digital, platform album, kit), edition, release date, label, region, catalog number, inclusions |
| Photocards | member, origin (album, pre-order, lucky draw, fansign, event), source, available for trade |
| Merchandise | lightsticks, apparel, photobooks, season's greetings, posters, keyrings, stationery |
| Clips | type, platform, link, release date, album, watched |
| Lyrics | original text, romanization, translation (searchable) |
| Events | concert, fansign, fan meeting, listening party, pop-up store, venue, city, seat |

Every entry also has an artist, a status (owned, ordered, wishlist, sold), a condition, a number of copies,
a price in any currency, an acquisition date and place, and notes. Dates may be partial (`2021`, `2021-08`, `2021-08-23`).

Features coming from stapik-common: undo/redo of every edit, three themes, Polish/English/German, atomic saving,
and optional synchronisation with your own Stapik Cloud.

## Building

Requirements: a C++20 compiler, CMake 4.2+, gtkmm-4.0, libcurl, Ninja (optional).

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
distributed as a zip. There is no installer.

* **From CI:** the *Package* workflow builds the zip on every push and pull request, runs the program once on the
  unpacked zip with only the Windows directories in `PATH` (`stapikkpop.exe --self-test`), and uploads it as an
  artifact. Run it by hand (Actions > Package > Run workflow) to get a build of any branch for testers; a `v*` tag
  publishes the zip, together with the Ubuntu package (see below), as one GitHub release.
* **Locally**, in an MSYS2 UCRT64 shell (packages: `gcc cmake ninja pkgconf gtkmm-4.0 curl adwaita-icon-theme
  hicolor-icon-theme librsvg`, prefixed `mingw-w64-ucrt-x86_64-`):

  ```sh
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build build --target stapikkpop_windows_bundle
  ```

  The folder is `build/stapikkpop-windows/`. Add `-DSTAPIK_WINDOWS_CONSOLE=ON` to keep a console window.

Data lives in `%APPDATA%\stapikkpop\data\collection.json`, settings in `%APPDATA%\stapikkpop\config` and the log
(stderr of the GUI program) in `%LOCALAPPDATA%\stapikkpop\cache\stapik.log`. The program draws with the Cairo
renderer by default; set `GSK_RENDERER=gl` to use the GPU. `packaging/windows/README.txt` is the note for testers that
goes into the zip.

`--self-test` is also available on Linux; it checks icons, GSettings schemas, resources, translations and a writable
data directory, logs every check and exits with 1 when one of them fails.

To change the stapik-common version: `-DSTAPIK_COMMON_GIT_TAG=<tag, branch or commit>`.

## Linux package

The *Package* workflow also builds `stapikkpop_<version>_amd64.deb` (CPack, installed under `/opt/stapikkpop` with a
launcher in `/usr/bin`, a `.desktop` entry and the icon) and, before uploading it, installs it in a clean Ubuntu 24.04
container with only its declared dependencies and starts it (`--self-test`). It needs Ubuntu 24.04 or newer (or Debian
13): the dependencies are those of the build machine (`libgtkmm-4.0-0`, `libstdc++6 >= 13`, `t64` library names).
Install with `sudo apt install ./stapikkpop_<version>_amd64.deb`. Locally: `cmake --build build` and then
`cd build && cpack -G DEB -D CPACK_DEBIAN_FILE_NAME=DEB-DEFAULT`.

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
