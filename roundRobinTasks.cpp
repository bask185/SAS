
#include "src/modules/debounceClass.h"
#include "roundRobinTasks.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
//#include "teachIn.h"
#include <Servo.h>

#define printNewState(x) case x: Serial.println(#x); break;

Debounce detector( detectorPin );
Debounce lockSignal( lockPin );
Debounce redButton( redPin );
Debounce yellowButton( yellowPin );
Debounce greenButton( greenPin );

/* signals to and from signals explained
a signal can be controlled by several types of inputs:
 1* the section feedback IO behind the signal will put the signal on red, uncondionally.
 2* if this signal is not followed by another signal, the red signal will turn green after a certain amount of time.
 3* if the signal receives a signal from a following signal, the signal may become green or yellow depening on it's type.
 4* optional buttons may override the signals upon a press.

 a signal may pull down an open collector line to the previous signal to indicate that that signal is red.
 Upon receiving this signal, the previous signal may turn itself green if it is a main type signal. A combi signal will
 become yellow instead. A yellow showing combi signal will signal green  to the previous signal so that one may become green.

 a pre signal will addopt the state of the previous signal and will relay the red/green signal to the previous signal.

*/

Servo  motor;

uint8_t newState ;
uint8_t	mode;
uint8_t previousState = 255;

enum signalTypes {
	mainSignal,
	combiSignal,
	germanPreSignal,
	dutchPreSignal,
	entrySignal,
} ;

enum signalStates {
	undefined,
	red,		// main signal states
	yellow,
	green,
	driveOnSight,
	expectGreen, // pre signal states
	expectYellow,
	expectRed
} ;

enum sections {
	occupied,
	available,
} ;

struct {
	uint8_t buttons ;
	uint8_t detectorState ; 
	uint8_t sendFreq ; 
	uint8_t recvFreq ; 
	uint8_t nextState ; 
	uint8_t locked ; 
	uint8_t redLedState ; 
	uint8_t yellowLedState ; 
	uint8_t greenLedState ; 
	uint8_t state ; 
	uint8_t type ;
	uint8_t section ;
	uint8_t wasLocked ; 
	uint8_t lastState ;
	uint8_t override ;
} signal ;


const uint8_t greenFreq = 10 ;
const uint8_t yellowFreq = 20 ;
const uint8_t redFreq = 30 ;

#define printType(x) case x: Serial.println(#x); break
void printStuff(){
	if( !printT ) { printT = 200;

		Serial.write( 12 );
		//Serial.println("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n") ;
		Serial.print("signal type     : ");
		switch(signal.type) {
			printType(mainSignal);
			printType(entrySignal);
			printType(germanPreSignal);
			printType(dutchPreSignal);
			printType(combiSignal);
		}

		Serial.print("buttons         : ");
		switch(signal.buttons) {
			printType(green);
			printType(yellow);
			printType(red);
			printType(undefined);
		}

		Serial.print("send Freq       : "); Serial.println( signal.sendFreq );
		Serial.print("recv Freq       : "); Serial.println( signal.recvFreq );
		Serial.print("nextState       : ");
		switch(signal.nextState) {
			printType(green);
			printType(yellow);
			printType(red);
			printType(undefined);
		}

		Serial.print("locked          : ");
		switch(signal.locked) {
			printType(ON);
			printType(OFF);
		}

		Serial.print("red Led State   : ");
		switch(signal.redLedState) {
			printType(1);
			printType(0);
		}

		Serial.print("yellow led State: ");
		switch(signal.yellowLedState) {
			printType(1);
			printType(0);
		}

		Serial.print("green Led State : ");
		switch(signal.greenLedState) {
			printType(1);
			printType(0);
		}

		Serial.print("section         : ");
		switch(signal.section) {
			printType(available);
			printType(occupied);
		}
	}
}



