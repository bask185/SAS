#include <Arduino.h>


#define ON 9
#define OFF 10


#ifndef button_h
#define	button_h

//#define 

class Debounce {
public:
	Debounce(unsigned char _pin);
	// bool fell() ;
	// bool rose() ;
	// bool isHigh() ;
	// bool isLow() ;
	unsigned char getState() ;
	void debounce() ;

private:
	unsigned char state ;
	unsigned char pin ; 
	bool oldSample = false ; 
	bool statePrev = false ;
};
	
#endif