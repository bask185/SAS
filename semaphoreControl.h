enum semaphoreControlStates {
	semaphoreControlIDLE,
	up,
	lowering,
	down,
	raising,
};

extern bool semaphoreControl(void); 
extern void semaphoreControlInit();
extern void semaphoreControlSetState(unsigned char);
extern unsigned char semaphoreControlGetState(void);
