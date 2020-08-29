#include "cnfig.h"
#include "src/basics/io.h"

uint8_t	mode;
uint8_t previousState = 255;

Signal signal ;
NextSignal nextSignal ;
Led redLed, greenLed, yellowLed ;

Debounce detector( detectorPin );
Debounce lockSignal( lockPin );
Debounce redButton( redPin );
Debounce yellowButton( yellowPin );
Debounce greenButton( greenPin );