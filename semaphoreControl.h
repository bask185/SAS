enum semaphoreControlStates {
	semaphoreControlIDLE,
	up,
	lowering,
	bounceAfterLowering,
	down,
	raising,
	bounceAfterRaising };

extern bool semaphoreControl(void); 
extern void semaphoreControlInit();
extern void semaphoreControlSetState(unsigned char);
extern unsigned char semaphoreControlGetState(void);
