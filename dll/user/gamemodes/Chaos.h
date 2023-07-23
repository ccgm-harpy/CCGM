#include "pch-il2cpp.h"

class Chaos {
public:
	static bool modeEnabled;
	static float givePointTime;
	static float eventTime;
	static float eventInterval;
	static void IncrementChaosTime(float dt);
	static void RandomEvent();
	static void WeaponEvent();
	static void SuperPunchEvent();
	static void JumpPunchEvent();
	static void ForceFieldEvent();
	static void GivePoint();
	static void ChanceStart(int percentChance);
	static void Start();
	static void Stop();
};