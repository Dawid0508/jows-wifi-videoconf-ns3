import os
import pandas as pd
import matplotlib.pyplot as plt

# Ścieżka do głównego folderu z wynikami
RESULTS_DIR = "results"
# Główny folder na wykresy (na poziomie scenario_1 i scenario_2)
PLOTS_DIR = os.path.join(RESULTS_DIR, "wykresy")

def plot_metric(scenario_name, data_dict, metric_col, metric_name, ylabel, qos_limit=None):
    """Funkcja pomocnicza do rysowania pojedynczego wykresu."""
    plt.figure(figsize=(10, 6))
    
    # Rysujemy linie dla każdego standardu/pasma
    for label, df in data_dict.items():
        if metric_col in df.columns:
            plt.plot(df['nVideo'], df[metric_col], marker='o', linewidth=2, label=label)
            
    # Dodanie czerwonej linii limitu QoS (jeśli istnieje)
    if qos_limit is not None:
        plt.axhline(y=qos_limit, color='r', linestyle='--', label=f'Limit QoS ({qos_limit})')

    plt.title(f"{metric_name} vs Liczba Stacji ({scenario_name})", fontsize=14)
    plt.xlabel("Liczba stacji wideo (nVideo)", fontsize=12)
    plt.ylabel(ylabel, fontsize=12)
    plt.grid(True, linestyle=':', alpha=0.7)
    plt.legend(title="Standard / Pasmo")
    
    # Zapis do pliku w folderze results/wykresy
    # Dodajemy nazwę scenariusza do nazwy pliku, by się nie nadpisywały!
    filename = f"{scenario_name}_{metric_name.replace(' ', '_')}.png"
    output_filename = os.path.join(PLOTS_DIR, filename)
    
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f" Zapisano wykres: {output_filename}")

# Tworzymy folder na wykresy (jeśli nie istnieje)
os.makedirs(PLOTS_DIR, exist_ok=True)

# Główna pętla przeskakująca po scenariuszach
for scenario in os.listdir(RESULTS_DIR):
    scenario_path = os.path.join(RESULTS_DIR, scenario)
    
    # Pomijamy pliki oraz pomijamy nasz nowy folder "wykresy"
    if not os.path.isdir(scenario_path) or scenario == "wykresy":
        continue
        
    print(f"\n--- Przetwarzanie scenariusza: {scenario} ---")
    
    # Słownik na dane z tego scenariusza
    scenario_data = {}
    
    # Przeszukujemy podfoldery standardów (n, ac, ax)
    for std_folder in os.listdir(scenario_path):
        std_path = os.path.join(scenario_path, std_folder)
        csv_path = os.path.join(std_path, "pojemnosc.csv")
            
        if os.path.isdir(std_path) and os.path.isfile(csv_path):
            try:
                # Wczytanie CSV
                df = pd.read_csv(csv_path)
                # Zapisujemy do słownika, kluczem jest nazwa folderu (np. 80211ac_5GHz)
                scenario_data[std_folder] = df
                print(f"  Wczytano dane z: {std_folder}")
            except Exception as e:
                print(f"  Błąd wczytywania {csv_path}: {e}")
                
    # Rysowanie wykresów dla zebranych danych
    if scenario_data:
        plot_metric(scenario, scenario_data, 'Delay_Avg', 'Średnie Opóźnienie', 'Opóźnienie [ms]', qos_limit=150)
        plot_metric(scenario, scenario_data, 'Loss_Avg', 'Straty Pakietów', 'Straty [%]', qos_limit=2.0)
        plot_metric(scenario, scenario_data, 'Throughput_Avg', 'Przepływność', 'Przepływność [Mb/s]')