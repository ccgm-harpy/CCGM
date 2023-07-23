#include "pch-il2cpp.h"
#include "HTTP.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <string>
#include "Server.h"
#include <format>
#include "Player.h"
#include "Mod.h"

using namespace nlohmann;                              // Json library namespace
httplib::Client HTTP::client("http://127.0.0.1:5555"); // For communication with webserver

// Post a json payload to a webserver endpoint
httplib::Result HTTP::PostData(json payload, std::string endpoint) {
	auto res = HTTP::client.Post(endpoint.c_str(), payload.dump(), "application/json");
	return res;
}

// Returns the body of a httplib::Result object
std::string HTTP::GetBody(std::string endpoint) {
	return HTTP::client.Get(endpoint.c_str())->body;
}

// Loads a player's config by posting their SteamID to the webserver
json HTTP::LoadPlayerConfig(long long clientId) {
	json payload = {
		{"steamId", std::to_string(clientId)}
	};

	auto res = HTTP::PostData(payload, "/load_player_config");
	return json::parse(res->body);
}

// Requests a block drop animation from the webserver
json HTTP::LoadBlockDropAnimation() {
	json payload = {
		{"players", PlayerUtils::FindPlayers("*a").size()}
	};

	auto res = HTTP::PostData(payload, "/load_block_drop_animation");
	return json::parse(res->body);
}

// Post a log to the logging endpoint
void HTTP::PostLog(std::string messageContent) {
	json payload = {
		{
			Server::ccgmConfig["shortName"].get<std::string>(),
			messageContent
		}
	};

	HTTP::PostData(payload, "/chatLog");
}

// Log a chat message
void HTTP::ChatLog(Player* fromPlayer, std::string messageContent) {	
	HTTP::PostLog(
		"[CHAT " + std::to_string(fromPlayer->m_ClientId) + "]: " + messageContent
	);
}

// Log a kill used by the !kill command
void HTTP::KillCommandLog(Player* moderatorPlayer, Player* targetPlayer) {
	HTTP::PostLog(
		"[" + std::to_string(moderatorPlayer->m_ClientId) + "] KILLED WITH COMMAND " + std::to_string(targetPlayer->m_ClientId)
	);
}

// Reports a player to webserver
void HTTP::ReportPlayer(Player* moderatorPlayer, Player* accusedPlayer, std::string reportReason) {
	json reportPayload = {
		{"report", {
			{"name", moderatorPlayer->m_Username},
			{"number", moderatorPlayer->m_PlayerId},
			{"steamId", moderatorPlayer->m_ClientId},
			{"reportReason", reportReason},
			{"accusedName", accusedPlayer->m_Username},
			{"accusedNumber", accusedPlayer->m_PlayerId},
			{"accusedSteamId", accusedPlayer->m_ClientId},
			{"serverName", Server::ccgmConfig["shortName"]}
		}}
	};

	try {
		PostData(reportPayload, "/report_player");
	}
	catch (...) {
		reportPayload["report"]["name"] = "encode error";
		reportPayload["report"]["accusedName"] = "encode error";
		PostData(reportPayload, "/report_player");
	}
}

// Logs when a moderator spawns a weapon for free
void HTTP::ModeratorWeaponCommandLog(Player* moderatorPlayer, Player* targetPlayer, std::string weaponName, bool gaveToSelf) {
	std::string message = gaveToSelf ?
		"[" + std::to_string(moderatorPlayer->m_ClientId) + "] GAVE THEMSELF WEAPON '" + weaponName + "'" :
		"[" + std::to_string(moderatorPlayer->m_ClientId) + "] GAVE PLAYER " + std::to_string(targetPlayer->m_ClientId) + " WEAPON '" + weaponName + "'";

	PostLog(message);
}

// Logs when a moderator uses the teleport command
void HTTP::ModeratorTeleportCommandLog(Player* moderatorPlayer, Player* targetPlayer) {
	std::string messageContent = "[" + std::to_string(moderatorPlayer->m_ClientId) + "] TELEPORTED PLAYER " + std::to_string(targetPlayer->m_ClientId);
	PostLog(messageContent);
}

// Logs when a moderator uses the respawn command
void HTTP::ModeratorRespawnCommandLog(Player* moderatorPlayer, Player* targetPlayer) {
	std::string messageContent = "[" + std::to_string(moderatorPlayer->m_ClientId) + "] RESPAWNED PLAYER " + std::to_string(targetPlayer->m_ClientId);
	PostLog(messageContent);
}

