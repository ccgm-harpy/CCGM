#include "pch-il2cpp.h"
#include "SnowBrawl.h"
#include "Server.h"
#include "Mod.h"

float SnowBrawl::SpawnSnowballsTime = 0;
bool SnowBrawl::CanSpawnSnowballs = false;

void SnowBrawl::IncrementSpawnSnowballsTime(float dt) {
	if (SnowBrawl::CanSpawnSnowballs) {
		SnowBrawl::SpawnSnowballsTime += dt;

		if (SnowBrawl::SpawnSnowballsTime >= Server::second * 4) {
			SnowBrawl::SpawnSnowballs();
		}
	}
	else {
		SnowBrawl::SpawnSnowballsTime = 0;
	}
}

void SnowBrawl::SpawnSnowballs() {
	SnowBrawl::SpawnSnowballsTime = 0;
	auto alivePlayers = PlayerUtils::FindPlayers("*a");

	Mod::ForceRemoveAllItem(9);
	Mod::ForceGiveAllItem(9);
}