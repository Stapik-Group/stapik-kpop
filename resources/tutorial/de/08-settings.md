# Einstellungen und Kommandozeile

## Einstellungen

**Einstellungen → Einstellungen…** öffnet das Fenster mit **Sprache** (Polnisch, Englisch oder Deutsch), **Thema**,
**Schriftart** und dem Schalter **Nach Updates suchen**. Änderungen wirken und werden sofort gespeichert.

Das Programm prüft einmal am Tag, ob es auf GitHub eine neuere Version gibt, und bietet an, die Download-Seite zu
öffnen. Nichts wird von selbst heruntergeladen oder installiert. Schalte die Prüfung hier aus, wenn du sie nicht willst.

**Hilfe → Protokoll anzeigen** zeigt das Protokoll des Programms, das du bei einer Fehlermeldung kopieren oder
speichern kannst.

## Kommandozeile

- `--version` gibt die Version aus und beendet das Programm.
- `--log-level=STUFE` legt fest, wie viel protokolliert wird: `debug`, `info`, `warning` oder `error`.
- `--data-dir=ORDNER` hält Daten, Einstellungen und Cache in `ORDNER` statt in den Systemordnern.
- `--portable` tut dasselbe im Ordner `portable` neben dem Programm, praktisch für einen USB-Stick. Nicht mit
  `--data-dir` kombinierbar.
- `--reset-settings` setzt die Einstellungen zurück. Die alte Datei bleibt als `settings.json.bak` erhalten.
- `--safe-mode` startet ohne Wiederherstellung von Fenstergröße und -position und ohne Aufräumen oder Synchronisieren,
  zur Fehlersuche.
- `--self-test` prüft Symbole, Ressourcen, Übersetzungen und den Datenordner, schreibt die Ergebnisse ins Protokoll und
  endet mit einem Fehlercode, wenn etwas nicht stimmt.

Einen Datenordner kann immer nur eine Kopie des Programms nutzen. Ein zweiter Start holt das erste Fenster nach vorn.
