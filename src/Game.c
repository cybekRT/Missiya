#include<string.h>
#include<stdio.h>
#include<i86.h>
#include"Log.h"
#include"Video.h"
#include"Keyboard.h"
#include"Game.h"
#include"Timer.h"
#include"Sound.h"

#include"Player.h"
#include"Computer.h"
#include"Brick.h"
#include"Robot.h"

const int levelW = 320;
const int levelH = 200;
char levelCurrent[64] = {0};

int songLevelStart[] = { 1000, 400, 100, 80, 200, 160, 60, 100, 200 };
int songLevelStartStep = 200;

#define BACKGROUNDS_TYPES 4
#define FLOORS_TYPES 2
#define WALLS_TYPES 2

struct Image* imgBackground[BACKGROUNDS_TYPES];
struct Image* imgFloor[FLOORS_TYPES];
struct Image* imgFloorTop[FLOORS_TYPES];
struct Image* imgWall[WALLS_TYPES];
struct Image* imgComputer;
struct Image* imgComputerEmpty;
struct Image* imgTeleport[2];
struct Image* imgRobot[2];

struct Image* imgHudOverlay;
struct Image* imgHudFloppy;
struct Image* imgHudTeleport;
int needsFullRefresh = 1;

// HUD
struct Position posHudFloppy = { .x = 32, .y = 2 };

struct Player player;
const int playerJumpPower = 30 * 4;

#define FLOORS_LIMIT 8
#define WALLS_LIMIT 8

int currentBackground = 0;
int gravity;
struct Brick floors[FLOORS_LIMIT];
int floorsCount = 0;

const int floorWidth = 16;
const int floorHeight = 8;
const int wallWidth = 16;
const int wallHeight = 16;
const int robotWidth = 16;
const int robotHeight = 20;
const int playerSpeed = 3;

struct Brick walls[WALLS_LIMIT];
int wallsCount = 0;

struct Position posTeleport;
char levelNext[64];

#define ROBOTS_LIMIT 8
struct Robot robots[ROBOTS_LIMIT];
int robotsCount = 0;

#define COMPUTERS_LIMIT 8
struct Computer computers[COMPUTERS_LIMIT];
int computersCount = 0;
int computerHarvested = -1;
int computersRemaining;

int Game_Init()
{
	char tmp[64];
	int a;

	// Backgrounds
	for(a = 0; a < BACKGROUNDS_TYPES; a++)
	{
		sprintf(tmp, "bg%d", a);
		imgBackground[a] = Video_Load(tmp);
		if(!imgBackground[a])
			return 0;
	}

	// Player
	player.imgs[PLAYER_STATE_STOP] = Video_Load("pla_s");
	player.imgs[PLAYER_STATE_LEFT] = Video_Load("pla_l");
	player.imgs[PLAYER_STATE_RIGHT] = Video_Load("pla_r");
	player.imgs[PLAYER_STATE_CROUCH] = Video_Load("pla_c");
	player.imgs[PLAYER_STATE_JUMP] = Video_Load("pla_j");
	player.imgs[PLAYER_STATE_FALL] = Video_Load("pla_f");

	for(a = 0; a < PLAYER_STATE_COUNT; a++)
	{
		if(!player.imgs[a])
			return 0;
	}

	player.size.w = player.imgs[PLAYER_STATE_STOP]->w;
	player.size.h = player.imgs[PLAYER_STATE_STOP]->h;

	// Walls and floors
	for(a = 0; a < FLOORS_TYPES; a++)
	{
		sprintf(tmp, "floor%d", a);
		imgFloor[a] = Video_Load(tmp);
		sprintf(tmp, "floor%d_t", a);
		imgFloorTop[a] = Video_Load(tmp);
		if(!imgFloor[a] || !imgFloorTop[a])
			return 0;
	}

	for(a = 0; a < WALLS_TYPES; a++)
	{
		sprintf(tmp, "wall%d", a);
		imgWall[a] = Video_Load(tmp);
		if(!imgWall[a])
			return 0;
	}

	// Computer
	imgComputer = Video_Load("pc");
	imgComputerEmpty = Video_Load("pc_d");
	if(!imgComputer || !imgComputerEmpty)
		return 0;

	// Teleport
	imgTeleport[0] = Video_Load("tp_ina3");
	imgTeleport[1] = Video_Load("tp_act3");
	if(!imgTeleport[0] || !imgTeleport[1])
		return 0;

	// Robot
	imgRobot[0] = Video_Load("robot2_l");
	imgRobot[1] = Video_Load("robot2_r");
	if(!imgRobot[0] || !imgRobot[1])
		return 0;

	// HUD
	imgHudOverlay = Video_Load("hud");
	imgHudFloppy = Video_Load("hud_fdd");
	imgHudTeleport = Video_Load("hud_tp");
	if(!imgHudOverlay || !imgHudFloppy || !imgHudTeleport)
		return 0;

	needsFullRefresh = 1;
	return 1;
}

