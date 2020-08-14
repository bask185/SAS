
#include "roundRobinTasks.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "teachIn.h"

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

enum signal.sections {
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
	uint8_t section;
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
		signal.detectorState = detector.readInput() ;

		// read in incomming signal from following module
			 if( signal.recvFreq >  greenFreq - 5 && signal.recvFreq <  greenFreq + 5 ) { signal.nextState =  green ; }
		else if( signal.recvFreq > yellowFreq - 5 && signal.recvFreq < yellowFreq + 5 ) { signal.nextState = yellow ; }
		else if( signal.recvFreq >    redFreq - 5 && signal.recvFreq <    redFreq + 5 ) { signal.nextState =    red ; }
	}
}


uint8_t readSignals() {
	switch( signal.type ) {
	
	/* DUTCH PRE SIGNAL KNOWS ONLY EXPECTING GREEN OR RED */
	case dutchPreSignal: 
		switch( signal.nextState ) {
			default:	 return undefined;
			case green:
			case yellow: return expectGreen ; 
			case red:	 return expectRed ; 

	/* GERMAN PRE SIGNAL KNOWS EXPECTING GREEN, YELLOW OR RED */
	case germanPreSignal:
		switch( signal.nextState ) {
			default:	 return undefined;
			case green:  return expectGreen ;
			case yellow: return expectYellow ;
			case red:	 return expectRed ;
		}

	case mainSignal:	// if a main signal receives a signal that the following state is 
		switch( signal.nextState ) {
			default:	 return undefined;
			case green:  return undefined ;
			case yellow: return undefined ;
			case red:	 return green;
		}

	case combiSignal:
		switch( signal.nextState ) {
			default:	 return undefined;
			case green:  return red ;
			case yellow: return red ;
			case red:	 return yellow ;
		}
	}

	return undefined ;
}





/***************** COMPUTE LOGIC ************************/
uint8_t fallTimeControl() {
	if( detectorState == RISING ) {	// if the detector no longer sees the train and there is no incomming signal, the yellow/red delay timer must be set.
		fallT = analogRead( potentiometer ) / 10;		// a time based signal must no longer last than 100 seconds tops.
		if( fallT <= 2 ) fallT = 2 ; 					// a mininum of 2 seconds is required
		signal.section = available;						// the section is now cleared
	}

	if( fallT == 1 ) { // in the last second of fall time  // change signal state
		fallT == 0;

		if( signal.type & combiSignal) {		// combi signal goes first to yellow before going to green
			if( signal.state == red ) {
				return yellow ;
				fallT = analogRead( potentiometer ) / 10;	
			}
			else if ( signal.state == yellow ) {		
				return green;
			}
		}

		if( signal.type == mainSignal ) {	// main signal goes straight to green
			return green;
		}
	}

	return undefined;
}


uint8_t processButtons() {
	switch( signal.buttons ) {
	default: return undefined;

	case green:
		if( signal.section == occupied )	return driveOnSight ;
		else								return green ;
		buttonOverride = false;	// UNSURE IF THIS FLAG WILL BE USED
		break;

	case yellow:
		if( signal.section == occupied )	return driveOnSight ;
		else								return yellow ;
		buttonOverride = true;
		break;

	case red:
		return red;
		buttonOverride = true;
		break;
	}
}

void computeLogic() {
	uint8_t newState;
	// the SAS can work both with partially detected blocks as fully detected blocks

	if( signal.locked == ON ) {								// if signal is not locked (inverted)

		if( signal.recvFreq == 0 ) {						// not connected to adjacent signal.
			 newState = fallTimeControl();					// handles the time based signal states
		}

		if( signal.detectorState == OFF ) { 				// while the detector sees a train, the state of the section is occupied
			signal.section = occupied ; 
		}												
		else {												// if the detector does not see a train the module listen to the adjacent connected signal;
			newState = readSignals() ;
		}

		if( signal.section == occupied ) { 					// if section is occupied -> red signal
			newState= red ;
		}		
			
		newState = processButtons() ; 						// occupied section can be overruled by a button press
	}

	else { // if signal is locked
		newState = red ;
	}
	
	if( newState != undefined ) signal.state = newState; 	// if a new state 

	#define setLedStates(x,y,z) greenLed.state=x;yellowLed.state=y,redLed.state=z 				// set the states of the LEDS accordingly
	switch( signal.state ) {//	  G  Y  R 
	case green:		setLedStates( 1, 0, 0 ) ;	break ;
	case yellow:	setLedStates( 0, 1, 0 ) ;	break ;
	case red:  		setLedStates( 0, 0, 1 ) ;	break ;
	case driveOnSight: 
		if( !blinkT ) { 
			blinkT = 200 ;
			yellowLed.state ^= 1 ;
		} 
		break ;		// 2 second interval toggle yellow LED
	}
}


/***************** HANDLE OUTPUTS **********************/
#define clrLeds() digitalWrite(greenLed,LOW);digitalWrite(yellowLed,LOW);digitalWrite(redLed,LOW);
#define setLeds(x,y,z)  if(x)digitalWrite(greenLed,x);if(y)digitalWrite(yellowLed,y);if(z)digitalWrite(redLed,z);
void fadeLeds() {
	static uint8_t ledSelector;

	if( !fadeT ) { fadeT = 1; // every 1ms
		if( pwm == 0 ) {	// if pwm is 0, the previous led has faded off and we can pick a new one
			clrLeds();	// first we clear all colors and than set the active one.
			// the german pre signal needs more than 1 active color
			if(  greenLed.state == 1 ) ledSelector = green; 	setLeds( HIGH,  LOW,  LOW );
			if( yellowLed.state == 1 ) ledSelector = yellow;	setLeds(  LOW, HIGH,  LOW );
			if(    redLed.state == 1 ) ledSelector = red;	  	setLeds(  LOW,  LOW, HIGH );
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
	servoInterval = 20;
} ;

void controlServo() {
	static uint8_t servoState = up;

	if( !servoT ) { servoT = servoInterval ; // 20ms?

		switch( servoState ) {
		case up:
			if( signal.state == yellow 
			||	signal.state == red ) { 
				servoState = raising;
			}
			break;
			
			
		case lowering:
			if( servoPos < servoPosMin ) servoPos-- ;
			if( servoPos ==  servoPosMin ) servoState = bounceAfterLowering ;
			servoState = bounceAfterLowering;
			break;

		case bounceAfterLowering:

			if( 1 /* done */ ) servoState = down;
			break;

		case down:
			if( signal.state == green ) servoState = raising;
			break;

		case raising: 
			if( servoPos < servoPosMax ) servoPos++;
			if( servoPos ==  servoPosMax ) servoState = bounceAfterRaising ;
			break; 

		case bounceAfterRaising:
			if( 1 /* done */ ) servoState = up;
			break; 
		}
		motor.write( servoPos );
	}
}


// 20Hz  -> 50ms
// 100Hz -> 10ms
void sendSignals() {
	static uint8_t state = 0 ;

	if( !sendFreqT ) {
		sendFreqT = map( sendFreq, 20, 100, 50, 10 ) ;

		switch( signal.state ) { // note pre signals are not supposed to send signals back
			case green:			sendFreq = greenFreq;	break;
			case yellow:		sendFreq = yellowFreq;	break;
			case red:			sendFreq = redFreq;		break;
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

	// physical inputs

	readInputs() ;		// read input

	// logic
	computeLogic() ;	// compute logic, determen what the signal should do
	setOutput() ;		// set the states of the leds and servo accordingly

	// physical outputs
	sendSignals() ;		// send the signal to the adjacent module
	fadeLeds() ;		// fade leds in and out accoriding to the state
	controlServo();		// handle the arm's servo motor including mass inertia movement
}