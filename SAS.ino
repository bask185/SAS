#include "src/basics/timers.h"
#include "src/basics/io.h"
#include "roundRobinTasks.h"
#include "teachIn.h"

void setup() {
	
	initIO();

	

	//delay(5000);
	//teachInInit();

	initRR();

	Serial.begin(115200);
	Serial.println("BOOTING SAS MODULE");

	// digitalWrite(9, LOW );
	// digitalWrite(redLed, HIGH);
	// for(int i = 255;i>=0;i--){
	// 	 analogWrite(9,i);
	// 	 Serial.println(i);
	// 	 delay(20);
	// }
	// digitalWrite(redLed, LOW);
	// digitalWrite(yellowLed, HIGH);
	// for(int i = 255;i>=0;i--){
	// 	 analogWrite(9,i);
	// 	 Serial.println(i);
	// 	 delay(20);
	// }
	// digitalWrite(yellowLed, LOW);
	// digitalWrite( greenLed, HIGH);
	// for(int i = 255;i>=0;i--){
	// 	 analogWrite(9,i);
	// 	 Serial.println(i);
	// 	 delay(20);
	// }

	initTimers();
}

void loop() {
	processRoundRobinTasks();

	//teachIn();
}