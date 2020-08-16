#!/bin/bash
python.exe updateTimers.py
python.exe updateIO.py
arduino-cli compile -b arduino:avr:nano ~/Documents/software/SAS
exit