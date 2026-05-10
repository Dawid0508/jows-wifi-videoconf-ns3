#!/bin/bash
export LC_ALL=C

# =======================================================
# KONFIGURACJA PASMA (Zmieniaj tylko to: 2.4 lub 5.0)
# =======================================================
BAND_VAL="5.0"                        # 2.4 lub 5.0
BAND_NAME="${BAND_VAL}GHz"            # Do nazw folderów

# --- Listy do przejechania ---
STANDARDS=("n" "ac" "ax")             # Jakie standardy testować
SCENARIOS=("scenario_1_no_background" "scenario_2_with_background")

# =======================================================
# PARAMETRY SYMULACJI
# =======================================================
STEP=5               
MAX_STA=45           
SIM_TIME=120              # Czas symulacji w sekundach (2 minuty)
NS3_EXE="../../ns3"

# Główna pętla po Scenariuszach
for SCENARIO in "${SCENARIOS[@]}"
do
    # Główna pętla po Standardach
    for STANDARD in "${STANDARDS[@]}"
    do
        # Blokada dla ac na 2.4GHz (bo fizycznie nie istnieje)
        if [[ "$STANDARD" == "ac" && "$BAND_VAL" == "2.4" ]]; then
            echo "--- POMIJAM: ac nie działa na 2.4GHz ---"
            continue
        fi

        # Ustawienie liczby stacji tła w zależności od scenariusza
        if [[ "$SCENARIO" == "scenario_1_no_background" ]]; then
            N_BACKGROUND=0
        else
            N_BACKGROUND=5  # Przykładowo 5 stacji tła dla drugiego wariantu
        fi

        # Folder na wyniki
        RESULTS_DIR="results/${SCENARIO}/80211${STANDARD}_${BAND_NAME}"
        mkdir -p "$RESULTS_DIR"
        OUT_FILE="${RESULTS_DIR}/pojemnosc.csv"
        TEMP_LOG="/tmp/ns3_temp_run.log"

        echo "nVideo,Throughput_Avg,Delay_Avg,Jitter_Avg,Loss_Avg" > "$OUT_FILE"

        echo "********************************************************"
        echo "START: $SCENARIO | Standard: $STANDARD | Pasmo: $BAND_NAME"
        echo "********************************************************"

        # Pętla po liczbie stacji Video
        for (( n=$STEP; n<=$MAX_STA; n+=$STEP ))
        do
            echo ">> nVideo=$n | nBG=$N_BACKGROUND | $STANDARD @ $BAND_NAME"
            
            # URUCHOMIENIE (Z parametrem --band!)
            $NS3_EXE run wifi_videoconf_capacity -- --standard=$STANDARD --band=$BAND_VAL --nVideo=$n --nBackground=$N_BACKGROUND --simTime=$SIM_TIME > "$TEMP_LOG" 2>&1
            
            # Sprawdzenie czy są wyniki
            if ! grep -q "VIDEO" "$TEMP_LOG"; then
                echo "   [BŁĄD] Brak statystyk w logu! Przejdź do następnego standardu."
                break
            fi

            # Ekstrakcja
            THR=$(grep "VIDEO" -A 4 "$TEMP_LOG" | grep "Throughput" | awk '{sum+=$2; count++} END {if (count > 0) print sum/count; else print 0}')
            DEL=$(grep "VIDEO" -A 4 "$TEMP_LOG" | grep "Mean Delay" | awk '{sum+=$3; count++} END {if (count > 0) print sum/count; else print 0}')
            JIT=$(grep "VIDEO" -A 4 "$TEMP_LOG" | grep "Mean Jitter" | awk '{sum+=$3; count++} END {if (count > 0) print sum/count; else print 0}')
            LOS=$(grep "VIDEO" -A 4 "$TEMP_LOG" | grep "Packet Loss" | awk '{sum+=$3; count++} END {if (count > 0) print sum/count; else print 0}')
            
            echo "$n,$THR,$DEL,$JIT,$LOS" >> "$OUT_FILE"
            echo "   Status: Delay=${DEL}ms, Loss=${LOS}%"
            cp "$TEMP_LOG" "${RESULTS_DIR}/raw_log_n${n}.txt"

            # Przerwanie jeśli sieć padnie
            if (( $(echo "$DEL > 150" | bc -l) )) || (( $(echo "$LOS > 2" | bc -l) )); then
                echo "   !!! NASYCENIE: Przekroczono progi QoS. Kończę ten wariant."
                break
            fi
        done
    done
done

echo "========================================================"
echo "WSZYSTKIE SYMULACJE DLA PASMA $BAND_NAME ZAKOŃCZONE!"