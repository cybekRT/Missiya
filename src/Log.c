#include<stdio.h>

FILE* logFile;
char logBuffer[1024] = {0};

int Log_Init()
{
	logFile = fopen("missiya.log", "w");
	if(!logFile)
	{
		printf("Oopsie...\n");
		return 0;
	}

	return 1;
}
