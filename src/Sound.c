#include<i86.h>
#include"Sound.h"
#include"Timer.h"
#include"Log.h"

#define FRAMES_LIMIT 16

struct AudioFrame
{
	int startTime;
	int endTime;
	int freq;
};

struct AudioFrame frames[FRAMES_LIMIT];

int Sound_Init()
{
	Sound_ClearQueue();

	return 1;
}

void Sound_Quit()
{
	Sound_ClearQueue();
	nosound();
}

void Sound_ClearQueue()
{
	int a;

	for(a = 0; a < FRAMES_LIMIT; a++)
	{
		frames[a].startTime = frames[a].endTime = 0;
	}
}

void Sound_Update()
{
	int a;
	int index = -1;
	int ticks = Timer_GetTicks();

	for(a = 0; a < FRAMES_LIMIT; a++)
	{
		if(frames[a].endTime < ticks)
			continue;

		if(index == -1 || frames[a].endTime < frames[index].endTime)
			index = a;
	}

	if(index >= 0)
		sound(frames[index].freq);
	else
		nosound();
}

void Sound_Play(int freq, int ms)
{
	Sound_PlayWithDelay(freq, 0, ms);
}

void Sound_PlayWithDelay(int freq, int delay, int ms)
{
	int a;
	int index = -1;
	int ticks = Timer_GetTicks();

	for(a = 0; a < FRAMES_LIMIT; a++)
	{
		if(frames[a].endTime < ticks)
		{
			index = a;
			break;
		}
	}

	if(index < 0)
	{
		LOG("Too many sounds playing");
		return;
	}

	frames[index].startTime = ticks + delay;
	frames[index].endTime = ticks + delay + ms;
	frames[index].freq = freq;

	// Sound_Update(); // needs it?
}

void Sound_PlayForOneFrame(int freq)
{
	Sound_Play(freq, TIMER_FRAME_DELAY);
}
