#include "pch.h"

static char selectedPNumCount = 0;
char randomPnum = 0;
static int pnumTimerRng = 0;
static char previousPnum = 0;

void Boss_SetNextPlayerToAttack(int timeLimit)
{
	if (GameState != 15 || !multiplayer::IsActive())
		return;

	if (pnumTimerRng <= timeLimit)
	{
		pnumTimerRng++;
	}
	else
	{
		randomPnum = rand() % multiplayer::GetPlayerCount();

		if (previousPnum == randomPnum)
		{
			selectedPNumCount++;
		}

		//if rng favorised the same player too many times, force a different player to get selected
		if (selectedPNumCount >= 2)
		{
			do
			{
				randomPnum = rand() % multiplayer::GetPlayerCount();
			} while (randomPnum == previousPnum);

			selectedPNumCount = 0;
		}

		previousPnum = randomPnum;
		pnumTimerRng = 0;
	}
}

void ResetBossRNG()
{
	selectedPNumCount = 0;
	randomPnum = 0;
	pnumTimerRng = 0;
	previousPnum = 0;
}