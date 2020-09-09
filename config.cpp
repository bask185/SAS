#include "config.h"
#include "src/basics/io.h"

uint8_t	mode;
uint8_t previousState = 255;
uint8_t rxFrequency ;

Signal signal ;
NextSignal nextSignal ;

Led redLed    ;
Led yellowLed ;
Led greenLed  ;

Debounce detector( detectorPin );
Debounce lockSignal( lockPin );
Debounce redButton( redPin );
Debounce yellowButton( yellowPin );
Debounce greenButton( greenPin );