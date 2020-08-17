#include "src/basics/timers.h"
#include "src/basics/io.h"
#include "roundRobinTasks.h"
#include "teachIn.h"

void setup() {
	
	initIO();

	initTimers();

	initRR();

	
	Serial.begin(115200);
	Serial.println("BOOTING SAS MODULE");

}

void loop() {
	processRoundRobinTasks();

	//teachIn();
}