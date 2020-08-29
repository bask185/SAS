#include "logic.h"
#include "config.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "src/modules/debounceClass.h"


void computeLogic(Signal *signal, NextSignal *nextSignal) {
	static uint8_t previousButtonState = 255;

	uint8_t newSignalState   = undefined ; 
	uint8_t newDetectorState = undefined ; 
	uint8_t newFallTimeState = undefined ;
	uint8_t newButtonState   = undefined ;
	uint8_t newState		 = undefined ;
	uint8_t temp 			 = undefined ;
	

	if( signal->type == dutchPreSignal || signal->type == germanPreSignal ) { // pre signals do not bother them selfes with locks, direction and detectors.
		//newSignalState = processSignals() ; OBSOLETE FUNCTION, NEEDS ALTERING
	}
	// the SAS can work both with partially detected blocks as fully detected blocks
	else if( signal->locked == ON ) {							// if signal is not locked (inverted signal)

		if( signal->wasLocked ) {								// if the signal was locked, it's previous state must be assumed
			signal->wasLocked = 0 ;
			newState = signal->lastState ;
		}

		if( detector.hasFallen ) { 								// if the detector sees a train (only 1 flank), the state of the section is occupied NOTE MIGHT BE OFF INSTEAD OF FALLING
			detector.hasFallen = 0;

			signal->section = occupied ; 
			Serial.println("section is ocupied");
		} 

		if( signal->connected == 0 ) {							// if not connected to adjacent signal->
			newFallTimeState = fallTimeControl() ;				// handles the time based signal states TO BE TESTED

			if( newFallTimeState == yellow || newFallTimeState == green ) {
				signal->section = available ;
				Serial.println("section is made available by fall time controll");
			}
		}	
		else {

			if( signal->section == occupied && nextSignal->transitionedToRed == 1 ) {	// if detector has rissen AND the adjacent signal jumped to red. Our section may now be free
				signal->section = available ;  nextSignal->transitionedToRed = 0 ;

				temp = red;
				Serial.println("section is available because the following signal became red and I was occupied");
			}
		}

		if( signal->override == 0 ) {											// if a button has overwrittent the signal, it may no longer occur

			if( signal->section == occupied ) {									// occupied sections let signal show red, period!!
				newState = red ;
			}	
			else if( signal->section == available ) {							// non occupied sections may display green or yellow depening on the next signal->
				if( nextSignal->transitionedToYellow == 1 ) {
					nextSignal->transitionedToYellow = 0 ;

					newState = green ; Serial.println("prev signal became yellow, I as combi signal became green");
				}
				if( temp == red ) {

					if( signal->type == mainSignal ) {  newState = green ; Serial.println("prev signal became red, I as main signal became green"); }
					if( signal->type == combiSignal ){  newState = yellow; Serial.println("prev signal became red, I as combi signal became yellow"); }
				}
			}
		}

		newButtonState = processButtons();
		if( newButtonState == green ) 	{ signal->override = 0 ; }
		else 							{ signal->override = 1 ; }
	}	

	else {														// if signal is locked, the state is unconditional red
		signal->state = red ;
		signal->wasLocked = 1 ;
		return ;
	}

	//newState |= newSignalState | newDetectorState | newFallTimeState | newButtonState ; was a brainfart but is not safe when more than one newstate is set. It is unlikely, but t
	if(		 newSignalState   != undefined ) { newState = newSignalState  ;  Serial.println("newSignalState"); }
	else if( newDetectorState != undefined ) { newState = newDetectorState ; Serial.println("newDetectorState"); }
	else if( newFallTimeState != undefined ) { newState = newFallTimeState ; Serial.println("newFallTimeState"); }
	else if( newButtonState   != undefined ) { newState = newButtonState ;   Serial.println("newButtonState"); }

	if( newState != undefined ) {

		signal->lastState = signal->state = newState ; 	// if a new state is selected, adopt it and store it.

		if( newState == red && signal->locked == 0 ) { digitalWrite( relayPin, HIGH ) ; }	// if signal is locked, relay must be low
		else				  						{ digitalWrite( relayPin,  LOW ) ; }


		// Serial.print("new state = ");  // print new state for debugging purposes BASLABEL DELETE ME WHEN DONE
		// switch( newState ) {
		// 	printNewState( red ) ;
		// 	printNewState( green ) ;
		// 	printNewState( yellow ) ;
		// 	printNewState( undefined ) ;
		// 	printNewState( expectGreen ) ;
		// 	printNewState( expectYellow ) ;
		// 	printNewState( expectRed ) ;
		// 	printNewState( driveOnSight ) ;
		// }
	}
}