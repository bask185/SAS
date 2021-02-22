#include "config.h"
#include "input.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "src/modules/debounceClass.h"
/*****************************
Monitors state of the buttons, and takes type of signal and occupied status in acount, returns new
signal state
If there is no change in buttons, function returns 'undefined' which is 0
****************************/
uint8_t processButtons( ) {

	if( signal.section == occupied 
	&&( greenButtonState == FALLING || yellowButtonState == FALLING )) 
	{
		signal.section = available ; 
		buttonOveride = 1 ;
		return driveOnSight ; 
	}

	uint8_t retVal ;
	
	if(		 greenButtonState  == FALLING )	retVal = green ;
	else if( yellowButtonState == FALLING )	retVal = yellow ;
	else if( redButtonState    == FALLING )	retVal = red ;
	else 									retVal = undefined ; 
	
	if( retVal ) buttonOveride = 0 ;
	
	return retVal ;
}

void readDetector( ) 
{
	if( signal.locked == 1 ) 		// if signal is locked, do not put state on occupied.
	{
		signal.section = available ;
	}
		
	else 
	if( detectorState == LOW )
	{
		signal.section = occupied ;
		fallT = 0 ;
	}
}


uint8_t fallTimeControl( ) {

	if( rxFrequency ) return undefined ;

	if( detectorState == RISING ) {	// if the detector no longer sees the train and there is no incomming signal, the yellow/red delay timer must be set.
		uint16_t sample = analogRead( potPin ) ;		// a time based signal must no longer last than 100 seconds tops.
		fallT = map( sample, 0, 1023, 0, 60 ) ;

		if( fallT <= 1 ) fallT = 0 ; 						// if pot is turned to full mininum, disable fall time control
		Serial.print("fall time = ");Serial.println(fallT);
	}

	if( fallT == 1 ) { // in the last second of fall time  // change signal state
		fallT = 0 ;
		
		signal.section = available ;

		if( signal.type == combiSignal) {		// combi signal goes first to yellow before going to green
			if( signal.state == red ) {
				uint16_t sample = analogRead( potPin ) ;		// a time based signal must no longer last than 100 seconds tops.
				fallT = map( sample, 0, 1023, 0, 60 ) ;
				if( fallT <= 1 ) fallT = 0 ; 
				return yellow ;
			}
			else if ( signal.state == yellow || signal.state == driveOnSight ) { return green ; }
		}

		if( signal.type == mainSignal ) { 
			return green ;
		}	// main signal goes straight to green
	}
	return undefined ;
}


// FUNCTION CONFIRMED TO WORK 17/01/21
void debounceInputs() {
	#ifndef V1_1
	static uint16_t sample = 0 ;
	#endif
	
	if( !receiverT ) {	// ever ms
		receiverT = 1;
		
		receiver.debounce() ;
	}
	
	if( !debounceT ) {
		debounceT = 50 ; //  debounce time in ms

		// debounce inputs
		directionSignal.debounce() ;
		detector.debounce() ;
		
		#ifndef V1_1
		redButton.debounce() ;
		greenButton.debounce() ;
		yellowButton.debounce() ;
		#else
		sample = analogRead( buttonPin ) ;
		if( sample >    redSample - 100 && sample <    redSample + 100 ) redButtonState    = FALLING ;
		if( sample > yellowSample - 100 && sample < yellowSample + 100 ) yellowButtonState = FALLING ;
		if( sample >  greenSample - 100 && sample <  greenSample + 100 ) greenButtonState  = FALLING ;
		
		#endif

		// stuff states in these variables for the remainder of the program's cycle		

		// if(greenButtonState == FALLING ) PORTB |= (1<<5); 
	    // if(yellowButtonState == FALLING ) PORTB &= ~(1<<5);
		// if(redButtonState == FALLING ) PORTB ^= (1<<5);
	}
	detectorState = detector.getState() ;
	
	#ifndef V1_1
	yellowButtonState = yellowButton.getState() ;
	redButtonState = redButton.getState() ;
	greenButtonState = greenButton.getState() ;	
	#endif
}

