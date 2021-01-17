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
       

// minature state-machine to controll the servo movement including mass innertia
enum servoStates {
    bounceAfterRaising,
    bounceAfterLowering,
    raising,
    lowering,
    up,
    down,
    servoInterval = 20,
} ;

const int servoPosMax = 135 ;
const int servoPosMin = 45 ;
const int massInertiaSteps = 9 ;

void servoControl( ) {
    static uint8_t servoState = up ;

    if( !servoT ) { servoT = servoInterval ; // 20ms?

        switch( servoState ) {
            static uint8_t steps = 0 ;

        case up:
            if( signal.state == yellow 
            ||  signal.state == red ) { 
                servoState = raising ;
            }
            break ;
            
            
        case lowering:
            if( servoPos < servoPosMin ) servoPos -- ;

            if( servoPos ==  servoPosMin ) servoState = bounceAfterLowering ;
            servoState = bounceAfterLowering ;
            break ;

        case bounceAfterLowering:
            if( steps < massInertiaSteps / 2 )    servoPos -- ;
            else                                  servoPos ++ ;

            steps ++ ;
            if( steps == massInertiaSteps ) {
                steps = 0 ;
                servoState = down ;
            }
            break ;

        case down:
            if( signal.state == green ) servoState = raising ;
            break ;

        case raising: 
            if( servoPos < servoPosMax ) servoPos++ ;

            if( servoPos ==  servoPosMax ) servoState = bounceAfterRaising ;
            break ; 

        case bounceAfterRaising:
            if( steps < massInertiaSteps / 2 )    servoPos ++ ;
            else                                  servoPos -- ;

            steps ++ ;
            if( steps == massInertiaSteps ) {
                steps = 0 ;
                servoState = up ;
            }
            break ; 
        }
    }
}


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
            case driveOnSight:   signal.sendFreq = yellowFreq ;   break ; // in the event of driving on sight, signal yellow to previous staet
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

    if( signal.locked == 1 ) {                // a locked signal may be passed from behind
        digitalWrite( relayPin, LOW ) ;
        digitalWrite( slowSpeed, LOW );
    }
    else if( signal. state == red ) {
        digitalWrite( relayPin, HIGH ) ;
        digitalWrite( relayPin, LOW ) ;
    }
    else if( signal.state == yellow ) {    // yellow signal must also set slow speed signal. This may be depended on module type.
        digitalWrite( slowSpeed, HIGH );
        digitalWrite( relayPin, HIGH ) ;
    } 
    else {
        digitalWrite( slowSpeed, LOW );
        digitalWrite( relayPin, LOW ) ;
    }
}



void fadeLeds() {
    if( (  greenLed.state == 1 ) && (  greenLed.pwm <  greenLed.max ) )  greenLed.pwm ++ ; // actual brightness is handled in timer 1 ISR
    if( ( yellowLed.state == 1 ) && ( yellowLed.pwm < yellowLed.max ) ) yellowLed.pwm ++ ;
    if( (    redLed.state == 1 ) && (    redLed.pwm <    redLed.max ) )    redLed.pwm ++ ;

    if( (  greenLed.state == 0 ) && (  greenLed.pwm >  greenLed.min ) )  greenLed.pwm -- ;
    if( ( yellowLed.state == 0 ) && ( yellowLed.pwm > yellowLed.min ) ) yellowLed.pwm -- ;
    if( (    redLed.state == 0 ) && (    redLed.pwm >    redLed.min ) )    redLed.pwm -- ;

    if( greenLed.state )    digitalWrite( greenLedPin, HIGH ) ;
    else                    digitalWrite( greenLedPin, LOW ) ;
    if( yellowLed.state )   digitalWrite( yellowLedPin, HIGH ) ;
    else                    digitalWrite( yellowLedPin, LOW ) ;
    if( redLed.state )      digitalWrite( redLedPin, HIGH ) ;
    else                    digitalWrite( redLedPin, LOW ) ;
}



/* time 1 handles software pwm for all 3 leds and the servo motor */
ISR(TIMER1_COMPA_vect) { // timer 1 ISR must run at 9kHz
    static byte dutyCycle = 0;
    static byte servoPulse = 0;
    static byte counter = 0 ;

    if( servoPulse == 0 ) digitalWrite( servoPin, HIGH ) ;

    if( dutyCycle == 0 ) {
        if( redLed.pwm    > 0 ) digitalWrite( redLedPin,    HIGH ) ; // change to port instructions when finished
        if( yellowLed.pwm > 0 ) digitalWrite( yellowLedPin, HIGH ) ;
        if( greenLed.pwm  > 0 ) digitalWrite( greenLedPin,  HIGH ) ;
    }

    if( dutyCycle ==    redLed.pwm )  digitalWrite( redLedPin,    LOW );
    if( dutyCycle == yellowLed.pwm )  digitalWrite( yellowLedPin, LOW );
    if( dutyCycle ==  greenLed.pwm )  digitalWrite( greenLedPin,  LOW );

    if( servoPulse == servoPos )     digitalWrite( servoPin,  LOW ) ;

    if( ++counter == 255 ) counter = 0 ;
    if( ++servoPulse == 20000 ) servoPulse = 0 ; // 20
    
}

extern void initTimer1() {
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 8khz increments
    OCR1A = 149;// = (16*10^6) / (1000*64) - 1 (must be <156)
    // turn on CTC mode
    TCCR1A |= (1 << WGM11);
    // Set CS11and and CS10 bit for 64 prescaler
    TCCR1B |=  (1 << CS10);
    TCCR1B |=  (1 << CS11); 
    TCCR1B &= ~(1 << CS12); 
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A); 

    greenLed.pwm = 255 ;
    yellowLed.pwm = 0 ;
    redLed.pwm = 0 ;
}