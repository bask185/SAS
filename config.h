#include <Arduino.h>
#include "src/modules/debounceClass.h"


extern Debounce detector( ) ;
extern Debounce lockSignal( ) ;
extern Debounce redButton( ) ;
extern Debounce yellowButton( ) ;
extern Debounce greenButton( ) ;

typedef struct {
	uint8_t buttons ;
	uint8_t detectorState ; 
	uint8_t sendFreq ; // N.B. frequenty is actually the cycle Time. The calculation to an actual frequenty is unneeded.
	uint8_t recvFreq ; 
	uint8_t nextState ; 
	uint8_t locked ; 
	uint8_t redLedState : 1 ; 
	uint8_t yellowLedState : 1 ; 
	uint8_t greenLedState : 1 ; 
	uint8_t state ; 
	uint8_t type ;
	uint8_t section ;
	uint8_t wasLocked ; 
	uint8_t lastState ;
	uint8_t override ;
	uint8_t connected ;
} Signal ;

extern Signal signal ;

typedef struct {
	uint8_t transitionedToRed ;
	uint8_t transitionedToYellow ;
	uint8_t transitionedToGreen  ;
	uint8_t state ;
} NextSignal ;

extern NextSignal nextSignal ;

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

const uint8_t greenFreq = 10 ;
const uint8_t yellowFreq = 20 ;
const uint8_t redFreq = 30 ;

extern uint8_t mode ;
extern uint8_t previousState ;
