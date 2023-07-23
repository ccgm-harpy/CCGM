#pragma once

#include "pch-il2cpp.h"

class Win {
public:
	static float SpawnBombsTime;
	static float SpawnBombsInterval;
	static bool SpawnBombs;
	static bool roundEnded;
	static bool wonRoundOrGame;
	static void IncrementSpawnBombsTime(float dt);
	static void SendWin(float dt);
	static void Fireworks(float dt);
	static void SendWinMistake(float dt);
	static void GivePlayerWin(Player* targetPlayer);
	static bool sayWinMistake;
	static uint64_t winMistakePlayerId;
	static float sayWinMistakeTime;
	static bool modSentWinGame;
};