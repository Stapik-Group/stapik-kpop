# Sicherungen und Wiederherstellen

## Automatische Kopien

Vor jedem Speichern hebt das Programm die vorherige Version der Sammlungsdatei auf. Es behält die **fünf** neuesten
Kopien neben der Sammlungsdatei, benannt `collection.json.bak`, `collection.json.bak.2` und so weiter bis `.bak.5`.
Eine beschädigte Datei wird nie kopiert, eine kaputte Datei verdrängt also keine gute Kopie.

## Eine ältere Version wiederherstellen

1. Wähle **Datei → Aus Sicherung wiederherstellen…**.
2. Wähle eine Kopie nach Datum und Uhrzeit.
3. Bestätige. Die Sammlung wird durch die gewählte Version ersetzt.

Die Sammlung von vor dem Wiederherstellen wird zur neuesten Kopie, du kannst das Wiederherstellen also auf dieselbe
Weise zurücknehmen. Das Wiederherstellen leert den Rückgängig-Verlauf. Bei verbundener Cloud wird die wiederhergestellte
Version auch dorthin gesendet.

> Bilder gehören nicht zu diesen Kopien. Ein wiederhergestellter Eintrag kann auf ein Cover zeigen, das auf diesem
> Computer nicht mehr vorhanden ist. Bei verbundener Cloud wird es erneut heruntergeladen.

## Wo die Daten liegen

Unter Linux liegt die Sammlung in `~/.local/share/stapikkpop/collection.json`, die Bilder in den Ordnern `images`
und `photos` daneben. Unter Windows liegt sie in `%APPDATA%\stapikkpop\data`. Für eine eigene Sicherung kopiere diesen
Ordner.

Ist die Sammlungsdatei beschädigt, legt das Programm sie beiseite (der Name bekommt `.corrupt-` und ein Datum), startet
mit einer leeren Sammlung und sagt dir Bescheid. Eine von einer neueren Programmversion gespeicherte Datei bleibt
unverändert und wird nicht geöffnet.
