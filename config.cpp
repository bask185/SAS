#include "config.h"
#include "src/basics/io.h"


void debug( String txt ) {
	#ifdef DEBUG
	Serial.println(txt);
	#endif
}

uint8_t	mode;
uint8_t previousState = 255;
uint8_t rxFrequency = 0 ;



uint8_t greenButtonState ;
uint8_t redButtonState ;
uint8_t yellowButtonState ;
uint8_t detectorState ;

Signal signal ;
NextSignal nextSignal ;

Servo semaphore ;
uint8_t servoPos ;
uint8_t servoPosMax ;
uint8_t servoPosMin ;

Led redLed    ;
Led yellowLed ;
Led greenLed  ;

Debounce detector( detectorPin );			// needs altering, for different debouncing
Debounce directionSignal( directionPin );
Debounce redButton( redButtonPin );
Debounce yellowButton( yellowButtonPin );
Debounce greenButton( greenButtonPin );
Debounce receiver( Rx );