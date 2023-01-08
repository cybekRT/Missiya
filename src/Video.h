#pragma once

#include<stdint.h>

struct Image
{
	uint32_t w;
	uint32_t h;
	uint8_t pixels[32*32];
};

void Video_Init();
void Video_Quit();
void Video_Clear(uint8_t col);
void Video_Flip();
void Video_FlipRect(int x, int y, int w, int h);

struct Image* Video_Load(char* name);
void Video_Free(struct Image* img);

void Video_Copy(struct Image* img, int dx, int dy);
void Video_CopyPart(struct Image* img, int sx, int sy, int sw, int sh, int dx, int dy);
void Video_DrawRect(int x, int y, int w, int h, uint8_t col);