// Logs when a moderator uses the unmute command
void HTTP::ModeratorUnmuteCommandLog(Player* moderatorPlayer, Player* targetPlayer) {
	std::string messageContent = "[" + std::to_string(moderatorPlayer->m_ClientId) + "] UNMUTED PLAYER " + std::to_string(targetPlayer->m_ClientId);
	PostLog(messageContent);
}

// Logs when a moderator uses the kick command
void HTTP::ModeratorKickCommandLog(Player* moderatorPlayer, Player* targetPlayer) {
	std::string messageContent = "[" + std::to_string(moderatorPlayer->m_ClientId) + "] KICKED PLAYER " + std::to_string(targetPlayer->m_ClientId);
	PostLog(messageContent);
}

// Logs when a moderator uses the mute command
void HTTP::ModeratorMuteCommandLog(Player* moderatorPlayer, Player* targetPlayer) {
	std::string messageContent = "[" + std::to_string(moderatorPlayer->m_ClientId) + "] MUTED PLAYER " + std::to_string(targetPlayer->m_ClientId);
	PostLog(messageContent);
}

// Logs when an administrator uses the perm command
void HTTP::ModeratorPermCommandLog(Player* moderatorPlayer, Player* targetPlayer, std::string perm, bool removed) {
	std::string action = removed ? "REMOVED" : "GAVE";
	std::string messageContent = "[" + std::to_string(moderatorPlayer->m_ClientId) + "] " + action + " PERM '" + perm + "' TO " + std::to_string(targetPlayer->m_ClientId);
	PostLog(messageContent);
}

// Requests pending Discord commands from the webserver
json HTTP::RequestDiscordCMDS() {
	std::string dest = "/do_commands_";
	dest.append(Server::ccgmConfig["shortName"].get<std::string>());
	std::string response = GetBody(dest);
	return json::parse(response);
}

// Requests pending Discord bans from the webserver
json HTTP::RequestDiscordBans() {
	std::string shortName = Server::ccgmConfig["shortName"].get<std::string>();

	json payload = {
		{"shortName", shortName}
	};

	std::string response = PostData(payload, "/remote_bans")->body;
	return json::parse(response);
}

// Tells the webserver that status of our server
// (waiting, injecting, started etc...)
void HTTP::PostServerStatus(const std::string& status) {
	json payload = {
		{Server::activeServerName, {{"status", status}}}
	};

	std::string endpoint = "/server_status";

	HTTP::PostData(payload, endpoint);
}

// Tells the webserver the max players we're configured for
void HTTP::PostMaxPlayers(int maxPlayers) {
	json payload = {
		{Server::ccgmConfig["shortName"].get<std::string>(), {
			{"maxPlayers", maxPlayers}
		}}
	};

	std::string endpoint = "/server_status";

	HTTP::PostData(payload, endpoint);
}

// Tells the webserver multiple things about about our lobby
// This is only called once the lobby is created and ready
void HTTP::PostLobbyReady(uint64_t lobbyCode, int maxPlayers) {
	json payload = {
		{Server::ccgmConfig["shortName"].get<std::string>(), {
			{"lobbyCode", lobbyCode},
			{"maxPlayers", maxPlayers},
			{"status", "Online"},
			{"map", 6},
			{"mode", 0}
		}}
	};

	std::string endpoint = "/server_status";

	HTTP::PostData(payload, endpoint);
}

// Tells the webserver what map and mode we're on
void HTTP::PostMapMode(int map, int mode) {
	json payload = {
		{Server::ccgmConfig["shortName"].get<std::string>(), {
			{"map", map},
			{"mode", mode}
		}}
	};

	std::string endpoint = "/server_status";

	HTTP::PostData(payload, endpoint);
}

// Tells the webserver how many players are on the server
void HTTP::PostPlayersIn(int playerCount) {
	json payload = {
		{Server::ccgmConfig["shortName"].get<std::string>(), {
			{"players", playerCount}
		}}
	};

	std::string endpoint = "/server_status";

	HTTP::PostData(payload, endpoint);
}

// Tells the webserver about a player's KD updating
void HTTP::PostPlayerKD(Player* killerPlayer, Player* deadPlayer, std::string killMessage) {
	std::string shortName = Server::ccgmConfig["shortName"].get<std::string>();

	int kills = killerPlayer->m_PlayerConfig["kills"][shortName];
	int deaths = deadPlayer->m_PlayerConfig["deaths"][shortName];
	kills += 1;
	deaths += 1;

	killerPlayer->m_PlayerConfig["kills"][shortName] = kills;
	deadPlayer->m_PlayerConfig["deaths"][shortName] = deaths;

	HTTP::PostData(killerPlayer->m_PlayerConfig, "/save_player_config");
	HTTP::PostData(deadPlayer->m_PlayerConfig, "/save_player_config");
}
