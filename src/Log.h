#pragma once

#include<stdio.h>

extern FILE* logFile;
extern char logBuffer[];

int Log_Init();

// #define LOG(...) { fprintf(logFile, __VA_ARGS__); fputc('\n', logFile); fflush(logFile); }
#define LOG(...) { sprintf(logBuffer, __VA_ARGS__); fputs(logBuffer, logFile); fputc('\n', logFile); fflush(logFile); }
