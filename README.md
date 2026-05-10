# 📶 Porównanie pojemności sieci Wi-Fi dla usług wideokonferencji

## 📌 O projekcie
Projekt realizowany w ramach przedmiotu **Jakość obsługi w sieciach (JOWS)**. 
Celem badań jest określenie maksymalnej liczby jednoczesnych użytkowników usługi wideokonferencji, jaką jest w stanie obsłużyć sieć Wi-Fi w różnych standardach, przy zachowaniu rygorystycznych wymagań QoS (Quality of Service). Symulacje przeprowadzane są z wykorzystaniem symulatora **NS-3**.

## 🎯 Główne cele badawcze
* Zbadanie pojemności sieci dla standardów: **IEEE 802.11n**, **802.11ac** oraz **802.11ax**.
* Porównanie wydajności w pasmach **2.4 GHz** oraz **5 GHz**.
* Analiza wpływu ruchu tła na jakość transmisji wideo i głosu o wysokim priorytecie.

## 📊 Wymagania QoS dla wideokonferencji
Aby uznać jakość połączenia za akceptowalną, przyjęto następujące graniczne wartości parametrów:
* **Opóźnienie (Delay):** < 150 ms
* **Zmienność opóźnienia (Jitter):** < 30 ms
* **Straty pakietów (Packet Loss):** < 1-2%

> [cite_start]**Ważne:** W przypadku usług typu "live", do których należą wideokonferencje, zbyt duży jitter może bezpośrednio przełożyć się na straty pakietów, ponieważ opóźnione dane stają się nieaktualne z punktu widzenia aplikacji[cite: 203]. [cite_start]Rozwiązaniem tego problemu może być ustawienie limitu czasu życia pakietu w kolejce transmisyjnej, co pozwala ograniczyć opóźnienie i jitter kosztem dopuszczalnych strat[cite: 204].

## 🧪 Scenariusze symulacyjne
Badania zostaną zrealizowane w dwóch głównych wariantach:
1. **Brak ruchu tła:** Sieć obsługuje wyłącznie stacje uczestniczące w wideokonferencji. Wpływ na pojemność ma jedynie rosnąca liczba stacji oraz narzut protokołów.
2. **Dociążenie sieci ruchem tła:** W sieci znajdują się dodatkowe stacje generujące intensywny ruch (np. pobieranie plików, intensywny ruch HTTP). Ten ruch będzie przypisany do domyślnej klasy dostępu **Best Effort (AC_BE)**, podczas gdy ruch wideokonferencyjny wykorzysta priorytetowe klasy EDCA (np. **AC_VI** / **AC_VO**).

## 🛠 Metodyka i czas symulacji
* **Środowisko:** Symulator NS-3.
* **Czasy testowe:** Podczas opracowywania skryptów i weryfikacji błędów symulacje będą uruchamiane na krótszy czas, np. [cite_start]10-20 s[cite: 212].
* [cite_start]**Czasy docelowe:** W celu uzyskania wiarygodnych wyników końcowych rekomendowany czas symulacji wyniesie 120 s (100 s właściwej symulacji + 20 s "wygrzewania" sieci)[cite: 211].

---

## 🚀 Uruchomienie skryptów

*Sekcja do uzupełnienia po stworzeniu pierwszych skryptów symulacyjnych.*
