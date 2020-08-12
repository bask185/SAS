#!/bin/bash
python.exe updateTimers.py
python.exe updateIO.py
echo "COMPILING"
arduino-cli compile -b arduino:avr:nano ~/Documents/software/newSignal
echo "UPLOADING"
arduino-cli upload -b arduino:avr:nano:cpu=atmega328old -p COM3 -i ~/Documents/software/newSignal/newSignal.arduino.avr.nano.hex
rm *.hex *.elf
exit