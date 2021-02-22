#include "logic.h"
#include "config.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "src/modules/debounceClass.h"
#include "input.h"

#define newLogic

#define setStates(x,y,z) greenLed.state = x ; yellowLed.state = y ; redLed.state = z ;

void blink() {
	static uint8_t mem = 0 ;

	if( mem == 0 ) {
		mem = 1;
		setStates(0,1,0);
	} else {
		mem = 0;
		setStates(0,0,0);
	}
}

/*** this function ensures the fade led function can do it's job ***/
void setLedStates( ) {
	switch( signal.state ) {
		case green:		setStates(1,0,0); break;
		case yellow:	setStates(0,1,0); break;
		case red:		setStates(0,0,1); break;
		case driveOnSight: repeat( &blinkT, 100, blink ) ; break ;
	}
}


/* Inputs in order of significance
* direction Line
* button override
* occupied state
* adjacent signals
*/


#ifdef newLogic

void computeLogic()
{
	uint8_t newState ;
	
//********************** DIRECTION LINE **********************/
	if( directionSignal == LOW )
	{								// signal is driven from behind, discard all other input
		signal.state = red ;
		return ;
	}
	else
	if( directionSignal == RISING )	// if direction line is released, a check is made is on the current detector state. 
	{
		if( detectorState == LOW )
		{
			signal.section = occupied ;
		}
		else
		{
			signal.section = available ;
		}
	}
/**************************************************/


//********************** BUTTONS **********************/
	newState = readInputs() ;
	if( newState )
	{
		signal.state = newState
	}
	if( signal.override == true ) return ; 			// if buttons have overridden signal, discard all other input 
/**************************************************/


/********************** DETECTOR **********************/
	if( signal.section == occupied )
	{
		signal.state = red ;
	}
	
	if( detectorState == LOW ) return;					// if the train is still seen, the following input can be disregarded.
/**************************************************/


/********************** ADJACENT SIGNALS **********************/
	if( signal.connected == 1 )
	{
		newState = checkNextSignal() ;
		
		if( nextSignal.transitionedToRed )
		{
			nextSignal.transitionedToRed = 0 ;
			signal.section = available ;  
		}
	}
/**************************************************/


/********************** FALL TIME CONTROL **********************/
	else
	{		
		uint8_t newFallTimeState = fallTimeControl() ;				// handles the time based signal states

		if( newFallTimeState != undefined )
		{
			signal.state = newFallTimeState ;
		}
	}
}
	
	
	
#else
void computeLogic( ) {
	static uint8_t previousButtonState = 255;

	uint8_t newSignalState   = undefined ; 
	uint8_t newDetectorState = undefined ; 
	uint8_t newFallTimeState = undefined ;
	uint8_t newButtonState   = undefined ;
	uint8_t newState		 = undefined ;
	uint8_t temp 			 = undefined ;
	

	//if( signal.type == dutchPreSignal || signal.type == germanPreSignal ) { // pre signals do not bother them selfes with locks, direction and detectors.
	newSignalState = processSignals() ;// OBSOLETE FUNCTION, NEEDS ALTERING
	//}
	// the SAS can work both with partially detected blocks as fully detected blocks
	if( signal.locked == 0 ) {							// if signal is not locked (inverted signal)

		if( signal.wasLocked ) {								// if the signal was locked, it's previous state must be assumed
			signal.wasLocked = 0 ;
			newState = signal.lastState ;
		}

		if( signal.connected == 0 ) {							// if not connected to adjacent signal.
			newFallTimeState = fallTimeControl() ;				// handles the time based signal states TO BE TESTED

			if( newFallTimeState == yellow || newFallTimeState == green ) {
				signal.section = available ;
				//Serial.println("section is made available by fall time controll");
			}
		}	
		else {

			if( signal.section == occupied && nextSignal.transitionedToRed == 1 ) {	// if detector has rissen AND the adjacent signal jumped to red. Our section may now be free
				signal.section = available ;  nextSignal.transitionedToRed = 0 ;

				temp = red;
				//Serial.println("section is available because the following signal became red and I was occupied");
			}
		}

		//if( signal.override == 0 ) {											// if a button has overwrittent the signal, it may no longer occur

			if( detectorState == FALLING ) {									// occupied sections let signal show red, period!!
				newState = red ;
			}	
			// else if( signal.section == available ) {							// non occupied sections may display green or yellow depening on the next signal.
			// 	if( nextSignal.transitionedToYellow == 1 ) {
			// 		nextSignal.transitionedToYellow = 0 ;

			// 		newState = green ; //Serial.println("prev signal became yellow, I as combi signal became green");
			// 	}
			// 	if( temp == red ) {

			// 		if( signal.type == mainSignal ) {  newState = green ; }//Serial.println("prev signal became red, I as main signal became green"); }
			// 		if( signal.type == combiSignal ){  newState = yellow; }//Serial.println("prev signal became red, I as combi signal became yellow"); }
			// 	}
			// }
		//}

		newButtonState = processButtons();
		if( newButtonState == green ) 	{ signal.override = 0 ; }
		else 							{ signal.override = 1 ; }
	}	

	else {														// if signal is locked, the state is unconditional red
		signal.state = red ;
		signal.wasLocked = 1 ;
		return ;
	}

	if(		 newSignalState   != undefined ) { newState = newSignalState  ;  Serial.println("newSignalState"); }
	else if( newDetectorState != undefined ) { newState = newDetectorState ; Serial.println("newDetectorState"); }
	else if( newFallTimeState != undefined ) { newState = newFallTimeState ; Serial.println("newFallTimeState"); }
	else if( newButtonState   != undefined ) { newState = newButtonState ;   Serial.println("newButtonState"); }

	if( newState != undefined ) {


		signal.lastState = signal.state = newState ; 	// if a new state is selected, adopt it and store it.

		// if( newState == red && signal.locked == 0 ) { digitalWrite( relayPin, HIGH ) ; }	// HANDLED IN DEDICATED FUNCTION
		// else				  						{ digitalWrite( relayPin,  LOW ) ; }


		// //Serial.print("new state = ");  // print new state for debugging purposes BASLABEL DELETE ME WHEN DONE
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

#endif
