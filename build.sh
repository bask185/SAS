#!/bin/bash
python.exe updateTimers.py
python.exe updateIO.py
/C/Users/Gebruiker/Documents/arduino-cli14/arduino-cli compile -b arduino:avr:nano ~/Documents/software/SAS -e

exit