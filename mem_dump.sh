#!/bin/bash
GUEST_USER="deepiha"
GUEST_IP="127.0.0.1"
GUEST_SSH_PORT="8080"  
MONITOR_HOST="127.0.0.1"
MONITOR_PORT="4444"

SSH="ssh -p $GUEST_SSH_PORT $GUEST_USER@$GUEST_IP"

BYTES_PER_STEP=1024  
MODE="xp"  # you can later prompt user for 'x' or 'xp'
BEFORE_FILE="memory_before.txt"
AFTER_FILE="memory_after.txt"
DIFF_FILE="memory_diff.txt"

# Step 1: Find PID
echo "[*] Finding sse_search_server PID..."
PID=$($SSH "pgrep -f sse_search_server" | head -n 1)
if [[ -z "$PID" ]]; then
    echo "[!] Error: sse_search_server not running!"
    exit 1
fi
echo "    Found PID = $PID"

# Step 2: Read maps
echo "[*] Reading /proc/$PID/maps..."
MAPS=$($SSH "cat /proc/$PID/maps")

# Step 3: Parse addresses
START_ADDR_HEX=$(echo "$MAPS" | grep '/sse_search_server' | grep 'r-xp' | head -n1 | awk '{print $1}' | cut -d'-' -f1)
END_ADDR_HEX=$(echo "$MAPS" | grep '/sse_search_server' | grep 'r-xp' | head -n1 | awk '{print $1}' | cut -d'-' -f2)

echo "    Start Address = $START_ADDR_HEX"
echo "    End Address   = $END_ADDR_HEX"

# Step 4: Connect to monitor ONCE
exec 3<>/dev/tcp/$MONITOR_HOST/$MONITOR_PORT
sleep 1

# Step 5: Helper function
dump_memory() {
    local output=$1
    > "$output"

    START_DEC=$((16#$START_ADDR_HEX))
    END_DEC=$((16#$END_ADDR_HEX))
    STEP=$BYTES_PER_STEP

    for ((addr=$START_DEC; addr<$END_DEC; addr+=$STEP)); do
        ADDR_HEX=$(printf "0x%x" $addr)

        # Send command
        echo "${MODE} /${BYTES_PER_STEP}b $ADDR_HEX" >&3
        sleep 0.05

        # Read response for this command
        {
            timeout 1 cat <&3
            echo    # blank line to separate
        } >> "$output"
    done
}


# Step 6: Dump before
echo "[*] Dumping memory BEFORE..."
dump_memory "$BEFORE_FILE"
echo
echo "[*] Run your query inside VM. Press ENTER to continue..."
read

# Step 7: Dump after
echo "[*] Dumping memory AFTER..."
dump_memory "$AFTER_FILE"

# Step 8: Close monitor connection
exec 3<&-
exec 3>&-

# Step 9: Diff
echo "[*] Diffing memory..."
diff --side-by-side --suppress-common-lines "$BEFORE_FILE" "$AFTER_FILE" > "$DIFF_FILE"

echo
echo "======================================================"
echo "Done! Memory differences stored in: $DIFF_FILE"
echo "======================================================"
