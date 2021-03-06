#define DEBUG

#include <Arduino.h>
#include "src/modules/debounceClass.h"
#include <Servo.h>

extern void debug( String ) ;

extern Debounce detector ;			// need altering for faster debounce
extern Debounce directionSignal ;
extern Debounce redButton ;
extern Debounce yellowButton ;
extern Debounce greenButton ;
extern Debounce receiver ;

extern Servo semaphore ;

extern uint8_t servoPosMax ;
extern uint8_t servoPosMin ;

typedef struct {
	uint8_t buttons ;
	uint8_t detectorState ; 
	uint8_t sendFreq ; // N.B. frequenty is actually the cycle Time. The calculation to an actual frequenty is unneeded.
	uint8_t recvFreq ; 
	uint8_t nextState ; 
	uint8_t locked ; 
	uint8_t state ; 
	uint8_t type ;
	uint8_t section ;
	uint8_t wasLocked ; 
	uint8_t lastState ;
	uint8_t override ;
	uint8_t connected ;
	uint8_t passFromBehind ;
} Signal ;

extern Signal signal ;

typedef struct {
	uint8_t transitionedToRed ;
	uint8_t transitionedToYellow ;
	uint8_t transitionedToGreen  ;
	uint8_t state ;
} NextSignal ;

extern NextSignal nextSignal ;

typedef struct {
	uint8_t state ;
	uint8_t max ;
	uint8_t min ;
	uint8_t pwm ;
	uint8_t pin ;
} Led ;

extern Led redLed, greenLed, yellowLed ;

extern uint8_t greenButtonState ;
extern uint8_t redButtonState ;
extern uint8_t yellowButtonState ;
extern uint8_t directionSignalState ;
extern uint8_t detectorState ;

enum signalTypes {
	mainSignal, 		// xx00
	combiSignal,		// xx01
	germanPreSignal,	// xx10
	dutchPreSignal,		// xx11
	/*passFromBehind*/		// x1xx
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

const uint8_t greenFreq = 10 ;
const uint8_t yellowFreq = 20 ;
const uint8_t redFreq = 30 ;

extern uint8_t mode ;
extern uint8_t previousState ;
extern uint8_t rxFrequency ;
extern uint8_t servoPos ;
extern uint8_t redServoPos ;
extern uint8_t greenServoPos ;
