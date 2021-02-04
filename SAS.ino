#include "src/basics/timers.h"
#include "src/basics/io.h"
#include "teachIn.h"
#include "input.h"
#include "output.h"
#include "logic.h"
#include "config.h"


/* General idea:

*    With the SAS modules, one can make make either a dutch
*    or a german signal system. The SAS require some input in the form
*    of dip switches to inform the signal whether what kind of signal
*    it is.

*    The SAS module can be controlled by several inputs. This may be the
*    detector pin, the signal from an adjacent module, input buttons, the 
*    lock pin, a dedicated fall time and button presses. logic is depended 
*    on the type of signal. Different signal types may react different to 
*    different inputs.

*    There is also a certain order of things. The lock pin has #1 priority
*    if a signal is locked it may be that the direction of the tracks is wrong
*    or a turnout is not in the right position. The 2nd highest priority 
*    belongs to the detector pin. If a module is locked or occupied, the 
*    signal is generally showing red.

*    if the signal is connected to an adjacent module, the state of the
*    adjacent module is also examined. This is especially the case for
*    pre signals, but other types may also use these signals.

*    Optionally up to 3 buttons may be connected to a SAS for red,
*    green and yellow states. The green and yellow button may override
*    a red showing signal. If that is the case the state of the signal
*    will display drive-on-sight. A locked signal can not be overriden

* KNOWN BUGS
- If a train passes a combi signal this signal becomes red, but it may be that the following signal sends a signal which turns this 
signal on green, eventhough a train just past. A transition from red to orange may from the following module may 'free' the block.
a transition from orange to green may not free the block. If the block is not freed, the combi signal must not be allowed to transition
to green.

- if an entire section is used to detect a train a combiSignal must use both the detector as well as the following signal. A signal
must remember that the following module transitioned to red (which would normally free the block). If the following combi signal jumps
to red while the detector still sees a train, the combin signal must not transition to yellow.
when it's own detector has rissen AND the following module is red, than a combiSignal may become orange. 
For this to work with both short and long detected sections, a rissen flag must be added to SW

Note:
when may a red showing combi signal become green?
    the detector must have rissen and the following signal must have transitioned to red -> SECTION FREED AND signal is not override by yellow or red

when may a yellow showing combi signal become green?
    The section must be free and the following module
*/


void setup() {
    cli() ;
    
    initIO() ;
    initTimers() ;
    // initTimer0() ;
    sei() ;

    redLed.max = 255 ;
    yellowLed.max  = 255 ;
    greenLed.max  = 20 ;

    signal.locked = 0 ;
    signal.section = available ; // INPUT DIPSWITCHES NEEDED HERE
    signal.type = combiSignal ;
    signal.state = green ;
    signal.wasLocked = 0 ;

    Serial.begin(115200);
    Serial.println("sup");
}



void loop() {
    
    // input
    debounceInputs() ;
    
    if( teachIn() ) return ;    // if this menu is active, all other code is to be disabled
    
    readIncFreq() ;
    readDirection() ;
    readDetector() ;
    
    // logic
    computeLogic( ) ;    // takes input and calculate what all relevant variables should be.
    setLedStates( ) ;

    //  output
    sendSignals() ;                             // send the signal to the adjacent module
    repeat( &fadeT, 1, fadeLeds );              // fade leds in and out to emulate glowblub effects, currently this takes 1/4 of a setting
    repeat( &servoT, 20, servoControl ) ;       // handle the arm's servo motor including mass inertia movement every, takes 1 step every x time
    controlBrakeModule() ;                      // handles the braking modules/shutoff relay
}












    // debug stuff
    // if( //Serial.read () == 'd') printStuff()
// if( //Serial.available () ) {
//     byte b = //Serial.read() ;

//     // if ( b == 'g' ) signal.sendFreq = 10 ; used to test the signal transmitting
//     // if ( b == 'y' ) signal.sendFreq = 20 ;
//     // if ( b == 'r' ) signal.sendFreq = 30 ;
//     // if ( b == 'o' ) signal.sendFreq = 0 ;
//     if ( b == 'd' ) printStuff() ;
// }

// #define printType(x) case x: //Serial.println(#x) ; break
// void printStuff(){
//     if( !printT ) { printT = 200;

//         //Serial.write( 12 ) ;
//         ////Serial.println("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n") ;
//         //Serial.print("signal type     : ") ;
//         switch(signal.type) {
//             printType(mainSignal) ;
//             printType(entrySignal) ;
//             printType(germanPreSignal) ;
//             printType(dutchPreSignal) ;
//             printType(combiSignal) ;
//         }

//         //Serial.print("buttons         : ") ;
//         switch(signal.buttons) {
//             printType(green) ;
//             printType(yellow) ;
//             printType(red) ;
//             printType(undefined) ;
//         }

//         //Serial.print("send Freq       : ") ; //Serial.println( signal.sendFreq ) ;
//         //Serial.print("recv Freq       : ") ; //Serial.println( signal.recvFreq ) ;
//         //Serial.print("nextState       : ") ;
//         switch( nextSignal.state ) {
//             printType(green) ;
//             printType(yellow) ;
//             printType(red) ;
//             printType(undefined) ;
//         }

//         //Serial.print("locked          : ") ;
//         switch(signal.locked) {
//             printType(ON) ;
//             printType(OFF) ;
//         }

//         //Serial.print("red Led State   : ") ;
//         switch(signal.redLedState) {
//             printType(1) ;
//             printType(0) ;
//         }

//         //Serial.print("yellow led State: ") ;
//         switch(signal.yellowLedState) {
//             printType(1) ;
//             printType(0) ;
//         }

//         //Serial.print("green Led State : ") ;
//         switch(signal.greenLedState) {
//             printType(1) ;
//             printType(0) ;
//         }

//         //Serial.print("section         : ") ;
//         switch(signal.section) {
//             printType(available) ;
//             printType(occupied) ;
//         }
//     }
// }

