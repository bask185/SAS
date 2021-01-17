#include "config.h"
#include "src/basics/io.h"

uint8_t	mode;
uint8_t previousState = 255;
uint8_t rxFrequency = 0 ;

uint8_t servoPos ;
uint8_t redSeervoPos ;
uint8_t greenServoPos ;

Signal signal ;
NextSignal nextSignal ;

Led redLed    ;
Led yellowLed ;
Led greenLed  ;

Debounce detector( detectorPin );
Debounce directionSignal( directionPin );
Debounce redButton( redButtonPin );
Debounce yellowButton( yellowButtonPin );
Debounce greenButton( greenButtonPin );