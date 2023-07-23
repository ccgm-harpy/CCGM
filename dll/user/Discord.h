#pragma once
#include "pch.h"
#include "Player.h"

class DiscordCommands {
public:
	static float requestTime;
	static void IncrementRequestTime(float dt);
	static void RequestCMDS();
	static void RequestBans();
};

class Discord {
public:
	static void ReportPlayerBan(Player* player, Player* targetPlayer, std::string banReason, std::string banTime, std::string discordUserName="", std::string discordUserId = "");
};