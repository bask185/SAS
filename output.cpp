#include "config.h"
#include "output.h"
#include "src/basics/io.h"
#include "src/basics/timers.h"
#include "src/modules/debounceClass.h"


/***************** HANDLE OUTPUTS **********************/
#define clrLeds() digitalWrite(greenLed,LOW);digitalWrite(yellowLed,LOW);digitalWrite(redLed,LOW);
#define setLeds(x,y,z)  if(x)digitalWrite(greenLed,x);if(y)digitalWrite(yellowLed,y);if(z)digitalWrite(redLed,z);

const int pwmMin = 0;
const int pwmMax = 255;
uint8_t pwm = 0;
       


// 20Hz  -> 50ms
// 100Hz -> 10ms
void sendSignals( ) {

    static uint8_t state = 0, counter = 0;

    if( !sendFreqT ) {
        sendFreqT =  signal.sendFreq ; // green = 10, yellow = 20, red  = 30

        switch( signal.state ) { // note pre signals are not supposed to send signals back?
            case green:          signal.sendFreq = greenFreq ;    break ;
            case yellow:         signal.sendFreq = yellowFreq ;   break ;
            case red:            signal.sendFreq = redFreq ;      break ;
            case driveOnSight:   signal.sendFreq = redFreq ;   break ; // in the event of driving on sight, signal yellow to previous staet
        }
        
        state ^= 1 ;
        if( state ) {
            pinMode( Tx, OUTPUT ) ;            // pull signal down
            digitalWrite( Tx, LOW ) ;
        }
        else {
            pinMode( Tx, INPUT_PULLUP ) ;    // pull signal up
        }
    }
}

void controlBrakeModule( ) {    // it may be that for analog trains something specials is needed

    if( signal.locked == 1 && signal.passFromBehind == 1 ) {      // If direction is opposite and signal may be bypassed from behind
        digitalWrite( relayPin, LOW ) ;
        digitalWrite( slowSpeed, LOW );
    }
    else if( signal. state == red ) {
        digitalWrite( relayPin, HIGH ) ;
        digitalWrite( slowSpeed, LOW ) ;
    }
    else if( signal.state == yellow ) {    // yellow signal must also set slow speed signal. This may be depended on module type.
        digitalWrite( relayPin, LOW ) ;
        digitalWrite( slowSpeed, HIGH );
    } 
    else {
        digitalWrite( slowSpeed, LOW );
        digitalWrite( relayPin, LOW ) ;
    }
}



void fadeLeds() {
    if(   greenLed.state == 1 ) /*&& (  greenLed.pwm <  greenLed.max )* / ) {  greenLed.pwm ++ ;*/digitalWrite( greenLedPin, HIGH ) ;// }
    if(  yellowLed.state == 1 ) /*&& ( yellowLed.pwm < yellowLed.max )* / ) { yellowLed.pwm ++ ;*/digitalWrite( yellowLedPin, HIGH ) ;// }
    if(     redLed.state == 1 ) /*&& (    redLed.pwm <    redLed.max )* / ) {    redLed.pwm ++ ;*/digitalWrite( redLedPin, HIGH ) ;// }

    if(   greenLed.state == 0 ) /*&& (  greenLed.pwm >  greenLed.min )* / ) {  greenLed.pwm -- ;*/digitalWrite( greenLedPin, LOW ) ;// }
    if(  yellowLed.state == 0 ) /*&& ( yellowLed.pwm > yellowLed.min )* / ) { yellowLed.pwm -- ;*/digitalWrite( yellowLedPin, LOW ) ;// }
    if(     redLed.state == 0 ) /*&& (    redLed.pwm >    redLed.min )* / ) {    redLed.pwm -- ;*/digitalWrite( redLedPin, LOW ) ;// }

    //Serial.print(greenLed.pwm);Serial.print(" "); Serial.print(yellowLed.pwm);Serial.print(" ");Serial.println(redLed.pwm);
}



// ISR(TIMER0_COMPA_vect) { // timer 1 ISR must run at 9kHz
//     static byte dutyCycle = 0;
//     static uint8_t localRed, localGreen, localYellow;

//     if( dutyCycle == 0 ) {
//         localRed = redLed.pwm ;
//         localYellow = yellowLed.pwm ;
//         localGreen = greenLed.pwm ;

//         if( localRed    > 0 ) PORTB |= (1<<2) ; // change to port instructions when finished
//         if( localYellow > 0 ) PORTB |= (1<<3) ;
//         if( localGreen  > 0 ) PORTB |= (1<<4) ;
//     }


//     if( dutyCycle ==    localRed )  PORTB &= ~(1<<2) ;
//     if( dutyCycle == localYellow )  PORTB &= ~(1<<3) ;
//     if( dutyCycle ==  greenLed.pwm )  PORTB &= ~(1<<4) ;
   

//     dutyCycle ++ ;
// }

extern void initTimer0() {
    TCCR0A = 0;// set entire TCCR1A register to 0
    TCCR0B = 0;// same for TCCR1B
    TCNT0  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR0A = 60;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR0B |= (1 << WGM02);
    // Set CS10 and CS12 bits for 1024 prescaler
    TCCR0B |= (1 << CS01) ;
    // enable timer compare interrupt
    TIMSK0 |= (1 << OCIE0A);

}