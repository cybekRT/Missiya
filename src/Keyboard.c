#include<conio.h>
#include<dos.h>

int kbdPressed = 0;
int kbdValue = 0;
int keyState[128] = {0};

/*
	Left	- 0x4b
	Right	- 0x4d
	Up	- 0x48
	Down	- 0x50
	Space	- 0x39
	Return	- 0x1c
	ESC	- 0x01
*/
void _WCINTERRUPT KeyboardISR()
{
	kbdPressed = 1;
	kbdValue = inp(0x60);

	if(kbdValue & 0x80)
	{
		keyState[kbdValue & 0x7F] = 0;
		kbdValue = kbdPressed = 0;
	}
	else
		keyState[kbdValue] = 1;

	outpw(0x20, 0x20);
}

void Keyboard_Init()
{
	_dos_setvect(9, KeyboardISR);
}

int Keyboard_IsPressed(int key)
{
	if(key >= 128)
		return 0;

	return keyState[key];
}

int Keyboard_GetAny()
{
	kbdPressed = 0;
	while(!kbdPressed)
	{
		__asm("hlt");
	}

	return kbdValue;
}

void Keyboard_Wait(int key)
{
	if(key >= 128)
		return;

	while(!keyState[key])
	{
		__asm("hlt");
	}
}

void Keyboard_SetKeyPressed(int key)
{
	if(key >= 128) // <0, huh
		return;

	keyState[key] = 1;
}
