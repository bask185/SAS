
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

enum signalStates {
	undefined,
	redSignal,
	yellowSignal,
	greenSignal,
	driveOnSight,
	expectGreen, // pre signal states
	expectYellow,
	expectRed
} ;

enum trackStates {
	occupied,
	available,
} ;

const uint8_t greenFreq = 10;
const uint8_t yellowFreq = 30;
const uint8_t redFreq = 50;

uint8_t signalState = greenSignal ;
uint8_t buttonState ;
uint8_t sendFreq  ;
uint8_t recvFreq ;
uint8_t override ;
uint8_t trackState ;
uint8_t lockState ;
uint8_t nextSignalState ;
uint8_t nextTrackState ;
uint8_t detectorState ;
uint8_t newSignalState ;

uint8_t readButtonStates()
{
	return 1 ;
}

void readInputs() {
	static uint8_t nextSignalStatePrev;

	if( !debounceT ) {
		debounceT = 200 ; // 200ms debounce time

		int val = analogRead( inputButtons ) ; 
			 if( val <  250 ) buttonState = redSignal ;
		else if( val <  600 ) buttonState = yellowSignal ;
		else if( val < 1000 ) buttonState = greenSignal ;


		// debounce inputs
		lockSignal.debounceInputs() ;
		detector.debounceInputs() ;
		//nextSignalState.debounceInputs() ;

		// read in input states
		lockState = lockSignal.readInput()  ;

		if( signalType == entrySignal ) { // in the event of an entry signal, the lockpin must be inverted
			 	 if( lockState ==  ON ) lockState == OFF; // 
			else if( lockState == OFF ) lockState ==  ON;
		}
		
		detectorState = detector.readInput()  ;
		//nextSignalState = nextSignal.readInput()  ;
		buttonState = readButtonStates() ; // yellow, red or green button control buttons
	}

	if( recvFreq >  greenFreq - 5 && recvFreq <  greenFreq + 5 ) { nextSignalState =  greenSignal ; }
	if( recvFreq > yellowFreq - 5 && recvFreq < yellowFreq + 5 ) { nextSignalState = yellowSignal ; }
	if( recvFreq >    redFreq - 5 && recvFreq <    redFreq + 5 ) { nextSignalState =    redSignal ; }
 
	if( nextSignalState != nextSignalStatePrev ) { // if new signal has changed, adopt it for 1 cycle
		nextSignalStatePrev = nextSignalState ;
	} 
	else {
		nextSignalState = undefined ;				// no new signal received
		nextSignalStatePrev = undefined ;
	}
}


void logic() {
	switch( signalType ) {
	
	/* DUTCH PRE SIGNAL KNOWS ONLY EXPECTING GREEN OR RED */
	case dutchPreSignal: 
		switch( nextSignalState ) {
		case greenSignal:
		case yellowSignal:
			newSignalState = expectGreen ;
			break;
		case redSignal:
			newSignalState = expectRed ;
			break;
		}
		break;

	/* GERMAN PRE SIGNAL KNOWS ONLY EXPECTING GREEN, YELLOW OR RED */
	case germanPreSignal:
		switch( nextSignalState ) {
		case greenSignal:
			newSignalState = expectGreen ;
			break ;
		case yellowSignal:
			newSignalState = expectYellow ;
			break ;
		case redSignal:
			newSignalState = expectRed ;
			break ;
		}
		break;

	case mainSignal:
		if( nextSignalState == redSignal ) newSignalState = greenSignal ;
		break;

	case combiSignal:
		switch( nextSignalState ) {
		case greenSignal:
		case yellowSignal:
			newSignalState = redSignal ;
			break ;
		case redSignal:
			newSignalState = yellowSignal ;
			break;
		}
		break;
	}
	

	if( detectorState == OFF ) trackState = occupied ;			// if our sensor is made, our track is occupied



	if( lockState == OFF || trackState == occupied ) { 				// pulling down lock signal overrides signal state to red,	#1 priority
		signalState = redSignal ; return ; 							// also occupied track means red signal
	} 

	if( buttonState == redSignal || buttonState == yellowSignal ) { // if yellow or red button is pressed, 						#2 priority
		signalState = buttonState ; 
		override = true  ;											//  set override flag	
		return ;
	}
	else if( buttonState == greenSignal ) {							// green button pressed										#3 priority
		signalState = buttonState ; 									// set signal green
		override = false  ;											// release override
	}
}

