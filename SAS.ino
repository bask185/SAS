#include "src/basics/timers.h"
#include "src/basics/io.h"
#include "roundRobinTasks.h"
#include "teachIn.h"

void setup() {
	
	initIO();
	cli();
	initTimers();
	sei();

	initRR();

	
	Serial.begin(115200);
	Serial.println("MODULE BOOTED");

}

void loop() {
	processRoundRobinTasks();

	//teachIn();
}