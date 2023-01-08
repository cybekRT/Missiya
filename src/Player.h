#pragma once

#include"Game.h"

typedef enum
{
	PLAYER_STATE_STOP = 0,
	PLAYER_STATE_LEFT,
	PLAYER_STATE_RIGHT,
	PLAYER_STATE_CROUCH,
	PLAYER_STATE_JUMP,
	PLAYER_STATE_FALL,

	PLAYER_STATE_COUNT
} PlayerState;

typedef enum
{
	PLAYER_DIR_NONE = 0,
	PLAYER_DIR_LEFT,
	PLAYER_DIR_RIGHT,

	PLAYER_DIR_COUNT
} PlayerDirection;

struct Player
{
	struct Image* imgs[PLAYER_STATE_COUNT];
	struct Position pos;
	struct Size size;
	struct Position posOld;

	PlayerDirection dir;
	PlayerState state;
	int onGround;
	int animTimer;
	int jumpTimer;

	int alive;
	int dataHarvested;
};