void setServo(uint8_t servoPos)
{
}



void setOutputs() {
	// first set the signals's arm and or LED's according to the current signalState
	#define setSignal(x,y,z,q,w) digitalWrite(greenLed,x);digitalWrite(yellowLed,y);digitalWrite(redLed,z);analogWrite( pwmPin,q);setServo(w)
	switch( signalState ) {       // GREEN, YELLOW, RED, BRIGHTNESS, SERVO POS
		case greenSignal:	setSignal( HIGH,  LOW,  LOW, greenPwm,  greenServoPos ) ;	break ;
		case yellowSignal:	setSignal(  LOW, HIGH,  LOW, yellowPwm, redServoPos   ) ;	break ;
		case redSignal:  	setSignal(  LOW,  LOW, HIGH, redPwm,    redServoPos   ) ;	break ;
	}

	switch( signalType ) {		// handle the signal to the previous signal module.
	case mainSignal: 
		//if( detectorState == 
		break ;
	}



	if( signalState == redSignal ) {
		pinMode( previousSignal, OUTPUT ) ;		// if signal is red, relay this to the previous signal
		digitalWrite( previousSignal, LOW ) ;
	}
	else {
		pinMode( previousSignal, INPUT_PULLUP ) ;// if signal is orange or green, don't signal red anymore
	}
}
// 20Hz  -> 50ms
// 100Hz -> 10ms
void sendSignal() {
	static uint8_t state = 0 ;

	if( !sendFreqT ) {
		sendFreqT = map( sendFreq, 20, 100, 50, 10 ) ;

		if( state ) {
			state = 0 ;
			pinMode( nextSignal, OUTPUT ) ;  // pull signal down
			digitalWrite( nextSignal, LOW ) ;
		}
		else {
			state = 1 ;
			digitalWrite( nextSignal, HIGH ) ;
			pinMode( nextSignal, INPUT_PULLUP ) ; 
		}
	}
}

void readIncFreq() {
	static uint8_t recvFreqPrev = 0;
	uint8_t time = recvFreqPrev - recvFreqT ;

	recvFreq = map( time, 50, 10, 20, 100 ); 

	recvFreqPrev = recvFreqT ;
	recvFreqT = 255 ;
}





extern void processRoundRobinTasks(void) {
	static unsigned char taskCounter = 255 - 1 ;

// HIGH PRIORITY ROUND ROBIN TASKS
	sendSignal() ;

// LOW PRIORITY ROUND ROBIN TASKS
	taskCounter ++ ;
	switch(taskCounter) {
	default: taskCounter = 0 ;

	case 0:
		if( !debounceT ) {
			debounceT = 20 ;
			/* analog reading of input buttons 
			0V = red button		  = 0 ADC
			1.67V = yellow button = 343 ADC
			3.33V = green button = 682 ADC
			5V is no button pressed = 1023 ADC
			*/
			int val = analogRead( inputButtons ) ; 
				 if( val <  250 ) buttonState = redSignal ;
			else if( val <  600 ) buttonState = yellowSignal ;
			else if( val < 1000 ) buttonState = greenSignal ;
			else				  buttonState = undefined ;
		}
		break ;

	case 1:
		if( teachInGetState() == waitButtonPress ) { // when the signal is not being configurated, handle IO
			readInputs() ;
			logic() ;
			setOutputs() ;
		}
		break ;


	case 255:
		attachInterrupt(digitalPinToInterrupt( interruptPin ), readIncFreq, FALLING);
		break;

	}
}