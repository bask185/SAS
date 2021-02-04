// HEADER FILES
#include <Arduino.h>
#include "teachIn.h"
#include "config.h"
#include "src/basics/timers.h"
#include <EEPROM.h>


enum EEaddresses {
	PWM_GREEN_ADDR = 0x00,
	PWM_YELLOW_ADDR,
	PWM_RED_ADDR,
	GREEN_SERVO_POS,
	RED_SERVO_POS,
	INIT_ADDR,
}; 


// MACROS
#define stateFunction(x) static bool x##F(void)
#define entryState if(runOnce) 
#define onState runOnce = false; if(!runOnce)
#define exitState if(!exitFlag) return false; else
#define State(x) break; case x: /*if(runOnce) Serial.println(#x) ;*/ if(x##F())
#define STATE_MACHINE_BEGIN if(!enabled) { \
	if(!teachInT) enabled = true; } \
else switch(state){\
	default: /*Serial.println("unknown state executed, state is idle now") ;*/ state = teachInIDLE; case teachInIDLE: return true;
#define STATE_MACHINE_END break;}return false;


#define beginState waitButtonPress
#ifndef beginState
#error beginState not yet defined
#endif

// VARIABLES
static unsigned char state = beginState;
static bool enabled = true, runOnce = true, exitFlag = false;

// FUNCTIONS
extern void teachInInit(void) { 

	state = beginState; 

	servoPos = 90 ;

	uint8_t firstEntry = EEPROM.read( INIT_ADDR ) ;
	if( firstEntry != 0xCC ) { // is signal is already started once, retreive values
		EEPROM.write( GREEN_SERVO_POS, 45 ) ;
		EEPROM.write( RED_SERVO_POS, 135 ) ;
		EEPROM.write( PWM_GREEN_ADDR, 100 ) ;
 		EEPROM.write( PWM_YELLOW_ADDR, 100 ) ;
		EEPROM.write( PWM_RED_ADDR, 100) ;

		EEPROM.write( INIT_ADDR, 0xCC) ;
		//Serial.println("FIRST TIME BOOTING, DEFAULT SETTINGS ARE LOADED") ;
	}

	greenServoPos =	EEPROM.read( GREEN_SERVO_POS ) ;
	redServoPos   =	EEPROM.read( RED_SERVO_POS ) ;
	greenLed.max  =	EEPROM.read( PWM_GREEN_ADDR ) ;
	yellowLed.max =	EEPROM.read( PWM_YELLOW_ADDR ) ;
	redLed.max    =	EEPROM.read( PWM_RED_ADDR ) ;

	signal.type = 
	  ( digitalRead( dip0 ) << 3 )
	| ( digitalRead( dip1 ) << 2 )
	| ( digitalRead( dip2 ) << 1 )
	| ( digitalRead( dip3 ) << 0 ) ;

	// switch( signal.type ) {
	// 	default:				//Serial.println("unknown type signal selected, correctly set the dipswitches nad reboot") ; break;
	// 	case dutchPreSignal:	//Serial.println("dutchPreSignal selected") ; 	break;
	// 	case germanPreSignal:	//Serial.println("germanPreSignal selected") ; 	break;
	// 	case mainSignal:		//Serial.println("mainSignal selected") ; 		break;
	// 	case combiSignal:		//Serial.println("combiSignal selected") ; 		break;
	// }
}

extern byte teachInGetState(void) { return state;}
extern void teachInSetState(unsigned char _state) { state = _state; runOnce = true; }
static void nextState(unsigned char _state, unsigned char _interval) {
	runOnce = true;
	exitFlag = false;
	if(_interval) {
		enabled = false;
		teachInT = _interval; } 
	state = _state; }

// STATE FUNCTIONS

stateFunction( waitButtonPress ) { // just wait on the first button press
	entryState {
		
	}
	onState {
		if( analogRead( potPin ) ) exitFlag = true ; // if value is zero, it means the button of the config thingy is pressed
	}
	exitState {

		return true;
	}
}

stateFunction(adjustGreenBrightness) {
	uint16_t val ;

	entryState {
		greenLed.pwm = 255 ;
		teachInT = 250 ; // 2,5 second for on time led

		timeOutT = 250 ; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 255 ) ;
				greenLed.pwm = val ;
			}
		}
	}
	exitState {
		greenLed.pwm = 0 ;
		EEPROM.write( PWM_GREEN_ADDR, val ) ;
		return true;
	}
}
stateFunction(adjustYellowBrightness) {
	uint16_t val ;

	entryState {
		yellowLed.pwm = 255 ;
		teachInT = 250 ; // 2,5 second for on time led

		timeOutT = 250 ; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 255 ) ;
				yellowLed.pwm = val ;
			}
		}
	}
	exitState {
		yellowLed.pwm = 0 ;
		EEPROM.write( PWM_YELLOW_ADDR, val ) ;
		return true;
	}
}

stateFunction(adjustRedBrightness) {
	uint16_t val ;

	entryState {
		redLed.pwm = 255 ;
		teachInT = 250 ; // 2,5 second for on time led

		timeOutT = 250 ; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 255 ) ;
				redLed.pwm = val ;
			}
		}
	}
	exitState {
		redLed.pwm = 0 ;
		EEPROM.write( PWM_RED_ADDR, val ) ;
		return true;
	}
}

stateFunction(setServoRed) {
	uint16_t val;

	entryState { 
		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 180) ;
				servoPos = val ; ;
			} 
		}
	}
	exitState {
		EEPROM.write( RED_SERVO_POS, val ) ;
		return true;
	}
}

stateFunction(setServoGreen) {
	uint16_t val;

	entryState {
		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potPin ) ;

			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 180) ;
				servoPos = val ; ;
			} 
		}
	}
	exitState {
		EEPROM.write( GREEN_SERVO_POS, val ) ;
		return true;
	}
}

// STATE MACHINE
extern bool teachIn(void) {
	STATE_MACHINE_BEGIN

	State(waitButtonPress) {
		nextState(adjustGreenBrightness, 100) ; }

	State(adjustGreenBrightness) {
		if( !timeOutT ) 					 nextState(waitButtonPress, 100) ;
		else if( signal.type == mainSignal ) nextState(adjustRedBrightness, 100) ;
		else								 nextState(adjustYellowBrightness, 100) ; }

	State(adjustYellowBrightness) {
		if( !timeOutT ) nextState(waitButtonPress, 100) ;
		else 			nextState(adjustRedBrightness, 100) ; }

	State(adjustRedBrightness) {
		if( !timeOutT ) nextState(waitButtonPress, 100) ;
		else			nextState(setServoRed, 100) ; }

	State(setServoRed) {
		if( !timeOutT ) nextState(waitButtonPress, 100) ;
		else			nextState(setServoGreen, 100) ; }

	State(setServoGreen) {
		nextState(waitButtonPress, 100) ; }

	STATE_MACHINE_END
}
