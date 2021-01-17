#include "logic.h"
#include "config.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "src/modules/debounceClass.h"
#include "input.h"


// void computeLogic( ) {
// 	static uint8_t previousButtonState = 255;

// 	uint8_t newSignalState   = undefined ; 
// 	uint8_t newDetectorState = undefined ; 
// 	uint8_t newFallTimeState = undefined ;
// 	uint8_t newButtonState   = undefined ;
// 	uint8_t newState		 = undefined ;
// 	uint8_t temp 			 = undefined ;
	

// 	if( signal.type == dutchPreSignal || signal.type == germanPreSignal ) { // pre signals do not bother them selfes with locks, direction and detectors.
// 		//newSignalState = processSignals() ; OBSOLETE FUNCTION, NEEDS ALTERING
// 	}
// 	// the SAS can work both with partially detected blocks as fully detected blocks
// 	else if( signal.locked == ON ) {							// if signal is not locked (inverted signal)

// 		if( signal.wasLocked ) {								// if the signal was locked, it's previous state must be assumed
// 			signal.wasLocked = 0 ;
// 			newState = signal.lastState ;
// 		}

// 		if( detector.hasFallen ) { 								// if the detector sees a train (only 1 flank), the state of the section is occupied NOTE MIGHT BE OFF INSTEAD OF FALLING
// 			detector.hasFallen = 0;

// 			signal.section = occupied ; 
// 			//Serial.println("section is ocupied");
// 		} 

// 		if( signal.connected == 0 ) {							// if not connected to adjacent signal.
// 			newFallTimeState = fallTimeControl() ;				// handles the time based signal states TO BE TESTED

// 			if( newFallTimeState == yellow || newFallTimeState == green ) {
// 				signal.section = available ;
// 				//Serial.println("section is made available by fall time controll");
// 			}
// 		}	
// 		else {

// 			if( signal.section == occupied && nextSignal.transitionedToRed == 1 ) {	// if detector has rissen AND the adjacent signal jumped to red. Our section may now be free
// 				signal.section = available ;  nextSignal.transitionedToRed = 0 ;

// 				temp = red;
// 				//Serial.println("section is available because the following signal became red and I was occupied");
// 			}
// 		}

// 		if( signal.override == 0 ) {											// if a button has overwrittent the signal, it may no longer occur

// 			if( signal.section == occupied ) {									// occupied sections let signal show red, period!!
// 				newState = red ;
// 			}	
// 			else if( signal.section == available ) {							// non occupied sections may display green or yellow depening on the next signal.
// 				if( nextSignal.transitionedToYellow == 1 ) {
// 					nextSignal.transitionedToYellow = 0 ;

// 					newState = green ; //Serial.println("prev signal became yellow, I as combi signal became green");
// 				}
// 				if( temp == red ) {

// 					if( signal.type == mainSignal ) {  newState = green ; }//Serial.println("prev signal became red, I as main signal became green"); }
// 					if( signal.type == combiSignal ){  newState = yellow; }//Serial.println("prev signal became red, I as combi signal became yellow"); }
// 				}
// 			}
// 		}

// 		newButtonState = processButtons();
// 		if( newButtonState == green ) 	{ signal.override = 0 ; }
// 		else 							{ signal.override = 1 ; }
// 	}	

// 	else {														// if signal is locked, the state is unconditional red
// 		signal.state = red ;
// 		signal.wasLocked = 1 ;
// 		return ;
// 	}

// 	/*if(		 newSignalState   != undefined ) { newState = newSignalState  ;  } //Serial.println("newSignalState"); }
// 	else if( newDetectorState != undefined ) { newState = newDetectorState ; } //Serial.println("newDetectorState"); }
// 	else if( newFallTimeState != undefined ) { newState = newFallTimeState ; } //Serial.println("newFallTimeState"); }
// 	else */if( newButtonState   != undefined ) { newState = newButtonState ;   } //Serial.println("newButtonState"); }

// 	if( newState != undefined ) {


// 		signal.lastState = signal.state = newState ; 	// if a new state is selected, adopt it and store it.

// 		// if( newState == red && signal.locked == 0 ) { digitalWrite( relayPin, HIGH ) ; }	// HANDLED IN DEDICATED FUNCTION
// 		// else				  						{ digitalWrite( relayPin,  LOW ) ; }


// 		// //Serial.print("new state = ");  // print new state for debugging purposes BASLABEL DELETE ME WHEN DONE
// 		// switch( newState ) {
// 		// 	printNewState( red ) ;
// 		// 	printNewState( green ) ;
// 		// 	printNewState( yellow ) ;
// 		// 	printNewState( undefined ) ;
// 		// 	printNewState( expectGreen ) ;
// 		// 	printNewState( expectYellow ) ;
// 		// 	printNewState( expectRed ) ;
// 		// 	printNewState( driveOnSight ) ;
// 		// }
// 	}
// }
#define setStates(x,y,z) greenLed.state = x ; yellowLed.state = y ; redLed.state = z ;

void setLedStates( ) {
	switch( signal.state ) {
		case green:		setStates(1,0,0); break;
		case yellow:	setStates(0,1,0); break;
		case red:		setStates(0,0,1); break;
	}
}

uint8_t checkNextSignal( ) {
	if( nextSignal.transitionedToYellow  )
	{
		nextSignal.transitionedToYellow = 0 ;
		return green ;
	}
	if( nextSignal.transitionedToRed )
	{
		if( signal.type == mainSignal  ) return green ; //Serial.println("prev signal became red, I as main signal became green"); }
		if( signal.type == combiSignal ) return yellow; //Serial.println("prev signal became red, I as combi signal became yellow"); }
	}
}


void computeLogic( ) {

	if( signal.locked ) return ;	// if signal is locked, don't do anything

	// HANDLE BUTTONS
	uint8_t newButtonState = processButtons();
	if( newButtonState )
	{
		if( newButtonState == green ) 
		{ 
			signal.override = 0 ;
			if( signal.section == occupied )
			{
				signal.state = driveOnSight ;
			}
			else
			{
				signal.state = green ;
			}
			return ;
		}
		else 
		{ 
			signal.override = 1 ; 
			signal.state = newButtonState ; // yellow or red
			
			return ;
		}
	}
	// NO BUTTONS ARE PRESSED

	// IS SIGNAL OVERRIDDEN BY BUTTONS?
	if( signal.override ) return ; 
	
	// IS TRACK OCCUPIED?
	if( signal.section == occupied ) {
		signal.state = red ;
		return ;
	}

	// NOT CONNECTED TO ADJACENT SIGNAL
	if( signal.connected == false )
	{
		uint8_t newState = fallTimeControl() ; // signal handled on time base
		if( newState )
		{
			signal.state = newState ;
		}
		return ;
	}

	// CONNECTED TO ADJACENT SIGNAL
	uint8_t newState = checkNextSignal( ) ;
	if( newState )
	{
		signal.state = newState ;
	}
}
	
	
		

