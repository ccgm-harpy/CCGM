#pragma once
#include "pch.h"
#include "httplib.h"
#include "Player.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;

class HTTP {
private:
	static void PostLog(std::string messageContent);
public:
	static httplib::Client client;
	static std::string GetBody(std::string endpoint);
	static httplib::Result PostData(json payload, std::string endpoint);
	static void ChatLog(Player* fromPlayer, std::string messageContent);
	static void KillCommandLog(Player* moderatorPlayer, Player* targetPlayer);
	static json LoadPlayerConfig(long long clientId);
	static json LoadBlockDropAnimation();
	static void ReportPlayer(Player* moderatorPlayer, Player* accusedPlayer, std::string reportReason);
	static void ModeratorWeaponCommandLog(Player* moderatorPlayer, Player* targetPlayer, std::string weaponName, bool gaveToSelf);
	static void ModeratorTeleportCommandLog(Player* moderatorPlayer, Player* targetPlayer);
	static void ModeratorRespawnCommandLog(Player* moderatorPlayer, Player* targetPlayer);
	static void ModeratorUnmuteCommandLog(Player* moderatorPlayer, Player* targetPlayer);
	static void ModeratorKickCommandLog(Player* moderatorPlayer, Player* targetPlayer);
	static void ModeratorMuteCommandLog(Player* moderatorPlayer, Player* targetPlayer);
	static void ModeratorPermCommandLog(Player* moderatorPlayer, Player* targetPlayer, std::string perm, bool removed = false);
	static json RequestDiscordCMDS();
	static json RequestDiscordBans();
	static void PostServerStatus(const std::string& status);
	static void PostMaxPlayers(int maxPlayers);
	static void PostLobbyReady(uint64_t lobbyCode, int maxPlayers);
	static void PostMapMode(int map, int mode);
	static void PostPlayersIn(int playerCount);
	static void PostPlayerKD(Player* killerPlayer, Player* deadPlayer);
	static void PostPlayerKD(Player* killerPlayer, Player* deadPlayer, std::string killMessage);
};