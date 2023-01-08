#include<stdlib.h>
#include<conio.h>
#include<dos.h>
#include<i86.h>
#include<string.h>
#include"Log.h"
#include"Video.h"

#define COLORKEY 0x0D
// uint8_t* buffer = (uint8_t*)0xA0000;

uint8_t prevVideoMode;
uint8_t* buffer;
uint8_t* vgaMemory = (uint8_t*)0xA0000;

void Video_Init()
{
	union REGS reg = {0};
	reg.h.ah = 0x0F;
	int386(0x10, &reg, &reg);
	prevVideoMode = reg.h.al;

	reg.w.ax = 0x13;
	int386(0x10, &reg, &reg);

	buffer = malloc(320*200);
}

void Video_Quit()
{
	union REGS reg = {0};
	reg.h.ah = 0;
	reg.h.al = prevVideoMode;
	int386(0x10, &reg, &reg);
}

void Video_Clear(uint8_t col)
{
	int a;
	for(a = 0; a < 320*200; a++)
		buffer[a] = col;
}

void Video_Flip()
{
	memcpy(vgaMemory, buffer, 320*200);
}

void Video_FlipRect(int x, int y, int w, int h)
{
	int a;

	if(x >= 320 || y >= 200)
		return;

	if(x < 0)
	{
		w += x;
		x = 0;
	}
	if(y < 0)
	{
		h += y;
		y = 0;
	}

	if(w < 0 || h < 0)
		return;

	if(x + w > 320)
		w = 320 - x;
	if(y + h > 200)
		h = 200 - y;

	for(a = 0; a < h; a++)
	{
		int offset = (y + a) * 320 + x;
		memcpy(vgaMemory + offset, buffer + offset, w);
	}
}

struct Image* Video_Load(char* name)
{
	int a;
	uint32_t w, h;
	struct Image* img;
	char path[64];
	int fileSize;
	FILE* f;

	sprintf(path, "data/%s.bin", name);
	f = fopen(path, "rb");

	LOG("Loading image: %s", path);
	if(!f)
	{
		LOG("Couldn't load file: %s", path);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread((char*)&w, sizeof(w), 1, f);
	fread((char*)&h, sizeof(h), 1, f);

	LOG("Image: %s, dimensions: %dx%d", path, w, h);
	img = malloc(sizeof(struct Image) + w*h);

	img->w = w;
	img->h = h;
	fread(&img->pixels, 1, w*h, f);
	if(ferror(f))
	{
		LOG("Unexpected end of file: %s", path);
		return NULL;
	}

	if(ftell(f) != fileSize)
		LOG("Remaining data in file: %s (%d/%d)", path, ftell(f), fileSize);

	fclose(f);
	return img;
}

void Video_Free(struct Image* img)
{
	free(img);
}

inline static void Video_SetPixel(int x, int y, uint8_t c)
{
	buffer[y * 320 + x] = c;
}

void Video_Copy(struct Image* img, int dx, int dy)
{
	// uint8_t c;
	// int y, x;
	// for(y = 0; y < img->h; y++)
	// {
	// 	for(x = 0; x < img->w; x++)
	// 	{
	// 		int fx = x + dx;
	// 		int fy = y + dy;
	// 		if(fx < 0 || fx >= 320 || fy < 0 || fy >= 200)
	// 			continue;

	// 		c = img->pixels[y * img->w + x];
	// 		if(c == COLORKEY)
	// 			continue;

	// 		Video_SetPixel(fx, fy, c);
	// 	}
	// }

	Video_CopyPart(img, 0, 0, img->w, img->h, dx, dy);
}

void Video_CopyPart(struct Image* img, int sx, int sy, int sw, int sh, int dx, int dy)
{
	uint8_t c;
	int y, x;
	for(y = 0; y < sh; y++)
	{
		for(x = 0; x < sw; x++)
		{
			int fx = x + dx;
			int fy = y + dy;
			if(fx < 0 || fx >= 320 || fy < 0 || fy >= 200)
				continue;

			c = img->pixels[(sy + y) * img->w + (sx + x)];
			if(c == COLORKEY)
				continue;

			Video_SetPixel(fx, fy, c);
		}
	}
}

void Video_DrawRect(int dx, int dy, int dw, int dh, uint8_t col)
{
	int x;
	int y;

	for(y = 0; y < dh; y++)
	{
		for(x = 0; x < dw; x++)
		{
			if(dx + x < 0 || dx + x >= 320 || dy + y < 0 || dy + y >= 200)
				continue;

			Video_SetPixel(dx + x, dy + y, col);
		}
	}
}
