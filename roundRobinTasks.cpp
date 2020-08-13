
#include "roundRobinTasks.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "teachIn.h"

/* signals to and from signals explained
a signal can be controlled by several types of inputs:
 1* the track feedback IO behind the signal will put the signal on red, uncondionally.
 2* if this signal is not followed by another signal, the red signal will turn green after a certain amount of time.
 3* if the signal receives a signal from a following signal, the signal may become green or yellow depening on it's type.
 4* optional buttons may override the signals upon a press.

 a signal may pull down an open collector line to the previous signal to indicate that that signal is red.
 Upon receiving this signal, the previous signal may turn itself green if it is a main type signal. A combi signal will
 become yellow instead. A yellow showing combi signal will signal green  to the previous signal so that one may become green.

 a pre signal will addopt the state of the previous signal and will relay the red/green signal to the previous signal.

*/



enum signal.states {
	undefined,

	red,		// main signal states
	yellow,
	green,
	driveOnSight,

	expectGreen, // pre signal states
	expectYellow,
	expectRed
} ;

enum signal.tracks {
	occupied,
	available,
} ;

struct {
	uint8_t buttons;
	uint8_t detectorState; 
	uint8_t sendFreq; 
	uint8_t recvFreq; 
	uint8_t nextState; 
	uint8_t state; 
	uint8_t locked; 
	uint8_t redLed; 
	uint8_t yellowLed; 
	uint8_t greenLed; 
	uint8_t state; 
	uint8_t type;
	uint8_t track;
	uint8_t override;
	uint8_t state2be;
} signal;


const uint8_t greenFreq = 10;
const uint8_t yellowFreq = 30;
const uint8_t redFreq = 50;



/******************** READ AND DEBOUNCE INPUTS **********************/
void readInputs() {
	static uint8_t signal.nextStatePrev;

	if( !debounceT ) {
		debounceT = 200 ; // 200ms debounce time

		int val = analogRead( inputButtons ) ; 
			 if( val <  250 ) signal.buttons = red ;
		else if( val <  600 ) signal.buttons = yellow ;
		else if( val < 1000 ) signal.buttons = green ;
		else				  signal.buttons = undefined ;


		// debounce inputs
		lockSignal.debounceInputs() ;
		detector.debounceInputs() ;

		// read in lock signal
		signal.locked = lockSignal.readInput()  ;
		if( signal.type == entrySignal ) { // in the event of an entry signal, the lockpin must be inverted
			 	 if( signal.locked ==  ON ) signal.locked == OFF; // 
			else if( signal.locked == OFF ) signal.locked ==  ON;
		}
		
		// read in the detector
		detectorState = detector.readInput() ;

		// read in incomming signal from following module
			 if( signal.recvFreq >  greenFreq - 5 && signal.recvFreq <  greenFreq + 5 ) { signal.nextState =  green ; }
		else if( signal.recvFreq > yellowFreq - 5 && signal.recvFreq < yellowFreq + 5 ) { signal.nextState = yellow ; }
		else if( signal.recvFreq >    redFreq - 5 && signal.recvFreq <    redFreq + 5 ) { signal.nextState =    red ; }
	}
}

/***************** COMPUTE LOGIC ************************/
void fallTimeControl() {
	if( detectorState == RISING && recvFreq == 0 ) {	// if the detector no longer sees the train and there is no incomming signal, the yellow/red delay timer must be set.
		fallT = analogRead( potentiometer ) / 10;		// a time based signal must no longer last than 100 seconds tops.
		if( fallT <= 2 ) fallT = 2 ; 					// a mininum of 2 seconds is required
		signal.track = available;							// the section is now cleared
	}

	if( fallT == 1 ) { // in the last second of fall time  // change signal state
		fallT == 0;

		if( signal.type & combiSignal) {		// combi signal goes first to yellow before going to green
			if( signal.state == red ) {
				signal.state = yellow ;
				fallT = analogRead( potentiometer ) / 10;	
			}
			else if ( signal.state == yellow ) {		
				signal.state = green;
			}
		}

		if( signal.type == mainSignal ) {	// main signal goes straight to green
			signal.state = green;
		}
	}
}


void processButtons() {
	switch( signal.buttons ) {
	case green:
		if( signal.track == occupied ) signal.state = driveOnSight;
		else						 signal.state = green;
		buttonOverride = false;	// UNSURE IF THIS FLAG WILL BE USED
		break;

	case yellow:
		if( signal.track == occupied ) signal.state = driveOnSight;
		else						 signal.state = yellow;
		buttonOverride = true;
		break;

	case red:
		signal.state = red;
		buttonOverride = true;
		break;
	}
}

void readSignals() {
	switch( signal.type ) {
	
	/* DUTCH PRE SIGNAL KNOWS ONLY EXPECTING GREEN OR RED */
	case dutchPreSignal: 
		switch( signal.nextState ) {
		case green:
		case yellow:
			newSignalState = expectGreen ;
			break;
		case red:
			newSignalState = expectRed ;
			break;
		}
		break;

	/* GERMAN PRE SIGNAL KNOWS EXPECTING GREEN, YELLOW OR RED */
	case germanPreSignal:
		switch( signal.nextState ) {
		case green:
			newSignalState = expectGreen ;
			break ;
		case yellow:
			newSignalState = expectYellow ;
			break ;
		case red:
			newSignalState = expectRed ;
			break ;
		}
		break;

	case mainSignal:	// if a main signal receives a signal that the following state is 
		if( signal.nextState == red ) {
			newSignalState = green ;
			signal.track = available;
		}
		break;

	case combiSignal:
		switch( signal.nextState ) {
		case green:
		case yellow:
			newSignalState = red ;
			break ;
		case red:
			newSignalState = yellow ;
			signal.track = available;
			break;
		}
		break;
	}
}


