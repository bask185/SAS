// uint8_t processButtons(Signal *signal) {

// 	if( greenButton->getState () == FALLING ) {

// 		if( signal->section == occupied 
// 		&&  signal->type != mainSignal ) return driveOnSight ; 
// 		else							 return green ; 
// 	}

// 	if( yellowButton.getState () == FALLING ) {
// 		if( signal->section == occupied 
// 		&&  signal->type != mainSignal ) return driveOnSight ;
// 		else							 return yellow ;
// 	}

// 	if( redButton.getState () == FALLING ) {
// 		return red ;
// 	}

// 	return undefined ;
// }


// uint8_t fallTimeControl(Signal *signal) {
// 	static uint8_t prevTime = 0;
// 	if( fallT != prevTime ) {
// 		prevTime = fallT ;
// 		Serial.println( fallT ) ;
// 	}

// 	if( signal->detectorState == RISING ) {	// if the detector no longer sees the train and there is no incomming signal, the yellow/red delay timer must be set.
// 		fallT = analogRead( potentiometer ) / 10 ;		// a time based signal must no longer last than 100 seconds tops.
// 		if( fallT <= 2 ) fallT = 2 ; 					// a mininum of 2 seconds is required
// 		Serial.print("fall time = ");Serial.println(fallT);
// 	}

// 	if( fallT == 1 ) { // in the last second of fall time  // change signal state
// 		fallT = 0 ;

// 		if( signal->type == combiSignal) {		// combi signal goes first to yellow before going to green
// 			if( signal->state == red ) {
// 				fallT = analogRead( potentiometer ) / 10 ;
// 				return yellow ;
// 			}
// 			else if ( signal->state == yellow || signal->state == driveOnSight ) { return green ; }
// 		}

// 		if( signal->type == mainSignal ) { 
// 			return green ;
// 		}	// main signal goes straight to green
// 	}
// 	return undefined ;
// }

// uint8_t processSignals(NextSignal *nextSignal) {
// 	static uint8_t previousSignalState;

// 	if( previousSignalState != nextSignal->state ) {
// 		previousSignalState = nextSignal->state ;

// 		switch( nextsignal->state ) {	// set these flags
// 			case red:    nextSignal->transitionedToRed	 = 1 ; Serial.println(" nextsignal->transitionedToRed"); break ;
// 			case yellow: nextSignal->transitionedToYellow = 1 ; Serial.println(" nextsignal->transitionedToYellow"); break ;
// 			case green:	 nextSignal->transitionedToGreen	 = 1 ; Serial.println(" nextsignal->transitionedToGreen");break ;
// 		}

// 		switch( signal->type ) {
		
// 		/* DUTCH PRE SIGNAL KNOWS ONLY EXPECTING GREEN OR RED */
// 		case dutchPreSignal: 
// 			switch( nextSignal->state ) {
// 				default:	 return undefined ;
// 				case green:	 return expectGreen ; 
// 				case red:	 return expectRed ; 
// 			}

// 		/* GERMAN PRE SIGNAL KNOWS EXPECTING GREEN, YELLOW OR RED */
// 		case germanPreSignal:
// 			switch( nextSignal->state ) {
// 				default:	 return undefined ;
// 				case green:  return expectGreen ;
// 				case yellow: return expectYellow ;
// 				case red:	 return expectRed ;
// 			}

// 		case mainSignal:	// if a main signal receives a signal that the following state is red, it's own state becomes green
// 			switch( nextSignal->state ) {
// 				default:	 return undefined ; // ignore green and yellow states
// 				case red:	 return green ;
// 			}

// 		case combiSignal:
// 			switch( nextSignal->state ) {
// 				default:	 return undefined ;
// 				case green:
// 				case yellow: return green ;
// 				case red:	 return yellow ;
// 			}
// 		}
// 	}
// 	return undefined ;
// }


// void readInputs(Signal *signal) {
// 	// static uint8_t nextsignal->statePrev ;

// 	if( !debounceT ) {
// 		debounceT = 200 ; // 200ms debounce time

// 		// debounce inputs
// 		locksignal->debounce() ;
// 		detector.debounce() ;
// 		redButton.debounce() ;
// 		greenButton.debounce() ;
// 		yellowButton.debounce() ;
// 	}


// 	// read in lock signal
// 	signal->locked = locksignal->getState()  ;

// 	if( signal->type == entrySignal ) { // in the event of an entry signal, the lockpin must be inverted
// 		if(		 signal->locked ==  ON ) signal->locked == OFF ; // 
// 		else if( signal->locked == OFF ) signal->locked ==  ON ;
// 	}
	
// 	// read in the detector
// 	signal->detectorState = detector.getState() ;
// 	if(signal->detectorState == RISING ) {Serial.println("DETECTOR ROSE ");}
// 	if(signal->detectorState == FALLING ) {Serial.println("DETECTOR FELL ");}
	
	

// 	// read in incomming signal from following module
// 	signal->connected = 1; // set true
// 	if (	 signal->recvFreq >  greenFreq - 3 && signal->recvFreq <  greenFreq + 3 ) { nextsignal->state =     green ; }
// 	else if( signal->recvFreq > yellowFreq - 3 && signal->recvFreq < yellowFreq + 3 ) { nextsignal->state =    yellow ; }
// 	else if( signal->recvFreq >    redFreq - 3 && signal->recvFreq <    redFreq + 3 ) { nextsignal->state =       red ; }
// 	else { signal->connected = 0;													  nextsignal->state = undefined ; } // no known frequency means not connected
// }