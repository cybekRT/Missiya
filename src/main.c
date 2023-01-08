#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<dos.h>
#include<i86.h>
#include<string.h>
#include"Log.h"
#include"Keyboard.h"
#include"Timer.h"
#include"Video.h"
#include"Game.h"
#include"Sound.h"

int exitProgram = 0;
char firstLevel[64] = "lvl0";

void MenuLoop()
{
	struct Image* bg = Video_Load("bg_menu");
	if(!bg)
	{
		exitProgram = 1;
		return;
	}

	Video_Copy(bg, 0, 0);
	Video_Flip();

	for(;;)
	{
		int key = Keyboard_GetAny();

		if(key == KEY_ESC)
		{
			exitProgram = 1;
			break;
		}
		else if(key == KEY_SPACE)
		{
			break;
		}
	}

	Video_Free(bg); // <<< never used this function before, lol
}

int main(int argc, char* argv[])
{
	int frameCounter = 0;
	int frameTicks = 0;

	(void*)argc;
	(void*)argv;

	if(argc > 1)
	{
		strcpy(firstLevel, argv[1]);
	}

	Log_Init();
	Timer_Init();
	Keyboard_Init();
	Sound_Init();

	while(0)
	{
		__asm("hlt");
		printf("Pressed: %x\n", Keyboard_GetAny());
	}

	Video_Init();

	MenuLoop();

	if(!Game_Init())
		goto exit;
	if(!Game_LoadLevel(firstLevel))
		goto exit;

	while(!exitProgram)
	{
		__asm("hlt");

		if(Keyboard_IsPressed(KEY_ESC))
			exitProgram = 1;

		if(!timerNextFrame)
			continue;

		timerNextFrame = 0;

		Game_Update();
		Game_Draw();

		frameCounter++;
		if(Timer_GetTicks() >= frameTicks + 1000)
		{
			// LOG("Frames: %f", frameCounter / ((Timer_GetTicks() - frameTicks) / 1000.0f));
			frameCounter = 0;
			frameTicks = Timer_GetTicks();
		}
	}

exit:
	Sound_Quit();
	Video_Quit();
	fclose(logFile);

	if(exitProgram == 1)
		printf("Thanks for playing~!\n");
	else
		printf("Game crashed, check log file...\nLast log: \"%s\"\n", logBuffer);
	return 0;
}
