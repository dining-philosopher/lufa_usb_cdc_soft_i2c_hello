#!/usr/bin/python
# coding=utf-8


import time
import serial
import sys

if len(sys.argv) < 2:
    print """
Serial terminal emulator

Usage:

./term.py terminal_device

e. g:

./term.py /dev/ttyACM0

or even better:

rlwrap ./term.py /dev/ttyACM0
"""
    exit()

ser = serial.Serial(sys.argv[1])

print "Connected to " + sys.argv[1]

while True:
    out = ""
    while ser.inWaiting() > 0:
        out += ser.read(1)
    print "Response: " + out
    # print "Response: " + ser.readline() # does not read line if it has not \r\n at the end
    command = raw_input("> ")
    if command == "exit":
        ser.close()
        break
    if command != "":
        print "Command: " + command
        ser.write(command + "\r\n")
    # let's wait one second before reading output (let's give device time to answer)
    time.sleep(0.2)
