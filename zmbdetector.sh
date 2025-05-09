#!/bin/bash

# Define the files that will be used
LOG_FILE="processes.txt"
C_FILE="it2023003.c"
OUTPUT_FILE="it2023003.out"



# Function to handle graceful exit
cleanup() {
    #check if the pid is not empty and the process is running
    if [ -n "$PID" ] && ps -p "$PID" > /dev/null 2>&1; then #silence the output of the ps command
        echo "Terminating process $PID..."
        kill SIGTERM "$PID"
    fi
    echo " Script terminated at $(date)" > /dev/tty
    exit
}

# Trap signals for graceful termination
trap cleanup SIGINT SIGTERM


# Start the script

#Compile the C program
if [ ! -e "$OUTPUT_FILE" ]; # check if c file has already been compiled
then
    gcc "$C_FILE" -pthread -o "$OUTPUT_FILE"
    if [ $? -ne 0 ]; then
        exit 1
    fi
fi

clean_zombies() {
    zombies=$(ps -eo pid,ppid,stat,comm | awk '$3 ~ /Z/ {print $1, $2}')

    if [[ -z "$zombies" ]]; then
        echo "No zombie processes found."
        return
    fi

    while read -r pid ppid; do
        echo "ZOMBIE PROCESS DETECTED: Parent process to kill: $ppid"

        if ps -p "$ppid" > /dev/null 2>&1; then
            kill -9 "$ppid"
            if [[ $? -eq 0 ]]; then
                echo "Successfully killed parent process (PID: $ppid)."
            else
                echo "Failed to kill parent process (PID: $ppid)."
            fi
        else
            echo "Parent process (PID: $ppid) is already terminated. Zombie should be cleaned up automatically."
        fi
    done <<< "$zombies"
}


# Main loop
while true; do
    {
    clear   
    #find any zombie processes that may exist
    echo "Checking for zombie processes..."
    # Find zombie processes and clean them
    clean_zombies

    # Capture the top 10 processes by CPU usage
    ps -eo pid,pcpu,pmem,rss,vsize,stat,ppid --sort=-%cpu | head -11 > "$LOG_FILE"

    ./"$OUTPUT_FILE" &
    PID=$! #get pid of the output file while it is still running
    wait "$PID" #wait for the process to stop running
    }
    # Wait for 30 seconds before the next iteration
    sleep 30
done
