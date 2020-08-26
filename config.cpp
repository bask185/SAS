#include "config.h"

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

Signal signal ;
NextSignal nextSignal ;

const uint8_t greenFreq = 10 ;
const uint8_t yellowFreq = 20 ;
const uint8_t redFreq = 30 ;