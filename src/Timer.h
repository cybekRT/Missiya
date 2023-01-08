#pragma once

// 20 FPS
#define TIMER_FRAME_DELAY 1000/20
extern int timerNextFrame;

void Timer_Init();
int Timer_GetTicks();
void Timer_Delay(int ms);
