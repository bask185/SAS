#include <Arduino.h>

void processRoundRobinTasks();
#define updateIO(); updateOutputs(); \
updateInputs();

extern uint8_t recvFreq ;