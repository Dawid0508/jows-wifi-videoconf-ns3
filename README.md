```markdown
# 📶 Porównanie pojemności sieci Wi-Fi dla usług wideokonferencji

## 📌 O projekcie
Projekt realizowany w ramach przedmiotu **Jakość obsługi w sieciach (JOWS)**. 
Celem badań jest określenie maksymalnej liczby jednoczesnych użytkowników usługi wideokonferencji, jaką jest w stanie obsłużyć sieć Wi-Fi w różnych standardach, przy zachowaniu rygorystycznych wymagań QoS (Quality of Service). Symulacje przeprowadzane są z wykorzystaniem symulatora **NS-3.47**.

## 🎯 Główne cele badawcze
* Zbadanie pojemności sieci dla standardów: **IEEE 802.11n**, **802.11ac** oraz **802.11ax**.
* Porównanie wydajności w pasmach **2.4 GHz** oraz **5 GHz**.
* Analiza wpływu ruchu tła na jakość transmisji wideo i głosu o wysokim priorytecie.

## 📊 Wymagania QoS dla wideokonferencji
Aby uznać jakość połączenia za akceptowalną, przyjęto następujące graniczne wartości parametrów:
* **Opóźnienie (Delay):** < 150 ms
* **Zmienność opóźnienia (Jitter):** < 30 ms
* **Straty pakietów (Packet Loss):** < 2%

> **Ważne:** W przypadku usług typu "live", do których należą wideokonferencje, zbyt duży jitter może bezpośrednio przełożyć się na straty pakietów, ponieważ opóźnione dane stają się nieaktualne z punktu widzenia aplikacji. Rozwiązaniem tego problemu zaimplementowanym w kodzie jest ustawienie limitu czasu życia pakietu w kolejce transmisyjnej (atrybut `MaxDelay`), co pozwala ograniczyć opóźnienie i jitter kosztem dopuszczalnych strat.

## 🧪 Scenariusze symulacyjne
Badania realizujemy w dwóch głównych wariantach:
1. **Brak ruchu tła (`scenario_1_no_background`):** Sieć obsługuje wyłącznie stacje uczestniczące w wideokonferencji. Wpływ na pojemność ma jedynie rosnąca liczba stacji oraz narzut protokołów.
2. **Dociążenie sieci ruchem tła (`scenario_2_with_background`):** W sieci znajdują się dodatkowe stacje (domyślnie 5) generujące intensywny ruch (np. pobieranie plików). Ten ruch przypisany jest do domyślnej klasy dostępu **Best Effort (AC_BE)**, podczas gdy wideo wykorzystuje priorytetowe kolejki EDCA (**AC_VI**).

## 📁 Struktura i organizacja plików
Projekt opiera się na zasadzie "jeden plik źródłowy, pełna automatyzacja wyników". Drzewo katalogów wygląda następująco:

```text
jows-wifi-videoconf-ns3/
├── wifi_videoconf_capacity.cc     # GŁÓWNY SKRYPT NS-3 (Kod w C++)
├── auto_run.sh                    # Skrypt bash automatyzujący pomiary w pętli
├── plot.py               # Skrypt Python do automatycznego generowania wykresów
└── results/                       # Główny folder na wyniki
    ├── scenario_1_no_background/
    │   ├── 80211n_5GHz/           # Tutaj zapisuje się pojemnosc.csv i logi surowe
    │   └── ...
    ├── scenario_2_with_background/
    └── wykresy/                   # Tutaj zapisywane są gotowe wykresy .png

```

## 🛠 Metodyka i czas symulacji

* **Środowisko:** Symulator NS-3.47.
* **Rozgrzewanie sieci (Warm-up):** Zbieranie statystyk przez moduł FlowMonitor rozpoczyna się dopiero od 20. sekundy symulacji (zmienna `calcStart = 20.0`).
* **Czasy testowe:** Do szybkiej weryfikacji działania skryptów i wykrywania błędów stosujemy krótki czas, np. `SIM_TIME=25` (co daje 5 sekund właściwego pomiaru).
* **Czasy docelowe:** W celu uzyskania stabilnych i wiarygodnych statystyk końcowych rekomendowany czas wynosi `SIM_TIME=120`. Oznacza to 100 sekund czystej symulacji ruchu sieciowego po odliczeniu czasu na wygrzewanie.

## 🚀 Instrukcja obsługi: Jak uruchomić pomiary i analizę?

Wszystkie pomiary dla danego pasma uruchamia się jednym poleceniem, a następnie drugim generuje się wykresy.

1. **Przygotowanie:** Upewnij się, że kod C++ został wcześniej skompilowany poleceniem `./ns3 build` w głównym folderze symulatora.
2. **Konfiguracja testu:** Otwórz plik `auto_run.sh` i w sekcji **PANEL STEROWANIA** zmień parametry:
* `BAND_VAL` – ustaw na `2.4` lub `5.0`.
* `SIM_TIME` – ustaw na `120` dla pełnych pomiarów do projektu.


3. **Uruchomienie pomiarów:**
Będąc w terminalu w folderze ze skryptem, nadaj mu uprawnienia i uruchom:
```bash
chmod +x auto_run.sh
./auto_run.sh

```


4. **Automatyka procesu:** * Skrypt samodzielnie iteruje po scenariuszach i standardach (omijając standard `ac` dla pasma 2.4 GHz).
* Automatycznie inkrementuje liczbę stacji o 5.
* Natychmiast przerywa pętlę dla danego wariantu przy przekroczeniu limitów QoS (nasycenie sieci).


5. **Generowanie wykresów:** Po zebraniu wyników, uruchom skrypt analityczny:
```bash
python plot.py

```


Skrypt połączy dane ze wszystkich plików CSV i wygeneruje wykresy `.png` do folderu `results/wykresy/`.

---

## ⚠️ Status prac (TODO)

* [x] **Aktualizacja kodu .cc:** Wdrożono nowy atrybut `ChannelSettings` dla pasma i obsługę parametru `--band`.
* [x] **Skrypty analityczne:** Dodano skrypt `rysuj_wykresy.py` z automatycznym zaznaczaniem limitów QoS.
* [x] **Zrobić badania dla 2.4 oraz 5 GHz:** Przeprowadzenie pełnych iteracji skryptu `auto_run.sh` z ustawionymi wartościami `BAND_VAL="2.4"` oraz `BAND_VAL="5.0"`
* [ ] **Analiza końcowa:** Skompletowanie wykresów z zebranych plików CSV i przygotowanie raportu podsumowującego.

```

```