/*******************************
This function checks changes of adjacent signal, and act accordingly
***********************************/
uint8_t checkNextSignal( ) {
	
	switch( signal.type ) {
		case mainSignal:
		if( nextSignal.transitionedToRed ) {
			nextSignal.transitionedToRed = 0 ;
			return green ;
		}
		if( nextSignal.transitionedToYellow ) {
			nextSignal.transitionedToYellow = 0 ;
			return green ;
		}
		break ;
		
		case combiSignal:
		if( nextSignal.transitionedToRed ) {
			nextSignal.transitionedToRed = 0 ;
			return yellow ;
		}
		if( nextSignal.transitionedToYellow ) {
			nextSignal.transitionedToYellow = 0 ;
			return green ;
		}
		if( nextSignal.transitionedToGreen) { // SHOULD NEVER OCCUR
			nextSignal.transitionedToGreen = 0 ;
			return green ;
		}
		break ;
		
		
		case dutchPreSignal:
		if( nextSignal.transitionedToRed ) {
			nextSignal.transitionedToRed = 0 ;
			return yellow ;
		}
		if( nextSignal.transitionedToGreen) {
			nextSignal.transitionedToGreen = 0 ;
			return red ;
		}
		break;
	}
}

void readIncFreq() { // ISR
	uint8_t state = receiver.getState() ;
	
	//rxFrequency = 0 ;
	if( state == RISING || state == FALLING ) {

		int8_t currentTime = ( 128 - recvFreqT ) ; // recvFreqT is always decrementing.

		rxFrequency =  constrain( currentTime, 0 , 100 ) ;
		
		recvFreqT = 128;
	}
}



void readDirection() {
	uint8_t state = directionSignal.getState() ;
	if( state == OFF )	signal.locked = 1 ;
	else 				signal.locked = 0 ;

	// if( signal.type == entrySignal ) { // in the event of an entry signal, the lockpin must be inverted
	// 	if(		 signal.locked ==  ON ) signal.locked = OFF ; // 
	// 	else if( signal.locked == OFF ) signal.locked =  ON ;
	// }
}





uint8_t processSignals() {
	static uint8_t previousSignalState = 255 ;

	signal.recvFreq = rxFrequency;

	// if( signal.connected == 0 ) return undefined ; // if not connected

	if (	 signal.recvFreq >  greenFreq - 3 && signal.recvFreq <  greenFreq + 3 ) { nextSignal.state =     green ; }
	else if( signal.recvFreq > yellowFreq - 3 && signal.recvFreq < yellowFreq + 3 ) { nextSignal.state =    yellow ; }
	else if( signal.recvFreq >    redFreq - 3 && signal.recvFreq <    redFreq + 3 ) { nextSignal.state =       red ; }
	else {   signal.connected = 0;													  nextSignal.state = undefined ; } // no known frequency means not connected

	if( previousSignalState != nextSignal.state ) {
		previousSignalState = nextSignal.state ;

		Serial.print("rxFrequency"); Serial.println(rxFrequency);

		switch( nextSignal.state ) {	// set these flags
			default: return undefined ;
			case red:    nextSignal.transitionedToRed	 = 1 ; Serial.println(" nextSignal.transitionedToRed"); break ;
			case yellow: nextSignal.transitionedToYellow = 1 ; Serial.println(" nextSignal.transitionedToYellow"); break ;
			case green:	 nextSignal.transitionedToGreen	 = 1 ; Serial.println(" nextSignal.transitionedToGreen");break ;
		}

		switch( signal.type ) {
		
		/* DUTCH PRE SIGNAL KNOWS ONLY EXPECTING GREEN OR RED */
		case dutchPreSignal: 
			switch( nextSignal.state ) {
				default:	 return undefined ; 	// need alteringex
				case green:	 return green ; // expectGreen
				case red:	 return yellow ; // expectRed
			}

		/* GERMAN PRE SIGNAL KNOWS EXPECTING GREEN, YELLOW OR RED */
		case germanPreSignal:
			switch( nextSignal.state ) {
				default:	 return undefined ;
				case green:  return expectGreen ;
				case yellow: return expectYellow ;
				case red:	 return red ;
			}

		case mainSignal:	// if a main signal receives a signal that the following state is red, it's own state becomes green
			switch( nextSignal.state ) {
				default:	 return undefined ; // ignore green and yellow states
				case red:	 return green ;
			}

		case combiSignal:
			switch( nextSignal.state ) {
				default:	 return undefined ;
				case green:
				case yellow: return green ;
				case red:	 return yellow ;
			}
		}
	}
	return undefined ;
}