void computeLogic() {
	static byte previousSignal = 253; // random number
	// the SAS can work both with partially detected blocks as fully detected blocks

	fallTimeControl();													// handles the time base signal states

	if( detectorState == OFF ) { signal.track = occupied ; }				// while the detector sees a train, the SAS ignores signals from adjacent modules
	else { readSignals(); }
	

	if( signal.track == occupied ) { signal.state = red ; }			// occupied track can be overruled by a button press

	processButtons();

	if( signal.locked == OFF ) { signal.state = red ; }				// lock signal overrides everything else
	
}



/***************** HANDLE OUTPUTS **********************/
#define clrColor() digitalWrite(greenLed,LOW);digitalWrite(yellowLed,LOW);digitalWrite(redLed,LOW);
#define setColor(x,y,z)  if(x)digitalWrite(greenLed,x);if(y)digitalWrite(yellowLed,y);if(z)digitalWrite(redLed,z);
void fadeLeds() {
	static uint8_t ledSelector;

	if( !fadeT ) { fadeT = 1; // every 1ms
		if( pwm == 0 ) {	// if pwm is 0, the previous led has faded off and we can pick a new one
			clrColor();	// first we clear all colors and than set the active one.
			// the german pre signal needs more than 1 active color
			if(  greenLed.state == 1 ) ledSelector = green; 	setColor( HIGH,  LOW,  LOW );
			if( yellowLed.state == 1 ) ledSelector = yellow;	setColor(  LOW, HIGH,  LOW );
			if(    redLed.state == 1 ) ledSelector = red;	  	setColor(  LOW,  LOW, HIGH );
		}
		else {
			switch( ledSelector ) {

			case green: 
				if( greenLed.state == 1 && pwm < pwmMax ) pwm ++;
				if( greenLed.state == 0 && pwm > pwmMin ) pwm --;
				break;

			case yellow:
				if( yellowLed.state == 1 && pwm < pwmMax ) pwm ++;
				if( yellowLed.state == 0 && pwm > pwmMin ) pwm --;
				break;

			case red:
				if( redLed.state == 1 && pwm < pwmMax ) pwm ++ ;
				if( redLed.state == 0 && pwm > pwmMin ) pwm -- ;
				break;
			}
			analogWrite( pwmPin, pwm ) ;
		}
	}
}


enum servoStates {
	bounceAfterRaising,
	bounceAfterLowering,
	raising,
	lowering,
	up,
	down,
	servoInterval = 10;
} ;

void controlServo() {
	static uint8_t servoState = up;

	if( !servoT ) { servoT = servoInterval ;
		switch( servoState ) {
			case up:
			case lowering:
			case bounceAfterLowering:
			case down:
			case raising:
			case bounceAfterRaising:

			break; }
}

	
#define setSignal(x,y,z) greenLed.state=x;yellowLed.state=y,redLed.state=z
void setOutput() {
	// first set the signals's arm and or LED's according to the current signal.state
	switch( signal.state ) {//		   G  Y  R 
		case green:	setSignal( 1, 0, 0 ) ;	break ;
		case yellow:	setSignal( 0, 1, 0 ) ;	break ;
		case red:  	setSignal( 0, 0, 1 ) ;	break ;
		case driveOnSight: if( !blinkT ) { blinkT = 200; yellowLed.state ^= 1; } break;// 2 second interval toggle yellow LED
	}
}

// 20Hz  -> 50ms
// 100Hz -> 10ms
void sendSignals() {
	static uint8_t state = 0 ;

	if( !sendFreqT ) {
		sendFreqT = map( sendFreq, 20, 100, 50, 10 ) ;

		switch( signal.state ) { // note pre signals are not supposed to send signals back
			case green:	sendFreq = greenFreq;	break;
			case yellow:	sendFreq = yellowFreq;	break;
			case red:		sendFreq = redFreq;		break;
			case driveOnSight:	sendFreq = yellowFreq;	break; // in the event of driving on sight, signal yellow to previous staet
		}
		
		state ^= 1 ;
		if( state ) {
			pinMode( nextSignal, OUTPUT ) ;			// pull signal down
			digitalWrite( nextSignal, LOW ) ;
		}
		else {
			pinMode( nextSignal, INPUT_PULLUP ) ;	// pull signal up
		}
	}
}

void readIncFreq() {
	static uint8_t recvFreqPrev = 0;
	uint8_t time = recvFreqPrev - recvFreqT ;

	recvFreq = map( time, 50, 10, 20, 100 ) ; 

	recvFreqPrev = recvFreqT ;
	recvFreqT = 255 ;
}


void initRR() {
	attachInterrupt(digitalPinToInterrupt( interruptPin ), readIncFreq, FALLING);
}

extern void processRoundRobinTasks(void) {

	readInputs() ;		// read input
	computeLogic() ;	// compute logic, determen what the signal should do
	setOutput() ;		// set the states of the leds and servo accordingly

	sendSignals() ;		// send the signal to the adjacent module
	fadeLeds() ;		// fade leds in and out accoriding to the state
	controlServo();		// handle the arm's servo motor including mass inertia movement
}