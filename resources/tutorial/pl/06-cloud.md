# Synchronizacja z chmurą

Kolekcję można utrzymywać zgodną na kilku komputerach przez własną **chmurę Stapik Cloud** (wersja 1.2.0 lub nowsza).
To opcja: bez chmury program działa tak samo, a wszystkie dane zostają na Twoim komputerze.

## Łączenie

1. Wybierz **Plik → Połącz z chmurą**.
2. Wpisz **Adres API** swojej chmury i **Klucz API** utworzony w jej panelu.
3. Naciśnij **Sprawdź**, aby zobaczyć, czy serwer odpowiada i przyjmuje klucz.
4. Naciśnij **Połącz**.

Użyj adresu zaczynającego się od `https://`. Przy zwykłym `http://` klucz i dane są wysyłane bez ochrony.

## Co jest synchronizowane

Kolekcja (wszystkie wpisy i artyści) oraz okładki i zdjęcia. Stan widać na dole okna. Zmiany są wysyłane niedługo po
ich wprowadzeniu, a **Plik → Synchronizuj** robi to od razu. Gdy dwa komputery zmieniły kolekcję, wygrywa nowsza
wersja.

## Szyfrowanie

Możesz wpisać **hasło szyfrowania end-to-end**. Wtedy kolekcja jest szyfrowana na Twoim komputerze, a serwer widzi
tylko dane, których nie umie odczytać. Pamiętaj:

- Używaj **tego samego hasła na każdym komputerze**.
- Bez hasła danych w chmurze **nie da się odzyskać**. Nikt nie może go zresetować.
- Szyfrowana jest tylko kolekcja. **Okładki i zdjęcia są wysyłane bez szyfrowania**, bo chmura nie szyfruje plików.

## Klucze i limity

Gdy Twój klucz API zaraz wygaśnie, program poda datę. Utwórz nowy klucz w panelu chmury i wpisz go ponownie w oknie
**Połącz z chmurą**. Gdy chmura poprosi o zwolnienie tempa, program sam poczeka i spróbuje ponownie.
