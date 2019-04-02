#include "AI.h"

int initailAstroids = 0;
int AiShipType = 1;
int PlayerShipType = 1;

void DoAI(const Ship& player, Ship& AI)
{
	AI.rightCommand = false;
	AI.leftCommand  = true;
	AI.accelCommand = true;
	AI.shootCommand = true;
}
