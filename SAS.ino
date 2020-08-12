#include "src/basics/timers.h"
#include "src/basics/io.h"
#include "roundRobinTasks.h"
#include "teachIn.h"

void setup() {
	initTimers();
	initIO();
	Serial.begin(115200);
	teachInInit();
}

void loop() {
	processRoundRobinTasks();

	teachIn();
}