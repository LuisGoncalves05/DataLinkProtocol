#!/bin/bash

# === Configuration ===
COMMANDS=(
  "baud 2400"
  "baud 4800"
  "baud 9600"
  "baud 19200"
  "baud 57600"
  "prop 0"
  "prop 10"
  "prop 10000"
  "prop 100000"
  "prop 1000000"
  "ber 0.000000"
  "ber 0.000001"
  "ber 0.000025"
  "ber 0.000050"
  "ber 0.000100"
)
RUNS=10
TIMEOUT_TX=120
TIMEOUT_RX=120

echo "Starting tests..."
mkdir -p logs

for CMD in "${COMMANDS[@]}"; do
    SAFE_CMD=$(echo "$CMD" | tr ' ' '_')
    OUTPUT_FILE="logs/times_${SAFE_CMD}.log"

    echo "=========================================="
    echo "Running command: '$CMD'"
    echo "Saving results to $OUTPUT_FILE"
    echo "Elapsed times (seconds):" > "$OUTPUT_FILE"

    make run_cable < <(cat) > /dev/null 2>&1 &
    CABLE_PID=$!

    sleep 5
    echo "$CMD" > /proc/$CABLE_PID/fd/0    

    for i in $(seq 1 $RUNS); do
        sleep 5
        echo "Run #$i..."
        timeout $TIMEOUT_RX make run_rx > /dev/null 2>&1 &
        RX_PID=$!
        sleep 5

        TX_OUTPUT=$(timeout $TIMEOUT_TX make run_tx 2>&1)
        TX_STATUS=$?

        TX_LOG="logs/tx_${SAFE_CMD}_${i}.log"
        echo "$TX_OUTPUT" > "$TX_LOG"

        wait $RX_PID 2>/dev/null

        if [ $TX_STATUS -eq 124 ]; then
            echo "Run #$i: TX timed out!"
            echo "TIMEOUT" >> "$OUTPUT_FILE"
            continue
        fi

        ELAPSED=$(echo "$TX_OUTPUT" | grep -oP 'Elapsed Time:\s*\K[0-9.]+')

        if [ -n "$ELAPSED" ]; then
            echo "Run #$i: $ELAPSED s"
            echo "$ELAPSED" >> "$OUTPUT_FILE"
        else
            echo "Run #$i: Elapsed time not found!"
            echo "N/A" >> "$OUTPUT_FILE"
        fi
    done

    kill -9 $CABLE_PID 2>/dev/null
    wait $CABLE_PID 2>/dev/null

    echo "Done with '$CMD'. Results saved to $OUTPUT_FILE."
done

echo "=========================================="
echo "All tests finished!"
