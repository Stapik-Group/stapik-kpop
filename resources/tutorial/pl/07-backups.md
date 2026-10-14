# Kopie zapasowe i przywracanie

## Kopie automatyczne

Przed każdym zapisem program zachowuje poprzednią wersję pliku kolekcji. Trzyma **pięć** ostatnich kopii obok pliku
kolekcji, o nazwach `collection.json.bak`, `collection.json.bak.2` i tak dalej do `.bak.5`. Uszkodzony plik nigdy nie
jest kopiowany, więc zepsuty plik nie wypchnie dobrej kopii.

## Przywracanie starszej wersji

1. Wybierz **Plik → Przywróć z kopii zapasowej…**.
2. Wybierz kopię po dacie i godzinie.
3. Zatwierdź. Kolekcja zostanie zastąpiona wybraną wersją.

Kolekcja sprzed przywrócenia staje się najnowszą kopią, więc przywrócenie możesz cofnąć w ten sam sposób. Przywrócenie
czyści historię cofania. Przy połączonej chmurze przywrócona wersja jest tam także wysyłana.

> Obrazy nie wchodzą w skład tych kopii. Przywrócony wpis może wskazywać okładkę, której nie ma już na tym komputerze.
> Przy połączonej chmurze zostanie pobrana ponownie.

## Gdzie są dane

W Linuksie kolekcja jest w `~/.local/share/stapikkpop/collection.json`, a obrazy w folderach `images` i `photos` obok
niej. W Windows jest w `%APPDATA%\stapikkpop\data`. Aby zrobić własną kopię, skopiuj ten folder.

Jeśli plik kolekcji jest uszkodzony, program odkłada go na bok (nazwa dostaje `.corrupt-` i datę), startuje z pustą
kolekcją i informuje Cię o tym. Plik zapisany przez nowszą wersję programu zostaje nietknięty i nie jest otwierany.
