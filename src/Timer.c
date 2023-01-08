#include<conio.h>
#include<dos.h>
#include"Sound.h"

int timerNextFrame = 0;
int timerTicks = 0;

void _WCINTERRUPT TimerISR()
{
	// timerNextFrame = 1;

	timerTicks++;
	if(timerTicks % 50 == 0)
	// if(timerTicks % 10 == 0)
		timerNextFrame = 1;

	Sound_Update();

	outpw(0x20, 0x20);
}

void Timer_Init()
{
	// PIC0 - 0x08, PIC1 - 0x70
	_dos_setvect(8, TimerISR);

	// PIT
	// 0x40 - ch0
	// 0x43 - mode/cmd
	outp(0x43, 0x34);
	outp(0x40, 0xa9);
	outp(0x40, 0x04);
}

int Timer_GetTicks()
{
	return timerTicks;
}

void Timer_Delay(int ms)
{
	int dstTimer = timerTicks + ms;
	while(timerTicks < dstTimer)
	{
		__asm("hlt");
	}
}