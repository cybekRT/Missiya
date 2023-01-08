#pragma once

#include"Game.h"

struct Robot
{
	struct Position pos;
	struct Position posLeft, posRight;
	struct Size size;
	int speedNormal, speedDetected;
	int dir;
};
