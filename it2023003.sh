#!/bin/bash

# Define the files that will be used
LOG_FILE="processes.txt"
C_FILE="it2023003.c"
OUTPUT_FILE="it2023003.out"



# Function to handle graceful exit
cleanup() {
    #check if the pid is not empty and the proccess is running
    if [ -n "$PID" ] && ps -p "$PID" > /dev/null 2>&1; then #silence the output of the ps command
        echo "Terminating process $PID..."
        kill -9 "$PID"
    fi
    echo "Script terminated at $(date)" > /dev/tty
    exit
}

# Trap signals for graceful termination
trap cleanup SIGINT SIGTERM


# Start the script

#Compile the C program
if [ ! -e "$OUTPUT_FILE" ];
then
    echo "Compiling $C_FILE ..."
    gcc "$C_FILE" -pthread -o "$OUTPUT_FILE"

    if [ $? -ne 0 ]; then
        echo "Compilation failed."
        exit 1
    else
        echo "Compilation successful."
        sleep 5
        clear
    fi
fi


# Main loop
while true; do
    {
    #find any zombie processes that may exist
    echo "Checking for zombie processes..."
    
    # Find zombie processes
    zombies=$(ps -eo pid,ppid,stat,comm | awk '$3 ~ /Z/ {print $1, $2}')
    if [[ -z "$zombies" ]]; then
        echo "No zombie processes found."
    else
    while read -r pid ppid; do
        echo "ZOMBIE PROCESS ALARM: parent process to kill: $ppid"
        if ps -p "$ppid" > /dev/null 2>&1; then
        kill -9 "$ppid"      
            if [[ $? -eq 0 ]]; then
                echo "Successfully killed parent process (PID: $ppid)."
            else
                echo "Failed to kill parent process (PID: $ppid)."
            fi
        # else
        # echo "Parent process (PID: $ppid) is no longer running. Zombie should be cleaned up automatically."
        fi
    done <<< "zombies"
    fi
    # Capture the top 10 processes by CPU usage
    ps -eo pid,pcpu,pmem,rss,vsize,stat,ppid --sort=-%cpu | head -11 > "$LOG_FILE"


    echo "Running output file at $(date)"
    ./"$OUTPUT_FILE" &
    PID=$! #get pid of the ouput file while it is still running
    wait "$PID" #wait for the proccess to stop running
        
    }
    # Wait for 30 seconds before the next iteration
    sleep 30
done
