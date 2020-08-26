// HEADER FILES
#include <Arduino.h>
#include "teachIn.h"
#include "serial.h"
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

uint8_t greenPwm, yellowPwm, redPwm, redServoPos, greenServoPos, signalType;

Servo armServo;
Debounce detector( detectorPin );
Debounce lockSignal( lockPin );

// MACROS
#define stateFunction(x) static bool x##F(void)
#define entryState if(runOnce) 
#define onState runOnce = false; if(!runOnce)
#define exitState if(!exitFlag) return false; else
#define State(x) break; case x: if(runOnce) Serial.println(#x); if(x##F())
#define STATE_MACHINE_BEGIN if(!enabled) { \
	if(!teachInT) enabled = true; } \
else switch(state){\
	default: Serial.println("unknown state executed, state is idle now"); state = teachInIDLE; case teachInIDLE: return true;
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

	armServo.write( 90 );
	armServo.attach( servoPin ); 

	uint8_t firstEntry = EEPROM.read( INIT_ADDR );
	if( firstEntry != 0xCC ) { // is signal is already started once, retreive values
		EEPROM.write( GREEN_SERVO_POS, 45 );
		EEPROM.write( RED_SERVO_POS, 135 );
		EEPROM.write( PWM_GREEN_ADDR, 255 );
 		EEPROM.write( PWM_YELLOW_ADDR, 255 );
		EEPROM.write( PWM_RED_ADDR, 255);

		EEPROM.write( INIT_ADDR, 0xCC);
		Serial.println("FIRST TIME BOOTING, DEFAULT SETTINGS ARE LOADED");
	}

	greenServoPos = EEPROM.read( GREEN_SERVO_POS ) ;
	redServoPos = 	EEPROM.read( RED_SERVO_POS ) ;
	greenPwm = 		EEPROM.read( PWM_GREEN_ADDR ) ;
	yellowPwm = 	EEPROM.read( PWM_YELLOW_ADDR ) ;
	redPwm = 		EEPROM.read( PWM_RED_ADDR ) ;

	signalType = 
	  ( digitalRead( dip0 ) << 3 )
	| ( digitalRead( dip1 ) << 2 )
	| ( digitalRead( dip2 ) << 1 )
	| ( digitalRead( dip3 ) << 0 ) ;

	switch( signalType ) {
		default: 				Serial.println("unknown type signal selected"); break;
		case dutchPreSignal: 	Serial.println("dutchPreSignal selected"); 		break;
		case germanPreSignal: 	Serial.println("germanPreSignal selected"); 	break;
		case mainSignal:		Serial.println("mainSignal selected"); 			break;
		case combiSignal: 		Serial.println("combiSignal selected"); 		break;
	}
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
		byte buttonState = 1;//configButton.readInput();
		if( buttonState == RISING ) exitFlag = true;
	}
	exitState {

		return true;
	}
}

stateFunction(adjustGreenBrightness) {
	uint16_t val;

	entryState {
		digitalWrite( greenLed, HIGH );
		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potentiometer );
			val = map( val, 0, 1023, 0, 255);
			analogWrite( pwmPin, val );
		}

		byte buttonState = 1;//configButton.readInput();
		if( buttonState == RISING || timeOutT == 0 ) exitFlag = true;
	}
	exitState {
		digitalWrite( greenLed, LOW );
		greenPwm = val;
		EEPROM.write( PWM_GREEN_ADDR, val );
		return true;
	}
}

stateFunction(adjustYellowBrightness) {
	uint16_t val;

	entryState {
		digitalWrite( yellowLed, HIGH );

		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potentiometer );
			val = map( val, 0, 1023, 0, 255);
			analogWrite( pwmPin, val );
		}

		byte buttonState = 1;//configButton.readInput();
		if( buttonState == RISING || timeOutT == 0 ) exitFlag = true;
	}
	exitState {
		digitalWrite( yellowLed, LOW );
		yellowPwm = val;
		EEPROM.write( PWM_YELLOW_ADDR, val );
		return true;
	}
}

stateFunction(adjustRedBrightness) {
	uint16_t val;

	entryState {
		digitalWrite( redLed, HIGH );

		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potentiometer );
			val = map( val, 0, 1023, 0, 255);
			analogWrite( pwmPin, val );
		}

		byte buttonState = 1;//configButton.readInput();
		if( buttonState == RISING || timeOutT == 0 ) exitFlag = true;
	}
	exitState {
		digitalWrite( redLed, LOW );
		redPwm = val;
		EEPROM.write( PWM_RED_ADDR, val );
		return true;
	}
}

stateFunction(setServoRed) {
	uint16_t val;

	entryState { 
		digitalWrite( redLed, HIGH ); // turn on red LED to indicate that the servo position is for red signal
		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potentiometer );
			val = map( val, 0, 1023, 0, 180);
			armServo.write( val ); 
		}

		byte buttonState = 1;//configButton.readInput();
		if( buttonState == RISING || timeOutT == 0 ) exitFlag = true;
	}
	exitState {
		digitalWrite( redLed, LOW );
		EEPROM.write( RED_SERVO_POS, val );
		return true;
	}
}

stateFunction(setServoGreen) {
	uint16_t val;

	entryState {
		digitalWrite( greenLed, HIGH ); // turn on green LED to indicate that the servo position is for green signal
		timeOutT = 250; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( potentiometer );
			val = map( val, 0, 1023, 0, 180);
			armServo.write( val ); 
		}

		byte buttonState = 1;//configButton.readInput();
		if( buttonState == RISING || timeOutT == 0 ) exitFlag = true;
	}
	exitState {
		digitalWrite( greenLed, LOW );
		EEPROM.write( GREEN_SERVO_POS, val );
		return true;
	}
}

// STATE MACHINE
extern bool teachIn(void) {
	STATE_MACHINE_BEGIN

	State(waitButtonPress) {
		nextState(adjustGreenBrightness, 0); }

	State(adjustGreenBrightness) {
		if( !timeOutT ) 					nextState(waitButtonPress, 0);
		else if( signalType == mainSignal ) nextState(adjustRedBrightness, 0);
		else								nextState(adjustYellowBrightness, 0); }

	State(adjustYellowBrightness) {
		if( !timeOutT ) nextState(waitButtonPress, 0);
		else 			nextState(adjustRedBrightness, 0); }

	State(adjustRedBrightness) {
		if( !timeOutT ) nextState(waitButtonPress, 0);
		else			nextState(setServoRed, 0); }

	State(setServoRed) {
		if( !timeOutT ) nextState(waitButtonPress, 0);
		else			nextState(setServoGreen, 0); }

	State(setServoGreen) {
		nextState(waitButtonPress, 0); }

	STATE_MACHINE_END
}
