#!/bin/bash

# Introduction ################################################################
# This script is used to run the normal mode of the experiment. It will start
# logging data from all the nodes and then stop the logging after a certain
# duration. The data will be plotted and saved in the Log directory.

YELOW='\033[1;33m'
REDDD='\033[0;31m'
GREEN='\033[0;32m'
CLEAR='\033[0m'
PATTERN='serial-log.py'
SLEEP_DURATION=5                 # define the duration of logging in seconds

# Check if the nodes are connected ############################################
clear
echo -e "${YELOW}Checking if the nodes are connected...${CLEAR}"

NEW_NODE=$(echo $NODE_RX | cut -d '/' -f 3)
HAS_NODE_RX=$(ls /dev | grep -c "$NEW_NODE")
if [ "$HAS_NODE_RX" -eq 0 ]; then
  echo -e "\n${REDDD}Node ${NODE_RX} (RX) is not connected!${CLEAR}"
  echo -e "${REDDD}Please connect the node and try again!${CLEAR}"
  exit 1
fi

NEW_NODE=$(echo $NODE_E1 | cut -d '/' -f 3)
HAS_NODE_E1=$(ls /dev | grep -c "$NEW_NODE")
if [ "$HAS_NODE_E1" -eq 0 ]; then
  echo -e "\n${REDDD}Node ${NODE_E1} (E1) is not connected!${CLEAR}"
  echo -e "${REDDD}Please connect the node and try again!${CLEAR}"
  exit 1
fi

NEW_NODE=$(echo $NODE_E2 | cut -d '/' -f 3)
HAS_NODE_E2=$(ls /dev | grep -c "$NEW_NODE")
if [ "$HAS_NODE_E2" -eq 0 ]; then
  echo -e "\n${REDDD}Node ${NODE_E2} (E2) is not connected!${CLEAR}"
  echo -e "${REDDD}Please connect the node and try again!${CLEAR}"
  exit 1
fi

NEW_NODE=$(echo $NODE_E3 | cut -d '/' -f 3)
HAS_NODE_E3=$(ls /dev | grep -c "$NEW_NODE")
if [ "$HAS_NODE_E3" -eq 0 ]; then
  echo -e "\n${REDDD}Node ${NODE_E3} (E3) is not connected!${CLEAR}"
  echo -e "${REDDD}Please connect the node and try again!${CLEAR}"
  exit 1
fi

NEW_NODE=$(echo $NODE_E4 | cut -d '/' -f 3)
HAS_NODE_E4=$(ls /dev | grep -c "$NEW_NODE")
if [ "$HAS_NODE_E4" -eq 0 ]; then
  echo -e "\n${REDDD}Node ${NODE_E4} (E4) is not connected!${CLEAR}"
  echo -e "${REDDD}Please connect the node and try again!${CLEAR}"
  exit 1
fi

# Arrange directories to log data #############################################
if [ -d "Log" ]; then
  fileDir="$(date +"%F--%I-%M-%S")"
  mv Log "DataLogs/${fileDir}"
  echo -e "${YELOW}Log file renamed to" "${REDDD}${fileDir}${YELOW}" "!${CLEAR}"
fi

cp -r _Log Log

# Start logging ###############################################################
echo -e "${GREEN}Logging started!${CLEAR}\n"

python3 serial-log.py "$NODE_RX" Log/Receive.txt &
python3 serial-log.py "$NODE_E1" Log/Eaves-1.txt &
python3 serial-log.py "$NODE_E2" Log/Eaves-2.txt &
python3 serial-log.py "$NODE_E3" Log/Eaves-3.txt &
python3 serial-log.py "$NODE_E4" Log/Eaves-4.txt &
python3 serial-log.py "$NODE_TX"

# Wait for some time ##########################################################
echo -e "${YELOW}Waiting for ${REDDD}${SLEEP_DURATION}${YELOW} seconds...${CLEAR}"
sleep "$SLEEP_DURATION"

# Stop logging ################################################################
PROCESS_IDS=$(pgrep -f ${PATTERN})

for i in $(echo "$PROCESS_IDS" | tr "\n" " ")
do
  COMD=$(ps -p "$i" -o command | awk '{print $3}' | tr "\n" " ")
  echo -e -n "${YELOW}Stopping ${REDDD}${i}${CLEAR} -${LBLUE}${COMD}${CLEAR} >>> "
  kill -6 "$i"
done

# Plot the data ###############################################################
cd Log || exit
python3 plot.py