import os

import serial as s
import sys
import signal
import time

Transmitter = True
Receiver = False

mode = Transmitter
if len(sys.argv) < 2:
    print("Port is missing")
    exit()

if len(sys.argv) == 3:
    mode = Receiver
    file = open(sys.argv[2], 'w')

ser = None
try:
    ser = s.Serial(sys.argv[1], baudrate=115200)
except s.SerialException as e:
    print("\033[91mOperation failed for\033[1m", (sys.argv[1]).split("/")[2], "\033[0m")
    exit()


def handler(signum, frame):
    ser.close()
    file.close()
    print('\033[94m', os.getpid(), "\033[0mstopped and cleaned!")
    exit()


signal.signal(signal.SIGABRT, handler)
ser.write(b'R\n')

attempted = False

while True:
    try:
        if mode == Receiver:
            line = ser.readline()
            if line:
                L = line.decode('utf-8', errors='ignore')
                file.write(str(time.time()) + ":" + L)
        else:
            command = input("Command (c,C,p,P,R,X,/0,q): ")
            if command == 'q':
                ser.close()
                exit()
            ser.write(command.encode() + b'\n')
    except Exception as e:  # Sometimes serial is not setup, so we retry once
        print("Error: {0}".format(e))
        ser = s.Serial(sys.argv[1], baudrate=115200)
        if attempted:
            file.close()
            ser.close()
            exit()
        else:
            attempted = True

