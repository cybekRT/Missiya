#pragma once

struct Position
{
	int x;
	int y;
};

struct Size
{
	int w;
	int h;
};

struct Rect
{
	struct Position pos;
	struct Size size;
};

int Game_Init();
int Game_LoadLevel(char* name);

void Game_Update();
void Game_Draw();
