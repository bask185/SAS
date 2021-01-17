#include "debounceClass.h"

#define OLD

Debounce::Debounce(unsigned char _pin) {
	pinMode(_pin, INPUT_PULLUP); // take note I use a pull-up resistor by default
	pin = _pin;
	state = ON ;
}

unsigned char Debounce::getState() {
	byte retValue = state;

	if(state == RISING)  {
		state = ON; // take note I use a pull-up resistor
		hasRissen = true ;
	}
	if(state == FALLING) {
		state = OFF;  // rising or falling may be returned only once
		hasFallen = true ;
	}

	return retValue;
}

void Debounce::debounce() {
	bool newSample = digitalRead( pin );

	if(newSample == oldSample) {	// if the same state is detected atleast twice in 20ms...
	
		if(newSample != statePrev) { // if a flank change occured return RISING or FALLING
			statePrev = newSample ;

			if(newSample)	state = RISING; 
			else			state = FALLING;
		}

		else {						// or if there is no flank change return PRESSED or RELEASED
			if(newSample)	state = ON; 
			else			state = OFF;
		}
	}
	oldSample = newSample;
}