int Game_LoadLevel(char* name)
{
	int a;
	char tmp[32];
	char path[64];
	int status = 0;
	FILE* f;

	if(levelCurrent[0] != 0)
	{
		Video_Clear(0);
		for(a = 0; a < 200; a++)
		{
			Video_FlipRect(0, a, 320, 1);
			Timer_Delay(5);
		}
	}

	// Save
	strcpy(levelCurrent, name);

	// Clear variables
	currentBackground = 0;
	gravity = 0;
	floorsCount = 0;
	wallsCount = 0;
	computersCount = 0;
	robotsCount = 0;

	player.size.w = player.imgs[PLAYER_STATE_STOP]->w;
	player.size.h = player.imgs[PLAYER_STATE_STOP]->h;
	player.dir = PLAYER_DIR_NONE;
	player.state = PLAYER_STATE_STOP;
	player.onGround = 0;
	player.jumpTimer = 0;
	player.alive = 1;
	player.dataHarvested = 0;

	LOG("Opening level: %s\n", name);
	sprintf(path, "data/%s", name);
	f = fopen(path, "r");
	if(!f)
	{
		LOG("Couldn't open file: %s\n", path);
		return 0;
	}

	LOG("Reading level...");
	while(fscanf(f, "%s", tmp) > 0)
	{
		LOG("Tag: \"%s\"", tmp);
		if(strcmp(tmp, "background") == 0)
		{
			fscanf(f, "%d", &currentBackground);

			if(currentBackground >= BACKGROUNDS_TYPES)
			{
				LOG("Invalid background: %d", currentBackground)
				goto exit;
			}
		}
		else if(strcmp(tmp, "player") == 0 || strcmp(tmp, "player_b") == 0)
		{
			fscanf(f, "%d %d", &player.pos.x, &player.pos.y);

			if(strcmp(tmp, "player_b") == 0)
				player.pos.y -= player.size.h;

			player.posOld = player.pos;
		}
		else if(strcmp(tmp, "gravity") == 0)
		{
			fscanf(f, "%d", &gravity);
		}
		else if(strcmp(tmp, "floor") == 0)
		{
			fscanf(f, "%d %d %d %d %d",
				&floors[floorsCount].type,
				&floors[floorsCount].pos.x, &floors[floorsCount].pos.y,
				&floors[floorsCount].size.w, &floors[floorsCount].size.h);

			if(floors[floorsCount].type >= FLOORS_TYPES)
			{
				LOG("Invalid floor type: %d", floors[floorsCount].type);
				goto exit;
			}

			if(floors[floorsCount].size.w % floorWidth != 0)
			{
				LOG("Invalid wall size: %d %% %d != 0", floors[floorsCount].size.w, floorWidth);
				goto exit;
			}

			floorsCount++;
			if(floorsCount > FLOORS_LIMIT)
			{
				LOG("Floors limit exceeded");
				goto exit;
			}
		}
		else if(strcmp(tmp, "wall") == 0)
		{
			fscanf(f, "%d %d %d %d %d",
				&walls[wallsCount].type,
				&walls[wallsCount].pos.x, &walls[wallsCount].pos.y,
				&walls[wallsCount].size.w, &walls[wallsCount].size.h);

			if(walls[wallsCount].type >= WALLS_TYPES)
			{
				LOG("Invalid wall type: %d", walls[wallsCount].type);
				goto exit;
			}

			if(walls[wallsCount].size.w % wallWidth != 0)
			{
				LOG("Invalid wall width: %d %% %d != 0", walls[wallsCount].size.w, wallWidth);
				goto exit;
			}

			if(walls[wallsCount].size.h % wallHeight != 0)
			{
				LOG("Invalid wall height: %d %% %d != 0", walls[wallsCount].size.h, wallHeight);
				goto exit;
			}

			wallsCount++;
			if(wallsCount > FLOORS_LIMIT)
			{
				LOG("Walls limit exceeded");
				goto exit;
			}
		}
		else if(strcmp(tmp, "computer") == 0 || strcmp(tmp, "computer_b") == 0)
		{
			fscanf(f, "%d %d %d",
				&computers[computersCount].pos.x, &computers[computersCount].pos.y,
				&computers[computersCount].dataToHarvest);

			if(strcmp(tmp, "computer_b") == 0)
				computers[computersCount].pos.y -= imgComputer->h;

			computers[computersCount].remainingData = computers[computersCount].dataToHarvest;

			computersCount++;
			if(computersCount > COMPUTERS_LIMIT)
			{
				LOG("Computers limit exceeded");
				goto exit;
			}
		}
		else if(strcmp(tmp, "teleport") == 0 || strcmp(tmp, "teleport_b") == 0)
		{
			fscanf(f, "%d %d %s",
				&posTeleport.x, &posTeleport.y, levelNext);

			if(strcmp(tmp, "teleport_b") == 0)
				posTeleport.y -= imgTeleport[0]->h;
		}
		else if(strcmp(tmp, "robot") == 0 || strcmp(tmp, "robot_b") == 0)
		{
			fscanf(f, "%d %d %d %d %d %d",
				&robots[robotsCount].posLeft.x, &robots[robotsCount].posLeft.y,
				&robots[robotsCount].posRight.x, &robots[robotsCount].posRight.y,
				&robots[robotsCount].speedNormal, &robots[robotsCount].speedDetected);

			if(strcmp(tmp, "robot_b") == 0)
			{
				robots[robotsCount].posLeft.y -= imgRobot[0]->h;
				robots[robotsCount].posRight.y -= imgRobot[0]->h;
			}

			robots[robotsCount].pos = robots[robotsCount].posLeft;
			robots[robotsCount].size.w = robotWidth;
			robots[robotsCount].size.h = robotHeight;
			robots[robotsCount].dir = 1;

			robotsCount++;
			if(robotsCount > ROBOTS_LIMIT)
			{
				LOG("Robots limit exceeded");
				goto exit;
			}
		}
		else
		{
			LOG("Invalid type: %s", tmp);
			goto exit;
		}

		if(ferror(f))
		{
			LOG("Unexpected end of file: %s", path);
			goto exit;
		}
	}

	// Set some variables...
	computersRemaining = computersCount;

	// Play song, yeaaaaa~!
	Sound_ClearQueue();
	for(a = 0; a < sizeof(songLevelStart) / sizeof(*songLevelStart); a++)
	{
		Sound_PlayWithDelay(songLevelStart[a], a * songLevelStartStep, songLevelStartStep);
	}

	// Fancy animation
	needsFullRefresh = 0;
	Game_Draw();
	for(a = 200; a >= 0; a--)
	{
		Video_FlipRect(0, a, 320, 1);
		Timer_Delay(5);
	}

	status = 1;
exit:
	needsFullRefresh = 1;
	fclose(f);
	return status;
}

