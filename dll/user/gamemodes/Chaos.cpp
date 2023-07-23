#include "pch-il2cpp.h"
#include "Chaos.h"
#include "Server.h"
#include "weapons.h"
#include "Chat.h"
#include "Server.h"

bool Chaos::modeEnabled = false;
float Chaos::givePointTime = 0;
float Chaos::eventTime;

float Chaos::eventInterval = 12000;

void Chaos::ChanceStart(int percentChance) {
	if (rand() % 100 + 1 <= percentChance) {
		Chaos::Start();
	}
	else {
		Chaos::Stop();
	}
}

void Chaos::Start() {
	Chaos::modeEnabled = true;
	Chaos::givePointTime = 0;
	Chaos::eventTime;
	Chaos::eventInterval = 12000;
}

void Chaos::Stop() {
	Chaos::modeEnabled = false;
	Chaos::givePointTime = 0;
	Chaos::eventTime;
	Chaos::eventInterval = 12000;
	PlayerUtils::ClearAllPlayerPowers();
}

void Chaos::IncrementChaosTime(float dt) {
	if (Chaos::modeEnabled) {
		Chaos::givePointTime += dt;
		Chaos::eventTime += dt;

		if (Chaos::eventTime >= Chaos::eventInterval) {
			Chaos::eventTime = 1;
			Chaos::eventInterval = Server::second * 30;
			PlayerUtils::ClearAllPlayerPowers();
			Chaos::RandomEvent();
		}

		if (Chaos::givePointTime >= Server::minute * 3) {
			Chaos::givePointTime = 0;
			Chaos::GivePoint();
		}
	}
}

void Chaos::RandomEvent() {
	int eventChoice = rand() % 7 + 1;

	if (eventChoice <= 3) {
		Chaos::WeaponEvent();
	}
	else if (eventChoice == 4) {
		Chaos::SuperPunchEvent();
	}
	else if (eventChoice == 5 || eventChoice == 6) {
		Chaos::JumpPunchEvent();
	}
	else if (eventChoice == 7) {
		Chaos::ForceFieldEvent();
	}
}

void Chaos::WeaponEvent() {
	auto alivePlayers = PlayerUtils::FindPlayers("*a");
	Weapon weapon = Weapons::SelectRandomWeapon();
	
	Chat::SendServerMessage("[RANDOM EVENT]");

	if (weapon.ammo > 1) {
		Chat::SendServerMessage("[DROPPED " + Chat::Capitalize(weapon.name) + "S " + std::to_string(weapon.ammo) + " AMMO]");
	}
	else {
		Chat::SendServerMessage("[DROPPED " + Chat::Capitalize(weapon.name) + "S]");
	}

	if (!weapon.tip.empty()) {
		Chat::SendServerMessage("(" + weapon.tip + ")");
	}

	for (size_t i = 0; i < alivePlayers.size(); i++)
	{
		auto targetPlayer = alivePlayers[i];
		
		if (Server::ccgmConfigBase["chaosWeaponEventToInventory"]) {
			Weapons::GiveWeapon(targetPlayer->m_ClientId, weapon.id);
		}
		else {
			Weapons::DropWeapon(targetPlayer->m_ClientId, weapon.id);
		}
	}
}

void Chaos::SuperPunchEvent() {
	auto alivePlayers = PlayerUtils::FindPlayers("*a");

	for (size_t i = 0; i < alivePlayers.size(); i++)
	{
		auto targetPlayer = alivePlayers[i];
		targetPlayer->m_SuperPunchEnabled = true;
	}

	Chat::SendServerMessage("[RANDOM EVENT]");
	Chat::SendServerMessage("[PLAYERS GIVEN SUPERPUNCH 30sec]");
	Chat::SendServerMessage("(Smack players to the moon)");
}

void Chaos::JumpPunchEvent() {
	auto alivePlayers = PlayerUtils::FindPlayers("*a");

	for (size_t i = 0; i < alivePlayers.size(); i++)
	{
		auto targetPlayer = alivePlayers[i];
		targetPlayer->m_JumpPunchEnabled = true;
	}

	Chat::SendServerMessage("[RANDOM EVENT]");
	Chat::SendServerMessage("[PLAYERS GIVEN JUMPPUNCH] 30sec");
	Chat::SendServerMessage("(Punch with hands to boost jump)");
}

void Chaos::ForceFieldEvent() {
	auto alivePlayers = PlayerUtils::FindPlayers("*a");

	for (size_t i = 0; i < alivePlayers.size(); i++)
	{
		auto targetPlayer = alivePlayers[i];
		targetPlayer->m_ForceFieldEnabled = true;
	}

	Chat::SendServerMessage("[RANDOM EVENT]");
	Chat::SendServerMessage("[PLAYERS GIVEN FORCEFIELD 30sec]");
	Chat::SendServerMessage("(Players cannot punch eachother)");
}

void Chaos::GivePoint() {
	auto allPlayers = PlayerUtils::FindPlayers("*");

	for (size_t i = 0; i < allPlayers.size(); i++)
	{
		auto targetPlayer = allPlayers[i];

		if (targetPlayer->m_PlayerConfig.contains("steamId")) {
			int points = targetPlayer->m_PlayerConfig["points"];

			if (points + 1 < 15) {
				points += 1;
			}
			else {
				points = 15;
			}

			targetPlayer->m_PlayerConfig["points"] = points;
			targetPlayer->SaveConfig();
		}
	}

	Chat::SendServerMessage("[PLAYERS GIVEN 1 POINT]");
	Chat::SendServerMessage("Say !points to see how many you have!");
}