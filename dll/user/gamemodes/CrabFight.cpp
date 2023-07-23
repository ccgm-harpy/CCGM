#include "pch-il2cpp.h"
#include "CrabFight.h"
#include "Chat.h"
#include "Server.h"
#include <nlohmann/json.hpp>
#include "Mod.h"
#include <string>

using namespace nlohmann;

bool CrabFight::bullyPlayer = false;
bool CrabFight::crabAngry = false;
Player* CrabFight::crabTarget = NULL;
bool CrabFight::increaseCrabHp = false;
float CrabFight::fakeHp = 100;
float CrabFight::fakeHpStart = 100;
float CrabFight::simulatedDamage = 3;
bool CrabFight::increaseDamage = false;
float CrabFight::damageMultiplier = 1.0;
float CrabFight::requiredHitsFake = 50.0;
float CrabFight::realHp = 100.0;
bool CrabFight::simulatedDamageSet = false;

float CrabFight::CalculateRealDamage() {
	return CrabFight::simulatedDamage;
}

float CrabFight::RandomizeHp() {
	json crabFightSettings = Server::ccgmConfig["gameModeSettings"]["crabFight"];
	int HPMin = crabFightSettings["randomHealthMin"];
	int HPMax = crabFightSettings["randomHealthMax"];
	float newHp = HPMin + (std::rand() % (HPMax - HPMin + 1));
	return newHp;
}

float CrabFight::RandomizeDamage() {
	json crabFightSettings = Server::ccgmConfig["gameModeSettings"]["crabFight"];
	float low = crabFightSettings["snowballDamageMultiplierMin"];
	float high = crabFightSettings["snowballDamageMultiplierMax"];
	float newDamage = low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low)));
	return newDamage;
}

void CrabFight::BullyChanceStart(int percentChance) {
	if (rand() % 100 + 1 <= percentChance) {
		CrabFight::BullyStart();
	}
}

void CrabFight::AngryChanceStart(int percentChance) {
	if (rand() % 100 + 1 <= percentChance) {
		CrabFight::AngryStart();
	}
}

void CrabFight::RandomizeHealthChanceStart(int percentChance) {
	if (rand() % 100 + 1 <= percentChance) {
		CrabFight::RandomizeHealthStart();
	}
}

void CrabFight::RandomizeHealthStart() {
	CrabFight::increaseCrabHp = true;
	CrabFight::fakeHpStart = CrabFight::RandomizeHp();
	CrabFight::fakeHp = CrabFight::fakeHpStart;
	std::string outputMessage = "!!! CRAB'S HEALTH IS " + std::to_string((int)CrabFight::fakeHp) + " (Normal 100) !!!";
	std::cout << outputMessage << std::endl;
	Chat::SendServerMessage(outputMessage);
}

void CrabFight::IncreaseDamageChanceStart(int percentChance) {
	int chanceResult = rand() % 100 + 1;
	std::cout << chanceResult << " <= " << percentChance << std::endl;
	if (chanceResult <= percentChance) {
		CrabFight::IncreaseDamageStart();
	}
}

void CrabFight::IncreaseDamageStart() {
	CrabFight::increaseDamage = true;
	CrabFight::damageMultiplier = CrabFight::RandomizeDamage();
	std::stringstream sDamageMultiplier;
	sDamageMultiplier << std::fixed << std::setprecision(2) << CrabFight::damageMultiplier;
	std::string outputMessage = "!!! SNOWBALL DAMAGE MULTIPLIER " + sDamageMultiplier.str() + " !!!";
	std::cout << outputMessage << std::endl;
	Chat::SendServerMessage(outputMessage);
}

void CrabFight::Stop() {
	CrabFight::bullyPlayer = false;
	CrabFight::crabAngry = false;
	CrabFight::increaseCrabHp = false;
	CrabFight::fakeHp = 100;
	CrabFight::fakeHpStart = 100;
	CrabFight::simulatedDamage = 3;
	CrabFight::increaseDamage = false;
	CrabFight::damageMultiplier = 1.0;
	CrabFight::requiredHitsFake = 50.0;
	CrabFight::simulatedDamageSet = false;
}

void CrabFight::SelectTarget() {
	auto alivePlayers = PlayerUtils::FindPlayers("*a");
	CrabFight::crabTarget = alivePlayers[rand() % alivePlayers.size()];
	Chat::SendServerMessage("!!! CRAB IS BULLYING " + CrabFight::crabTarget->GetSelector() + " !!!");
}

void CrabFight::AngryStart() {
	CrabFight::crabAngry = true;
}

void CrabFight::BullyStart() {
	CrabFight::bullyPlayer = true;

	CrabFight::SelectTarget();
}