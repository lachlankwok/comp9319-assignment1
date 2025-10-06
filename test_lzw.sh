#!/bin/bash

TOTAL_FAILURES=0
TOTAL_TESTS=${1:-1} 

echo "--- COMP9319 LZW Test Script by Juno ($TOTAL_TESTS Runs) ---"

echo "1. Cleaning up previous executables..."
rm -f lencode ldecode

echo -e "\n2. Compiling source files..."
if [ -f lencode.cpp ]; then
    g++ lencode.cpp -o lencode
elif [ -f lencode.c ]; then
    gcc lencode.c -o lencode
else
    echo "--- ERROR: No lencode.{cpp,c} found ---"; exit 1
fi
[ $? -ne 0 ] && echo "--- COMPILATION FAILED: lencode ---" && exit 1
echo "lencode compiled successfully."

if [ -f ldecode.cpp ]; then
    g++ ldecode.cpp -o ldecode
elif [ -f ldecode.c ]; then
    gcc ldecode.c -o ldecode
else
    echo "--- ERROR: No ldecode.{cpp,c} found ---"; exit 1
fi
[ $? -ne 0 ] && echo "--- COMPILATION FAILED: ldecode ---" && exit 1
echo "ldecode compiled successfully."

for i in $(seq 1 $TOTAL_TESTS); do
    INPUT_FILE="input_$i.txt"
    ENCODED_FILE="encoded_$i.lzw"
    DECODED_FILE="decoded_$i.txt"

    FILE_SIZE=$(( (RANDOM % 9000) + 1000 ))
    RESET_PERIOD=$((0 + (RANDOM % 199900)))

    echo -e "\n--- Running Test $i of $TOTAL_TESTS (Input Size: ${FILE_SIZE}KB, N=$RESET_PERIOD) ---"
    ALPHABET="TOBENR"

    echo "3. Generating random input file: $INPUT_FILE"
    if (( RANDOM % 2 )); then
        echo "   → Using restricted alphabet: $ALPHABET"
        head -c $((${FILE_SIZE}*2048)) /dev/urandom \
        | tr -cd "$ALPHABET" \
        | head -c $((${FILE_SIZE}*1024)) \
        > "$INPUT_FILE"
    else
        echo "   → Using full 7-bit ASCII (excluding NUL)"
        head -c $((${FILE_SIZE}*1024)) /dev/urandom \
        | tr -cd '\1-\177' \
        > "$INPUT_FILE"
    fi

    echo "4. Encoding: ./lencode $INPUT_FILE $ENCODED_FILE $RESET_PERIOD"
    ./lencode "$INPUT_FILE" "$ENCODED_FILE" "$RESET_PERIOD"
    [ $? -ne 0 ] && echo -e "\033[31m--- TEST $i FAILED (ENCODE) ---\033[0m" && TOTAL_FAILURES=$((TOTAL_FAILURES+1)) && continue

    echo "5. Decoding: ./ldecode $ENCODED_FILE $DECODED_FILE"
    ./ldecode "$ENCODED_FILE" "$DECODED_FILE"
    [ $? -ne 0 ] && echo -e "\033[31m--- TEST $i FAILED (DECODE) ---\033[0m" && TOTAL_FAILURES=$((TOTAL_FAILURES+1)) && continue

    echo "6. Verifying integrity (diff between $INPUT_FILE and $DECODED_FILE)..."
    diff -q "$INPUT_FILE" "$DECODED_FILE"
    if [ $? -eq 0 ]; then
        echo -e "\033[32m--- TEST $i PASSED (Original and decoded files are IDENTICAL) ---\033[0m"
        rm -f "$INPUT_FILE" "$ENCODED_FILE" "$DECODED_FILE"
    else
        echo -e "\033[31m--- TEST $i FAILED (Files differ) ---\033[0m"
        TOTAL_FAILURES=$((TOTAL_FAILURES+1))
    fi
done

echo -e "\n--- FINAL RESULTS ---"
if [ $TOTAL_FAILURES -eq 0 ]; then
    echo -e "\033[32mALL $TOTAL_TESTS TESTS PASSED SUCCESSFULLY.\033[0m"; exit 0
else
    echo -e "\033[31m$TOTAL_FAILURES / $TOTAL_TESTS TESTS FAILED.\033[0m"; exit 1
fi
