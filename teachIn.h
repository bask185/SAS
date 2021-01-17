#include "src/basics/io.h"

enum teachInStates {
	teachInIDLE,
	adjustGreenBrightness,
	adjustYellowBrightness,
	adjustRedBrightness,
	setServoRed,
	setServoGreen,
	waitButtonPress
} ;



extern bool teachIn(void) ; 
extern void teachInInit() ;
extern void teachInSetState(unsigned char) ;
extern unsigned char teachInGetState(void) ;
