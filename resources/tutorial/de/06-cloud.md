# Cloud-Synchronisierung

Die Sammlung lässt sich über deine eigene **Stapik Cloud** (Version 1.2.0 oder neuer) auf mehreren Computern auf
demselben Stand halten. Das ist optional: Ohne Cloud arbeitet das Programm genauso, und alle Daten bleiben auf deinem
Computer.

## Verbinden

1. Wähle **Datei → Mit Cloud verbinden**.
2. Gib die **API-URL** deiner Cloud und einen im Panel erstellten **API-Schlüssel** ein.
3. Drücke **Prüfen**, um zu sehen, ob der Server antwortet und den Schlüssel akzeptiert.
4. Drücke **Verbinden**.

Nutze eine Adresse, die mit `https://` beginnt. Bei einfachem `http://` werden Schlüssel und Daten ungeschützt
gesendet.

## Was synchronisiert wird

Die Sammlung (alle Einträge und Künstler) sowie Cover und Fotos. Den Stand siehst du unten im Fenster. Änderungen
werden kurz nach dem Vornehmen gesendet, **Datei → Synchronisieren** erledigt das sofort. Haben zwei Computer die
Sammlung geändert, gewinnt die neuere Version.

## Verschlüsselung

Du kannst eine **Passphrase für Ende-zu-Ende-Verschlüsselung** eingeben. Dann wird die Sammlung auf deinem Computer
verschlüsselt, und der Server sieht nur Daten, die er nicht lesen kann. Bedenke:

- Nutze **dieselbe Passphrase auf jedem Computer**.
- Ohne die Passphrase lassen sich die Daten in der Cloud **nicht wiederherstellen**. Niemand kann sie zurücksetzen.
- Nur die Sammlung wird verschlüsselt. **Cover und Fotos werden unverändert gesendet**, weil die Cloud keine Dateien
  verschlüsselt.

## Schlüssel und Grenzen

Läuft dein API-Schlüssel bald ab, nennt dir das Programm das Datum. Erstelle im Cloud-Panel einen neuen Schlüssel und
gib ihn erneut unter **Mit Cloud verbinden** ein. Bittet die Cloud darum, langsamer zu machen, wartet das Programm von
selbst und versucht es erneut.
