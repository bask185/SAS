// HEADER FILES
#include <Arduino.h>
#include "semaphoreControl.h"
//#include "serial.h"
#include "src/basics/timers.h"
#include "src/basics/io.h"
#include "config.h"
//#include <Servo.h>

// MACROS
#define stateFunction(x) static bool x##F(void)
#define entryState if(runOnce) 
#define onState runOnce = false; if(!runOnce)
#define exitState if(!exitFlag) return false; else
#define State(x) break; case x: if(runOnce) Serial.println(#x); if(x##F())
#define STATE_MACHINE_BEGIN semaphore.write( servoPos ) ;if(!enabled) { \
	if(!semaphoreControlT) enabled = true; } \
else switch(state){\
	default: Serial.println("unknown state executed, state is idle now"); state = semaphoreControlIDLE; case semaphoreControlIDLE: return true;
#define STATE_MACHINE_END break;}return false;


#define beginState raising
#ifndef beginState
#error beginState not yet defined
#endif

// VARIABLES
static unsigned char state = beginState ;
static bool enabled = true, runOnce = true, exitFlag = false;


// FUNCTIONS
extern void semaphoreControlInit(void) { state = beginState;servoPosMin=45;servoPosMax=135; }
extern byte semaphoreControlGetState(void) { return state;}
extern void semaphoreControlSetState(unsigned char _state) { state = _state; runOnce = true; }
static void nextState(unsigned char _state, unsigned char _interval) {
	runOnce = true;
	exitFlag = false;
	if(_interval) {
		enabled = false;
		semaphoreControlT = _interval; } 
	state = _state; }


const int massInertiaSteps = 30 ;

// STATE FUNCTIONS
stateFunction(up) {
	entryState {
		semaphore.detach( );
	}
	onState {
		if( signal.state == yellow || signal.state == red ) exitFlag = true; 
	}
	exitState {
		semaphore.attach( servoPin );
		return true;
	}
}

stateFunction(lowering) {
	entryState {
		
	}
	onState {
		if( servoPos > servoPosMin ) servoPos -- ;
		else exitFlag = true;
	}
	exitState {

		return true;
	}
}

stateFunction(bounceAfterLowering) {
	static uint8_t steps ;
	
	entryState {
		steps = 0 ;
	}
	onState {
				
		if( steps < (massInertiaSteps / 2) )	servoPos -- ;
		else									servoPos ++ ;
		
		steps ++ ;
		if( steps >= massInertiaSteps )	exitFlag = true; 
	}
	exitState {

		return true;
	}
}

stateFunction(down) {
	entryState {
		semaphore.detach( );
	}
	onState {
		if( signal.state == green )  exitFlag = true;
	}
	exitState {
		semaphore.attach( servoPin );
		return true;
	}
}

stateFunction(raising) {
	entryState {
		
	}
	onState {
		if( servoPos <  servoPosMax )	servoPos  ++ ;
		else exitFlag = true;
	}
	exitState {

		return true;
	}
}

stateFunction(bounceAfterRaising) {
	static uint8_t steps ;
	
	entryState {
		steps = 0 ;
	}
	onState {
		if( steps < massInertiaSteps / 2 )	servoPos ++ ;
		else								servoPos -- ;

		steps ++ ;
		if( steps >= massInertiaSteps )	exitFlag = true;
	}
	exitState {

		return true;
	}
}

// STATE MACHINE
extern bool semaphoreControl(void) {
	STATE_MACHINE_BEGIN

	State(up) {
		nextState(lowering, 0); }

	State(lowering) {
		nextState(bounceAfterLowering, 0); }

	State(bounceAfterLowering) {
		nextState(down, 0); }

	State(down) {
		nextState(raising, 0); }

	State(raising) {
		nextState(bounceAfterRaising, 0); }

	State(bounceAfterRaising) {
		nextState(up, 0); }

	STATE_MACHINE_END
}
