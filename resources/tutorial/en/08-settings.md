# Settings and the command line

## Settings

**Settings → Settings…** opens the window with the **Language** (Polish, English or German), the **Theme**, the
**Font** and the switch **Check for updates**. Changes apply and save at once.

The program checks once a day whether a newer release exists on GitHub and offers to open the download page. Nothing is
downloaded or installed by itself. Switch the check off here if you do not want it.

**Help → View log** shows the log of the program, which you can copy or save when you report a problem.

## Command line

- `--version` prints the version and quits.
- `--log-level=LEVEL` sets how much is logged: `debug`, `info`, `warning` or `error`.
- `--data-dir=DIR` keeps the data, settings and cache in `DIR` instead of the system folders.
- `--portable` does the same in the `portable` folder next to the program, handy for a USB stick. It cannot be combined
  with `--data-dir`.
- `--reset-settings` returns the settings to the defaults. The old file is kept as `settings.json.bak`.
- `--safe-mode` starts without restoring the window size and position and without cleaning up or synchronising, for
  troubleshooting.
- `--self-test` checks icons, resources, translations and the data folder, writes the results to the log and quits with
  an error code when something is wrong.

Only one copy of the program can use a data folder at a time. A second start brings the first window to the front.