int LevelRestart()
{
	const int songDelay = 100;

	// Play sad song
	Sound_ClearQueue();
	Sound_PlayWithDelay(1200, 0 * songDelay, songDelay);
	Sound_PlayWithDelay(800, 1 * songDelay, songDelay);
	Sound_PlayWithDelay(500, 2 * songDelay, songDelay);
	Sound_PlayWithDelay(200, 3 * songDelay, songDelay);

	return Game_LoadLevel(levelCurrent);
}

int LevelNext()
{
	Sound_PlayWithDelay(800, 0, 200);
	Sound_PlayWithDelay(1000, 300, 800);
	Sound_PlayWithDelay(1400, 600, 1500);
	return Game_LoadLevel(levelNext);
}

int Collide(struct Rect r1, struct Rect r2)
{
	// LOG("Check: %d %d %d %d",
	// 	r1.pos.x + r1.size.w < r2.pos.x,
	// 	r1.pos.y + r1.size.h < r2.pos.y,
	// 	r1.pos.x >= r2.pos.x + r2.size.w,
	// 	r1.pos.y >= r2.pos.y + r2.size.h);

	if(r1.pos.x + r1.size.w < r2.pos.x || r1.pos.y + r1.size.h < r2.pos.y
		|| r1.pos.x > r2.pos.x + r2.size.w || r1.pos.y > r2.pos.y + r2.size.h)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int IsPlayerOnGround()
{
	int a;
	struct Rect playerRect;
	struct Rect floorRect;
	playerRect.pos = player.pos;
	playerRect.size = player.size;

	for(a = 0; a < floorsCount; a++)
	{
		floorRect.pos = floors[a].pos;
		floorRect.size = floors[a].size;

		if(floors[a].pos.y != player.pos.y + player.size.h)
			continue;

		if(Collide(playerRect, floorRect))
			return 1;
	}

	return 0;
}

int IsPlayerNextToWall()
{
	int a;
	struct Rect playerRect;
	struct Rect wallRect;
	playerRect.pos = player.pos;
	playerRect.size = player.size;

	for(a = 0; a < wallsCount; a++)
	{
		wallRect.pos = walls[a].pos;
		wallRect.size = walls[a].size;

		// if(walls[a].pos.x + imgWall->w - 1 != player.pos.x && walls[a].pos.x != player.pos.x + player.size.w)
		// 	continue;

		if(Collide(playerRect, wallRect))
			return 1;
	}

	return 0;
}

void HarvestComputerData()
{
	int a;
	int index = -1;
	struct Rect playerRect;
	struct Rect computerRect;

	playerRect.pos = player.pos;
	playerRect.size = player.size;
	computerRect.size.w = imgComputer->w;
	computerRect.size.h = imgComputer->h;

	for(a = 0; a < computersCount; a++)
	{
		computerRect.pos = computers[a].pos;
		if(Collide(playerRect, computerRect)
			&& player.pos.y + player.size.h == computerRect.pos.y + computerRect.size.h
			&& player.pos.x >= computerRect.pos.x
			&& player.pos.x + player.size.w <= computerRect.pos.x + computerRect.size.w)
		{
			index = a;
			break;
		}
	}

	if(index < 0 || computers[index].remainingData <= 0)
	{
		computerHarvested = -1;
		return;
	}

	computers[index].remainingData--;
	computerHarvested = index;
}

void Player_Update()
{
	int a;
	PlayerState playerOldState = player.state;

	player.posOld = player.pos;

	for(a = 0; a < gravity; a++)
	{
		if(player.jumpTimer > 0)
		{
			player.onGround = 0;
			player.pos.y--;
			player.jumpTimer--;
		}
		else if(IsPlayerOnGround())
		{
			player.onGround = 1;
			break;
		}
		else
		{
			player.onGround = 0;
			player.state = PLAYER_STATE_FALL;
			player.pos.y++;

			if(player.pos.y > levelH)
				player.alive = 0;
		}
	}

	if(player.onGround)
	{
		int leftPressed = Keyboard_IsPressed(KEY_LEFT);
		int rightPressed = Keyboard_IsPressed(KEY_RIGHT);
		int jumpPressed = Keyboard_IsPressed(KEY_UP);
		int teleportPressed = Keyboard_IsPressed(KEY_DOWN);

		if(player.state == PLAYER_STATE_FALL)
			Sound_Play(150, 100);

		if(leftPressed && !rightPressed)
		{
			player.dir = PLAYER_DIR_LEFT;
		}
		else if(rightPressed && !leftPressed)
		{
			player.dir = PLAYER_DIR_RIGHT;
		}
		else
		{
			player.state = PLAYER_STATE_STOP;
			player.dir = PLAYER_DIR_NONE;
		}

		if(jumpPressed)
		{
			player.state = PLAYER_STATE_JUMP;
			player.jumpTimer = playerJumpPower / gravity;

			Sound_Play(300, 300);
		}
		else
		{
			if(player.dir == PLAYER_DIR_LEFT)
				player.state = PLAYER_STATE_LEFT;
			else if(player.dir == PLAYER_DIR_RIGHT)
				player.state = PLAYER_STATE_RIGHT;
		}

		if(player.state == PLAYER_STATE_STOP && Keyboard_IsPressed(KEY_SPACE))
		{
			HarvestComputerData();

			if(computerHarvested >= 0)
			{
				int timer = (Timer_GetTicks() - player.animTimer) / 100;

				if(computers[computerHarvested].remainingData <= 0)
					computersRemaining--;

				if(timer % 4 == 0)
					Sound_PlayForOneFrame(200);
				else if(timer % 4 == 1)
					Sound_PlayForOneFrame(300);
				else if(timer % 4 == 2)
					Sound_PlayForOneFrame(500);
				else if(timer % 4 == 3)
					Sound_PlayForOneFrame(200);
			}
		}
		else
		{
			computerHarvested = -1;
		}

		if(computersRemaining <= 0 && teleportPressed)
		{
			if(player.pos.x + player.size.w >= posTeleport.x
				&& player.pos.x < posTeleport.x + imgTeleport[0]->w
				&& player.pos.y + player.size.h == posTeleport.y + imgTeleport[0]->h)
			{
				if(!LevelNext())
				{
					extern int exitProgram;
					LOG("No more levels? Or invalid name: %s", levelNext);
					exitProgram = 2;
				}

				return;
			}
		}
	}

	if(player.dir != PLAYER_DIR_NONE)
	{
		for(a = 0; a < playerSpeed; a++)
		{
			int isOnWall;
			int wasOnWall;
			int dir;

			wasOnWall = IsPlayerNextToWall();

			if(player.dir == PLAYER_DIR_LEFT)
				dir = -1;
			else if(player.dir == PLAYER_DIR_RIGHT)
				dir = 1;

			player.pos.x += dir;
			isOnWall = IsPlayerNextToWall();

			if((isOnWall && wasOnWall) || player.pos.x < 0 || player.pos.x + player.size.w >= levelW)
			{
				player.pos.x -= dir;
				if(player.state == PLAYER_STATE_LEFT || player.state == PLAYER_STATE_RIGHT)
					player.state = PLAYER_STATE_STOP;
			}
		}
	}

	if(player.state != playerOldState)
	{
		player.animTimer = Timer_GetTicks();
		nosound();
	}

	if(player.state == PLAYER_STATE_LEFT || player.state == PLAYER_STATE_RIGHT)
	{
		int timer = (Timer_GetTicks() - player.animTimer) / 100;
		if(timer % 5 == 1)
			Sound_PlayForOneFrame(500);
	}
}

void Robots_Update()
{
	int a;
	int b;
	struct Rect playerRect;
	struct Rect robotRect;
	struct Rect robotAreaRect;

	playerRect.pos = player.pos;
	playerRect.size = player.size;

	for(a = 0; a < robotsCount; a++)
	{
		int speed = robots[a].speedNormal;
		robotRect.size = robots[a].size;

		LOG("Robot speed: %d", speed);

		robotAreaRect.pos = robots[a].posLeft;
		robotAreaRect.size.w = robots[a].posRight.x - robots[a].posLeft.x;
		robotAreaRect.size.h = robots[a].posRight.y - robots[a].posLeft.y;

		if(Collide(playerRect, robotAreaRect))
		{
			if(robots[a].dir > 0 && player.pos.x >= robots[a].pos.x + robots[a].size.w)
				speed = robots[a].speedDetected;
			if(robots[a].dir < 0 && player.pos.x + player.size.w < robots[a].pos.x)
				speed = robots[a].speedDetected;
		}

		for(b = 0; b < speed; b++)
		{
			robots[a].pos.x += robots[a].dir;

			if(robots[a].dir < 0 && robots[a].pos.x <= robots[a].posLeft.x)
				robots[a].dir = 1;
			else if(robots[a].dir > 0 && robots[a].pos.x + robots[a].size.w >= robots[a].posRight.x)
				robots[a].dir = -1;

			robotRect.pos = robots[a].pos;
			// TODO
			robotRect.pos.x += 4;
			robotRect.size.w -= 8;
			robotRect.pos.y += robotRect.size.h / 2;
			robotRect.size.h /= 2;
			robotRect.size.h -= 8;
			if(Collide(playerRect, robotRect))
			{
				player.alive = 0;
			}
		}
	}
}

void Game_Update()
{
	if(Keyboard_IsPressed(KEY_F2) || !player.alive)
	{
		// if(!Game_LoadLevel(levelCurrent))
		if(!LevelRestart())
		{
			extern int exitProgram;
			LOG("Couldn't restart level, something bad happened...");
			exitProgram = 2;
		}
		return;
	}

	Player_Update();
	Robots_Update();
}

void Game_FlipRect(struct Position* pos, struct Position* oldPos, int w, int h)
{
	int rx, ry, rw, rh;

	rx = (oldPos->x < pos->x) ? oldPos->x : pos->x;
	ry = (oldPos->y < pos->y) ? oldPos->y : pos->y;
	rw = (oldPos->x < pos->x) ? pos->x + w - rx : oldPos->x + w - rx;
	rh = (oldPos->y < pos->y) ? pos->y + h - ry : oldPos->y + h - ry;

	Video_FlipRect(rx, ry, rw, rh);
}

void Game_Draw()
{
	int playerFrame = (Timer_GetTicks() - player.animTimer) / 300;
	int robotFrame = Timer_GetTicks() / 300;
	int a;
	int b; // huh...

	Video_Copy(imgBackground[currentBackground], 0, 0);

	// Floors
	for(a = 0; a < floorsCount; a++)
	{
		int x = 0;
		int w = floors[a].size.w;
		int type = floors[a].type;
		for(x = 0; x < w; x += floorWidth)
		{
			Video_Copy(imgFloorTop[type], floors[a].pos.x + x, floors[a].pos.y - floorHeight);
			Video_Copy(imgFloor[type], floors[a].pos.x + x, floors[a].pos.y);
		}
	}

	// Walls
	for(a = 0; a < wallsCount; a++)
	{
		int y = 0;
		int h = walls[a].size.h;
		int type = walls[a].type;
		for(y = 0; y < h; y += wallHeight)
		{
			Video_Copy(imgWall[type], walls[a].pos.x, walls[a].pos.y + y);
		}
	}

	// Computers
	for(a = 0; a < computersCount; a++)
	{
		struct Image* img = (computers[a].remainingData > 0) ? imgComputer : imgComputerEmpty;
		Video_Copy(img, computers[a].pos.x, computers[a].pos.y);
	}

	// Teleport
	if(computersRemaining > 0)
	{
		Video_Copy(imgTeleport[0], posTeleport.x, posTeleport.y);
	}
	else
	{
		int x = Timer_GetTicks() / 200 * imgTeleport[0]->w % imgTeleport[1]->w;
		Video_CopyPart(imgTeleport[1], x, 0, imgTeleport[0]->w, imgTeleport[0]->h, posTeleport.x, posTeleport.y);
	}

	// Robots
	for(a = 0; a < robotsCount; a++)
	{
		struct Image* img = robots[a].dir < 0 ? imgRobot[0] : imgRobot[1];
		int sx = (robotFrame * robotWidth) % img->w;

		Video_DrawRect(robots[a].posLeft.x, robots[a].posLeft.y, 1, robotHeight, 3);
		Video_DrawRect(robots[a].posRight.x, robots[a].posRight.y, 1, robotHeight, 3);

		Video_CopyPart(img, sx, 0, robotWidth, robotHeight, robots[a].pos.x, robots[a].pos.y);
	}

	// Player
	// Video_Copy(imgPlayer[playerState], playerPos.x, playerPos.y);
	Video_CopyPart(player.imgs[player.state],
		(playerFrame * player.size.w) % player.imgs[player.state]->w, 0, player.size.w, player.size.h,
		player.pos.x, player.pos.y);

	// Harvesting HUD
	if(computerHarvested >= 0)
	{
		int totalLength = 20;
		int totalHeight = 6;

		int dstLen = totalLength -
			totalLength * computers[computerHarvested].remainingData / computers[computerHarvested].dataToHarvest;

		int cx = player.pos.x + player.size.w / 2 - totalLength / 2;
		int cy = player.pos.y - totalHeight;

		Video_DrawRect(cx - 1, cy - 1, totalLength + 2, totalHeight + 2, 0);
		Video_DrawRect(cx, cy, totalLength, totalHeight, 4);
		Video_DrawRect(cx, cy, dstLen, totalHeight, 3);
	}

	// HUD overlay
	Video_Copy(imgHudOverlay, 0, 0);

	// Remaining computers
	b = 0;
	for(a = 0; a < computersCount; a++)
	{
		if(computers[a].remainingData <= 0)
			continue;

		Video_Copy(imgHudFloppy, posHudFloppy.x + imgHudFloppy->w * b, posHudFloppy.y);
		b++;
	}
	// if(b == 0) // No more active computers
	if(computersRemaining <= 0)
	{
		Video_Copy(imgHudTeleport, posHudFloppy.x, posHudFloppy.y);
	}

	// Update the video memory
	if(needsFullRefresh)
		Video_Flip();
	else
	{
		Video_FlipRect(&player.pos, &player.posOld, player.size.w, player.size.h);
	}
}
