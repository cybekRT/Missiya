#pragma once

#define KEY_LEFT	0x4b
#define KEY_RIGHT	0x4d
#define KEY_UP		0x48
#define KEY_DOWN	0x50
#define KEY_SPACE	0x39
#define KEY_RETURN	0x1c
#define KEY_ESC		0x01

#define KEY_F2		0x3c

void Keyboard_Init();
int Keyboard_IsPressed(int key);
int Keyboard_GetAny();
void Keyboard_Wait(int key);
void Keyboard_SetKeyPressed(int key);
