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
        
        # Capture the top 10 processes by CPU usage
        ps -eo pid,pcpu,pmem,rss,vsize,stat,ppid --sort=-%cpu | head -11 > "$LOG_FILE"


        echo "Running output file at $(date)"
        ./"$OUTPUT_FILE" &
        PID=$! #get pid of the ouput file while it is still running
        echo $PID #DEBUGGING REMOVE LATER
        wait "$PID" #wait for the proccess to stop running
        
    }
    # Wait for 30 seconds before the next iteration
    sleep 30
done
