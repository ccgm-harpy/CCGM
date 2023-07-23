#include "pch-il2cpp.h"
#include "gamemodes/Chaos.h"
#include "Server.h"
#include "Win.h"
#include "Chat.h"
#include "weapons.h"
#include "Mod.h"

float Win::SpawnBombsTime = 0;
float Win::SpawnBombsInterval = 0;
bool Win::roundEnded = false;
bool Win::wonRoundOrGame = false;
bool Win::SpawnBombs = false;

bool Win::modSentWinGame = false;
bool Win::sayWinMistake = false;
float Win::sayWinMistakeTime = 0;
uint64_t Win::winMistakePlayerId = 0;

void Win::IncrementSpawnBombsTime(float dt) {
	if (SpawnBombs) {
		SpawnBombsTime += dt;
	}
}

void Win::Fireworks(float dt) {
	if (SpawnBombs && SpawnBombsTime >= 50) {
		SpawnBombsInterval += dt;
		SpawnBombsTime = 0;

		if (SpawnBombsInterval >= 1000) {
			SpawnBombsInterval = 0;
			SpawnBombs = false;
		}

		auto alivePlayers = PlayerUtils::FindPlayers("*a");
		auto allPlayers = PlayerUtils::FindPlayers("*");

		if (alivePlayers.size() == 1 && allPlayers.size() > 1) {
			auto targetPlayer = alivePlayers[0];
			
			for (int i = 0; i < 3; i++) {
				Weapons::DropWeapon(targetPlayer->m_ClientId, 5);
				Weapons::DropWeapon(targetPlayer->m_ClientId, 12);
				//Mod::DropMoney(targetPlayer);
			}
		}
	}
}

void Win::GivePlayerWin(Player* targetPlayer) {
	std::string requiredPlayersString = std::to_string(Server::ccgmConfigBase["winsPlayerRequirement"].get<int>());
	if (Server::gameStartPlayerCount >= Server::ccgmConfigBase["winsPlayerRequirement"]) {
		std::string shortName = Server::ccgmConfig["shortName"];
		int points = targetPlayer->m_PlayerConfig["points"];
		int wins = targetPlayer->m_PlayerConfig["wins"][shortName];
		wins += 1;

		if (Chaos::modeEnabled) {
			Chat::SendServerMessage("PLAYER #" + std::to_string(targetPlayer->m_PlayerId) + " WON 5 POINTS FOR BEING LAST STANDING");

			if (points + 5 < 15) {
				points += 5;
			}
			else {
				points = 15;
			}

			targetPlayer->m_PlayerConfig["points"] = points;
		}

		targetPlayer->m_PlayerConfig["wins"][shortName] = wins;

		SpawnBombs = true;

		targetPlayer->SaveConfig();
		Chat::SendServerMessage("PLAYER #" + std::to_string(targetPlayer->m_PlayerId) + " HAS " + std::to_string(wins) + " WINS");

		if (Win::sayWinMistake) {
			Chat::SendServerMessage("Sorry I missed your win!");
		}
	}
	else {
		Chat::SendServerMessage("Need at least " + requiredPlayersString + " players to count wins");
	}
}

void Win::SendWinMistake(float dt) {
	if (Win::sayWinMistake) {
		Win::sayWinMistakeTime += dt;

		if (Win::sayWinMistakeTime >= Server::second * 4) {
			auto targetPlayer = PlayerUtils::GetPlayer(Win::winMistakePlayerId);
			Win::GivePlayerWin(targetPlayer);
			Win::sayWinMistakeTime = 0;
			Win::sayWinMistake = false;
		}
	}
}

void Win::SendWin(float dt) {
	IncrementSpawnBombsTime(dt);
	Fireworks(dt);

	auto alivePlayers = PlayerUtils::FindPlayers("*a");

	if (alivePlayers.size() == 1 && Server::gameStartPlayerCount > 1 && Win::roundEnded) {
		Win::roundEnded = false;
		Win::wonRoundOrGame = true;
		auto targetPlayer = alivePlayers[0];
		Win::GivePlayerWin(targetPlayer);
	}
}