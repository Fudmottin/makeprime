#!/bin/bash

# Check if exactly one argument is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_runs>"
    exit 1
fi

# Store the number of runs
RUNS=$1

# Validate that the argument is a positive integer
if ! [[ "$RUNS" =~ ^[0-9]+$ ]] || [ "$RUNS" -le 0 ]; then
    echo "Error: Number of runs must be a positive integer"
    exit 1
fi

PROGRAM="./build/makeprime 1000"

# Initialize variable to store total time
TOTAL_TIME=0

# Run the program the specified number of times
for ((i=1; i<=RUNS; i++)); do
    # Capture both program output and time output, ensuring program output is displayed
    # Use a temporary file to store time output
    TEMP_FILE=$(mktemp)
    { /usr/bin/time -p sh -c "$PROGRAM" 2> "$TEMP_FILE"; }

    # Display program output (if any)
    eval "$PROGRAM"

    # Extract the real time from time output (format: real 0m0.123s)
    TIME_SECONDS=$(grep '^real' "$TEMP_FILE" | awk '{print $2}' | sed 's/.*m\([0-9.]*\)s/\1/')

    # Add the time to total (using bc for floating-point arithmetic)
    if [[ -n "$TIME_SECONDS" ]]; then
        TOTAL_TIME=$(echo "$TOTAL_TIME + $TIME_SECONDS" | bc)
    else
        echo "Warning: Could not parse time for run $i"
        TIME_SECONDS=0
    fi

    # Clean up temporary file
    rm "$TEMP_FILE"
done

# Output the total runtime
echo "Total runtime for $RUNS runs: $TOTAL_TIME seconds"