/******************** READ AND DEBOUNCE INPUTS **********************/
void readInputs() {
	// static uint8_t signal.nextStatePrev ;

	if( !debounceT ) {
		debounceT = 200 ; // 200ms debounce time

		// debounce inputs
		lockSignal.debounce() ;
		detector.debounce() ;
		redButton.debounce() ;
		greenButton.debounce() ;
		yellowButton.debounce() ;
	}

		// read in buttons
		// 	 if(    redButton.getState() == FALLING ) 	{ signal.buttons = red ; 	} // these states may be true for 1 cycle
		// else if(  greenButton.getState() == FALLING )	{ signal.buttons = green ;	 }
		// else if( yellowButton.getState() == FALLING ) 	{ signal.buttons = yellow ;	 }
		// else {
		// 	signal.buttons = undefined ;
		// }

	// read in lock signal
	signal.locked = lockSignal.getState()  ;

	if( signal.type == entrySignal ) { // in the event of an entry signal, the lockpin must be inverted
			 if( signal.locked ==  ON ) signal.locked == OFF ; // 
		else if( signal.locked == OFF ) signal.locked ==  ON ;
	}
	
	// read in the detector
	signal.detectorState = detector.getState() ;

	// read in incomming signal from following module
		 if( signal.recvFreq >  greenFreq - 5 && signal.recvFreq <  greenFreq + 5 ) { signal.nextState =  green ; }
	else if( signal.recvFreq > yellowFreq - 5 && signal.recvFreq < yellowFreq + 5 ) { signal.nextState = yellow ; }
	else if( signal.recvFreq >    redFreq - 5 && signal.recvFreq <    redFreq + 5 ) { signal.nextState =    red ; }
}


uint8_t readSignals() {
	static uint8_t previousSignalState;

	if( previousSignalState != signal.nextState ) {
		previousSignalState = signal.nextState ;

		switch( signal.type ) {
		
		/* DUTCH PRE SIGNAL KNOWS ONLY EXPECTING GREEN OR RED */
		case dutchPreSignal: 
			switch( signal.nextState ) {
				default:	 return undefined ;
				case green:	  
				case yellow: return expectGreen ; 
				case red:	 return expectRed ; 
			}

		/* GERMAN PRE SIGNAL KNOWS EXPECTING GREEN, YELLOW OR RED */
		case germanPreSignal:
			switch( signal.nextState ) {
				default:	 return undefined ;
				case green:  return expectGreen ;
				case yellow: return expectYellow ;
				case red:	 return expectRed ;
			}

		case mainSignal:	// if a main signal receives a signal that the following state is red, it's own state becomes green
			switch( signal.nextState ) {
				default:
				case green:  
				case yellow: return undefined ; // ignore green and yellow states
				case red:	 signal.section = available; return green ;
			}

		case combiSignal:
			switch( signal.nextState ) {
				default:	 return undefined ;
				case green:
				case yellow: return green ;
				case red:	 signal.section = available; return yellow ;
			}
		}
	}
	return undefined ;
}





