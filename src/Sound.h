#pragma once

int Sound_Init();
void Sound_Quit();
void Sound_ClearQueue();
void Sound_Update();
void Sound_Play(int freq, int ms);
void Sound_PlayWithDelay(int freq, int delay, int ms);
void Sound_PlayForOneFrame(int freq);
