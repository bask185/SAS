#!/bin/bash
python.exe updateTimers.py
python.exe updateIO.py
echo "COMPILING"
arduino-cli.exe compile -b arduino:avr:nano ~/Documents/software/SAS -e
echo "UPLOADING"
arduino-cli.exe upload -b arduino:avr:nano:cpu=atmega328old -p COM3 -i ~/Documents/software/SAS/build/arduino.avr.nano/SAS.ino.with_bootloader.hex
rm -r builds/arduino.avr.nano/*

exit