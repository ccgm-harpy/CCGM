#include "pch-il2cpp.h"
#include <string>
#include <map>
#include "Server.h"
#include <nlohmann/json.hpp>

using namespace nlohmann;

class CrabFight {
public:
	static float CalculateRealDamage();
	static void SelectTarget();
	static void BullyChanceStart(int percentChance);
	static void BullyStart();
	static void AngryChanceStart(int percentChance);
	static void AngryStart();
	static void RandomizeHealthChanceStart(int percentChance);
	static void RandomizeHealthStart();
	static void Stop();
	static float RandomizeHp();
	static float RandomizeDamage();
	static void IncreaseDamageChanceStart(int percentChance);
	static void	IncreaseDamageStart();
	static bool increaseDamage;
	static bool bullyPlayer;
	static bool crabAngry;
	static bool increaseCrabHp;
	static float fakeHp;
	static float simulatedDamage;
	static float fakeHpStart;
	static float damageMultiplier;
	static float requiredHitsFake;
	static float realHp;
	static bool simulatedDamageSet;
	static Player* crabTarget;
};