/***************** COMPUTE LOGIC ************************/
uint8_t fallTimeControl() {
	static uint8_t prevTime = 0;
	if( fallT != prevTime ) {
		prevTime = fallT ;
		Serial.println( fallT ) ;
	}

	if( signal.detectorState == RISING ) {	// if the detector no longer sees the train and there is no incomming signal, the yellow/red delay timer must be set.
		fallT = analogRead( potentiometer ) / 10 ;		// a time based signal must no longer last than 100 seconds tops.
		if( fallT <= 2 ) fallT = 2 ; 					// a mininum of 2 seconds is required
		Serial.print("fall time = ");Serial.println(fallT);
	}

	if( fallT == 1 ) { // in the last second of fall time  // change signal state
		fallT == 0 ;
		//signal.section = available ; DONE IN COMPUTE LOGIC

		if( signal.type == combiSignal) {		// combi signal goes first to yellow before going to green
			if( signal.state == red ) {
				fallT = analogRead( potentiometer ) / 10 ;
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






uint8_t processButtons() {

	if( greenButton.getState () == FALLING ) {
		if( signal.section == occupied )	return driveOnSight ;
		else								return green ;
	}

	if( yellowButton.getState () == FALLING ) {
		if( signal.section == occupied )	return driveOnSight ;
		else								return yellow ;
	}

	if( redButton.getState () == FALLING ) {
		return red ;
	}

	return undefined ;
}
/* General idea:

*	With the SAS modules, one can make make either a dutch
*	or a german signal system. The SAS require some input in the form
*	of dip switches to inform the signal whether what kind of signal
*	it is.

*	The SAS module can be controlled by several inputs. This may be the
*	detector pin, the signal from an adjacent module, input buttons, the 
*	lock pin, a dedicated fall time and button presses. logic is depended 
*	on the type of signal. Different signal types may react different to 
*	different inputs.

*	There is also a certain order of things. The lock pin has #1 priority
*	if a signal is locked it may be that the direction of the tracks is wrong
*	or a turnout is not in the right position. The 2nd highest priority 
*	belongs to the detector pin. If a module is locked or occupied, the 
*	signal is generally showing red.

*	if the signal is connected to an adjacent module, the state of the
*	adjacent module is also examined. This is especially the case for
*	pre signals, but other types may also use these signals.

*	Optionally up to 3 buttons may be connected to a SAS for red,
*	green and yellow states. The green and yellow button may override
*	a red showing signal. If that is the case the state of the signal
*	will display drive-on-sight. A locked signal can not be overriden

*/

/***********************
description
*************************/
void computeLogic() {
	static uint8_t previousButtonState = 255;
	
	// the SAS can work both with partially detected blocks as fully detected blocks

	if( signal.locked == ON ) {									// if signal is not locked (inverted signal)

		if( signal.wasLocked ) {								// if the signal was locked, it's previous state must be assumed
			signal.wasLocked = 0 ;
			newState = signal.lastState ;
			Serial.print("signal unlocked assuming last state: ") ; Serial.println(signal.lastState);
		}

		if( signal.detectorState == FALLING ) { 				// if the detector sees a train, the state of the section is occupied NOTE MIGHT BE OFF INSTEAD OF FALLING
			signal.section = occupied ; 
			newState = red ;
			//signal.state = red ;
			Serial.println("section occupied");
		}	

		if( signal.recvFreq == 0 ) {							// if not connected to adjacent signal.

			newState = fallTimeControl() ;						// handles the time based signal states TO BE TESTED
			if(newState) {Serial.print("fall time returned: ");Serial.println(newState);}

		}	
		else {
			newState = readSignals() ;							// these are the signals from the following modules, only returns a value upon change.
			if(newState){ Serial.print("reading signals returned: ");Serial.println(newState);}
		}

		//newState = processButtons() ;							
		//newState = processButtons() ; 				// occupied section can be overruled by a button press
		byte buttonState = processButtons();
		if( buttonState != previousButtonState ) {				// only read if button state has changed
			previousButtonState = buttonState;
			newState = buttonState ;
		}

		if( newState == green || newState == yellow ) {
			signal.section = available ; 					// after the time-out the section becomes available again.
			Serial.println("section freed");
		}
	}	

	else {														// if signal is locked, the state is red
		signal.state = red ;
		signal.wasLocked = 1 ;
	}

	if( newState != undefined ) {

		previousState = newState;
		signal.state = newState ; 	// if a new state is selected, adpot it.
		signal.lastState = newState;

		newState = undefined ;

		Serial.print("new state = "); 
		switch( newState ) {
			printNewState( red ) ;
			printNewState( green ) ;
			printNewState( yellow ) ;
			printNewState( undefined ) ;
			printNewState( expectGreen ) ;
			printNewState( expectYellow ) ;
			printNewState( expectRed ) ;
			printNewState( driveOnSight ) ;
		}
	}
}


/***************** HANDLE OUTPUTS **********************/
#define clrLeds() digitalWrite(greenLed,LOW);digitalWrite(yellowLed,LOW);digitalWrite(redLed,LOW);
#define setLeds(x,y,z)  if(x)digitalWrite(greenLed,x);if(y)digitalWrite(yellowLed,y);if(z)digitalWrite(redLed,z);

const int pwmMin = 0;
const int pwmMax = 255;
uint8_t pwm = 0;
void fadeLeds() {
	static uint8_t ledSelector ;

	#define setLedStates(x,y,z) signal.greenLedState=x;signal.yellowLedState=y;signal.redLedState=z;				// set the states of the LEDS accordingly
	switch( signal.state ) {// G  Y  R 
	case green:	 		setLedStates( 1, 0, 0 ) ;	break ;
	case yellow: 		setLedStates( 0, 1, 0 ) ;	break ;
	case red:  	 		setLedStates( 0, 0, 1 ) ;	break ;
	case expectGreen:	setLedStates( 1, 0, 0 ) ;	break ; // FOR PRE SIGNAL
	case expectYellow:	setLedStates( 1, 1, 0 ) ;	break ;
	case expectRed:		setLedStates( 0, 1, 0 ) ;	break ; // diode solution for german signal is yet to be made. Perhaps a 4th io pin?
	case driveOnSight: 
		if( !blinkT ) { 
			blinkT = 150 ;
			signal.redLedState = 0;
			signal.greenLedState = 0;
			signal.yellowLedState ^= 1 ;
		} 
		break ;		// 2 second interval toggle yellow LED
	}

	if( !fadeT ) { fadeT = 1 ; // every 1ms
		if( pwm == pwmMax ) {	// if pwm is 0, the previous led has faded off and we can pick a new one
			clrLeds() ;	// first we clear all colors and than set the active one.
			//Serial.print("led state ");Serial.println(signal.state);
			//Serial.print(signal.greenLedState) ;
			//Serial.print(signal.yellowLedState) ;
			//Serial.println(signal.redLedState) ;
			// the german pre signal needs more than 1 active color
			if(  signal.greenLedState == 1 ) { ledSelector = green ; 	setLeds( HIGH,  LOW,  LOW ) ; }
			if( signal.yellowLedState == 1 ) { ledSelector = yellow ;	setLeds(  LOW, HIGH,  LOW ) ; }
			if(    signal.redLedState == 1 ) { ledSelector = red ;		setLeds(  LOW,  LOW, HIGH ) ; }

			pwm --;
		}
		else {
			switch( ledSelector ) {
			default: ledSelector = green;

			case green: 
				if( signal.greenLedState == 0 && pwm < pwmMax ) pwm ++ ;
				if( signal.greenLedState == 0 && pwm < pwmMax ) pwm ++ ;
				if( signal.greenLedState == 1 && pwm > pwmMin ) pwm -- ;
				if( signal.greenLedState == 1 && pwm > pwmMin ) pwm -- ;
				break;

			case yellow:
				if( signal.yellowLedState == 0 && pwm < pwmMax ) pwm ++ ;
				if( signal.yellowLedState == 0 && pwm < pwmMax ) pwm ++ ;
				if( signal.yellowLedState == 1 && pwm > pwmMin ) pwm -- ;
				if( signal.yellowLedState == 1 && pwm > pwmMin ) pwm -- ;
				break;

			case red:
				if( signal.redLedState == 0 && pwm < pwmMax ) pwm ++ ;
				if( signal.redLedState == 0 && pwm < pwmMax ) pwm ++ ;
				if( signal.redLedState == 1 && pwm > pwmMin ) pwm -- ;
				if( signal.redLedState == 1 && pwm > pwmMin ) pwm -- ;
				break ;
			}
			//if( pwm != 0 && pwm != pwmMax ) Serial.println(pwm);
			analogWrite( pwmPin, pwm ) ;
		}
	}
}

// minature state-machine to controll the servo movement including mass innertia
enum servoStates {
	bounceAfterRaising,
	bounceAfterLowering,
	raising,
	lowering,
	up,
	down,
	servoInterval = 20,
} ;


const int servoPosMax = 135 ;
const int servoPosMin = 45 ;
const int massInertiaSteps = 9 ;
uint8_t servoPos;


void servoControl() {
	static uint8_t servoState = up ;

	if( !servoT ) { servoT = servoInterval ; // 20ms?

		switch( servoState ) {
			static uint8_t steps = 0 ;

		case up:
			if( signal.state == yellow 
			||	signal.state == red ) { 
				servoState = raising ;
			}
			break ;
			
			
		case lowering:
			if( servoPos < servoPosMin ) servoPos -- ;

			if( servoPos ==  servoPosMin ) servoState = bounceAfterLowering ;
			servoState = bounceAfterLowering ;
			break ;

		case bounceAfterLowering:
			if( steps < massInertiaSteps / 2 )	servoPos -- ;
			else  								servoPos ++ ;

			steps ++ ;
			if( steps == massInertiaSteps ) {
				steps = 0 ;
				servoState = down ;
			}
			break ;

		case down:
			if( signal.state == green ) servoState = raising ;
			break ;

		case raising: 
			if( servoPos < servoPosMax ) servoPos++ ;

			if( servoPos ==  servoPosMax ) servoState = bounceAfterRaising ;
			break ; 

		case bounceAfterRaising:
			if( steps < massInertiaSteps / 2 )	servoPos ++ ;
			else  								servoPos -- ;

			steps ++ ;
			if( steps == massInertiaSteps ) {
				steps = 0 ;
				servoState = up ;
			}
			break ; 
		}
		motor.write( servoPos ) ;
	}
}


// 20Hz  -> 50ms
// 100Hz -> 10ms
void sendSignals() {

	static uint8_t state = 0, counter = 0;

	if( !sendFreqT ) {
		sendFreqT = map( /*signal.sendFreq*/ mode , 20, 100, 50, 10 ) ;	// TEMPORARILY OVERRULED BY SERIAL INPUT

		static uint8_t prevFreq ;
		if( signal.sendFreq != prevFreq ) {
			Serial.print("new freq = ");Serial.println(signal.sendFreq);
			prevFreq = signal.sendFreq;
		}
		

		//if( ++counter == 10 ) {
			//counter = 0;
			//Serial.print("send Frequency is: ");
			//Serial.println( sendFreqT );
		//}

		switch( signal.state ) { // note pre signals are not supposed to send signals back
			case green:			signal.sendFreq = greenFreq ;	break ;
			case yellow:		signal.sendFreq = yellowFreq ;	break ;
			case red:			signal.sendFreq = redFreq ;		break ;
			case driveOnSight:	signal.sendFreq = yellowFreq ;	break ; // in the event of driving on sight, signal yellow to previous staet
		}
		
		state ^= 1 ;
		if( state ) {
			pinMode( interruptPin, OUTPUT );
			digitalWrite ( interruptPin, HIGH );

			pinMode( nextSignal, OUTPUT ) ;			// pull signal down
			digitalWrite( nextSignal, LOW ) ;
		}
		else {
			digitalWrite ( interruptPin, LOW );
			digitalWrite( nextSignal, HIGH ) ; // delete me
			//pinMode( nextSignal, INPUT_PULLUP ) ;	// pull signal up
		}
	}
}

void readIncFreq() {
	static uint8_t recvFreqPrev = 0 ;
	uint8_t currentTime = 255 - recvFreqT ; // recvFreqT is always decrementing

	signal.recvFreq = map( currentTime, 50, 10, 20, 100 ) ; 

	signal.recvFreq = 0; // delete me

	recvFreqPrev = recvFreqT ;
	recvFreqT = 255 ;
}


void initRR() {
	Serial.println("INITIALIZING");
	//motor.attach( servoPin ) ; 
	//motor.write( 45 ) ; 
	attachInterrupt(digitalPinToInterrupt( interruptPin ), readIncFreq, FALLING) ;

	signal.locked = 0 ;
	signal.section = available ;
	signal.type = combiSignal ;
	signal.state = green ;
	signal.wasLocked = 0 ;
 
	
	Serial.println("INITIALIZING FINISHED");
}




extern void processRoundRobinTasks(void) {
	// physical inputs
	readInputs() ;		// debounces and reads all input signals

	// logic
	computeLogic() ;	// takes input and calculate what all relevant variables should be.

	// physical outputs
	sendSignals() ;		// send the signal to the adjacent module
	fadeLeds() ;		// fade leds in and out to emulate glowblub effects, currently this takes 1/4 of a setting
	servoControl() ;	// handle the arm's servo motor including mass inertia movement

	// debug stuff
	// if( Serial.read () == 'd') printStuff();
	if( Serial.available () ) {
		byte b = Serial.read() ;

		if ( b == 'g' ) mode = 10 ;
		if ( b == 'y' ) mode = 20 ;
		if ( b == 'r' ) mode = 30 ;
		if ( b == 'o' ) mode = 0 ;
		if ( b == 'd' ) printStuff() ;
	}
}