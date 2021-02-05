// HEADER FILES
#include <Arduino.h>
#include "teachIn.h"
#include "config.h"
#include "src/basics/timers.h"
#include <EEPROM.h>
#include "src/modules/SoftPWM.h"


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
#define State(x) break; case x: if(runOnce) Serial.println(#x) ; if(x##F())
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

	uint8_t firstEntry = EEPROM.read( INIT_ADDR ) ;
	if( firstEntry != 0xCC ) { // is signal is already started once, retreive values
		EEPROM.write( GREEN_SERVO_POS, 45 ) ;
		EEPROM.write( RED_SERVO_POS, 135 ) ;
		EEPROM.write( PWM_GREEN_ADDR, 255 ) ;
 		EEPROM.write( PWM_YELLOW_ADDR, 255 ) ;
		EEPROM.write( PWM_RED_ADDR, 255) ;

		EEPROM.write( INIT_ADDR, 0xCC) ;
		Serial.println("FIRST TIME BOOTING SAS SOFTWARE V1.O, DEFAULT SETTINGS ARE LOADED") ;
	}

	servoPosMin =	EEPROM.read( GREEN_SERVO_POS ) ; Serial.println(servoPosMin);
	servoPosMax   =	EEPROM.read( RED_SERVO_POS )   ; Serial.println(servoPosMax);
	greenLed.pwm  =	EEPROM.read( PWM_GREEN_ADDR ) ;  Serial.println(greenLed.pwm );
	yellowLed.pwm =	EEPROM.read( PWM_YELLOW_ADDR ) ; Serial.println(yellowLed.pwm);
	redLed.pwm    =	EEPROM.read( PWM_RED_ADDR ) ;    Serial.println(redLed.pwm);


#define printType(x) case x: Serial.println(#x);break;
	signal.type = ( digitalRead( dip2 ) << 1 ) | ( digitalRead( dip1 ) ) ;	// determen which signal type
	switch( signal.type ) {					//	1	2	3	4
		printType( mainSignal ) ;			//	ON	ON	X	X
		printType( combiSignal ) ;			//	OFF	ON	X	X
		printType( dutchPreSignal ) ;		//	OFF	OFF	X	X
		printType( germanPreSignal ) ;		//	ON	OFF	X	X
	}

	signal.passFromBehind = !digitalRead( dip4 );							// if a signal is locked, it may be passed from behind
	if( signal.passFromBehind ) Serial.println("passing from behind allowed") ;
	//	DIP 4 IS ON => PASSING FROM BEHIND ALLOWED
	
	// dip 4 has no purpose yet
	

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
		if(!teachInT) { teachInT = 20 ;
			uint16_t sample = analogRead( configPin ) ;
			if( sample == 0 ) exitFlag = true ; // if value is zero, it means the button of the config thingy is pressed
		}
	}
	exitState {

		return true;
	}
}

stateFunction(adjustGreenBrightness) {
	uint16_t val ;

	entryState {
		SoftPWMSet( greenLedPin, 255 ) ;
		SoftPWMSet( yellowLedPin, 0 ) ;
		SoftPWMSet( redLedPin, 0 ) ;
		teachInT = 250 ; // 2,5 second for on time led
		timeOutT = 250 ; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( configPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true; // if timeout or button press occurs, exit
			else {
				val = map( val, 0, 1023, 0, 255 ) ;
				greenLed.pwm = val ;
			}
			SoftPWMSet( greenLedPin, greenLed.pwm ) ;
		}
	}
	exitState {
		EEPROM.write( PWM_GREEN_ADDR, greenLed.pwm ) ;
		SoftPWMSet( greenLedPin, 0 ) ;
		return true;
	}
}
stateFunction(adjustYellowBrightness) {
	uint16_t val ;

	entryState {
		SoftPWMSet( yellowLedPin, 255 ) ;
		teachInT = 250 ; // 2,5 second for on time led
		timeOutT = 250 ; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( configPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 255 ) ;
				yellowLed.pwm = val ;
			}
			SoftPWMSet(yellowLedPin, yellowLed.pwm);
		}
	}
	exitState {
		SoftPWMSet(yellowLedPin, 0);
		EEPROM.write( PWM_YELLOW_ADDR, yellowLed.pwm  ) ;
		return true;
	}
}

stateFunction(adjustRedBrightness) {
	uint16_t val ;

	entryState {
		SoftPWMSet( redLedPin, 255 ) ;
		teachInT = 250 ; // 2,5 second for on time led

		timeOutT = 250 ; // 25 seconds timeout should suffice
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( configPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 255 ) ;
				redLed.pwm = val ;
			}
			SoftPWMSet( redLedPin, redLed.pwm ) ;
		}
	}
	exitState {
		SoftPWMSet( redLedPin, 0 ) ;
		EEPROM.write( PWM_RED_ADDR, redLed.pwm ) ;
		return true;
	}
}

stateFunction(setServoRed) {
	uint16_t val;

	entryState { 
		timeOutT = 250; // 25 seconds timeout should suffice
		semaphore.attach( servoPin );
		digitalWrite( redLedPin, HIGH ) ;
		semaphore.write(90);
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( configPin ) ;
			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 180) ;
				servoPosMax = val ;
				semaphore.write( servoPosMax ) ;
			} 
		}
	}
	exitState {
		EEPROM.write( RED_SERVO_POS, servoPosMax ) ;
		semaphore.detach();
		digitalWrite( redLedPin, LOW ) ;
		return true;
	}
}

stateFunction(setServoGreen) {
	uint16_t val;

	entryState {
		timeOutT = 250; // 25 seconds timeout should suffice
		semaphore.attach(servoPin);
		digitalWrite( greenLedPin, HIGH ) ;
	}
	onState {
		if( !teachInT ) { teachInT = 10; // 10 updates per second should suffice
			val = analogRead( configPin ) ;

			if( val < 10 || timeOutT == 0 ) exitFlag = true;
			else {
				val = map( val, 0, 1023, 0, 180) ;
				servoPosMin = val ;
				semaphore.write( servoPosMin ) ;
			} 
		}
	}
	exitState {
		EEPROM.write( GREEN_SERVO_POS, servoPosMin ) ;
		semaphore.detach();
		digitalWrite( greenLedPin, LOW ) ;
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
