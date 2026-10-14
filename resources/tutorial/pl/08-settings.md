# Ustawienia i wiersz poleceń

## Ustawienia

**Ustawienia → Ustawienia…** otwiera okno z wyborem **Języka** (polski, angielski lub niemiecki), **Motywu**,
**Czcionki** i przełącznikiem **Sprawdzaj aktualizacje**. Zmiany działają i zapisują się od razu.

Program raz dziennie sprawdza, czy na GitHubie jest nowsze wydanie, i proponuje otwarcie strony pobierania. Nic nie jest
pobierane ani instalowane samo. Jeśli nie chcesz sprawdzania, wyłącz je tutaj.

**Pomoc → Podgląd dziennika** pokazuje dziennik programu, który możesz skopiować albo zapisać przy zgłaszaniu problemu.

## Wiersz poleceń

- `--version` wypisuje wersję i kończy.
- `--log-level=POZIOM` ustawia, ile jest zapisywane w dzienniku: `debug`, `info`, `warning` lub `error`.
- `--data-dir=KATALOG` trzyma dane, ustawienia i pamięć podręczną w `KATALOG` zamiast w folderach systemowych.
- `--portable` robi to samo w folderze `portable` obok programu, co przydaje się na pendrive. Nie można łączyć z
  `--data-dir`.
- `--reset-settings` przywraca domyślne ustawienia. Stary plik zostaje jako `settings.json.bak`.
- `--safe-mode` startuje bez przywracania rozmiaru i położenia okna oraz bez sprzątania i synchronizacji, do
  diagnozowania problemów.
- `--self-test` sprawdza ikony, zasoby, tłumaczenia i folder danych, zapisuje wyniki w dzienniku i kończy z kodem błędu,
  gdy coś jest nie tak.

Z jednego folderu danych może korzystać naraz tylko jedna kopia programu. Drugie uruchomienie wysuwa na wierzch okno
pierwszego.
