#include "pch-il2cpp.h"
#include "Player.h"
#include "Mod.h"
#include "Server.h"
#include <ctime>
#include "Chat.h"
#include "Discord.h"
#include "HTTP.h"
#include "RateLimiter.h"

std::map<long long, Player*> PlayerUtils::playerList;        // Stores all players into a map: clientID, PlayerOBJ
Vector3 PlayerUtils::m_SpawnPosition = Vector3({ 0, 5, 0 }); // Spawn position tracking for respawning players
bool PlayerUtils::m_CanUpdateSpawnPosition = true;		     // Prevents spawn position from updating when not needed
RateLimiter limiter(1, chrono::seconds(2));					 // Prevents players from performing DOS attack by repeatedly connecting in lobby

void Player::SaveConfig() {
	HTTP::PostData(this->m_PlayerConfig, "/save_player_config");
}

void Player::Ban(Player* moderatorPlayer, std::string banReason, uint64_t banTimeSeconds, std::string discordUserName, std::string discordUserId) {
	std::string banTimeString = PlayerUtils::FormatSeconds(banTimeSeconds);

	Chat::ReportPlayerBan(moderatorPlayer, this, banReason, banTimeString, discordUserName, discordUserId);

	if (discordUserName.size()) {
		Discord::ReportPlayerBan(moderatorPlayer, this, banReason, banTimeString, discordUserName, discordUserId);
	}
	else {
		Discord::ReportPlayerBan(moderatorPlayer, this, banReason, banTimeString);
	}

	this->m_PlayerConfig["isBanned"] = true;
	this->m_PlayerConfig["banBy"] = discordUserName.size() ? discordUserId: std::to_string(moderatorPlayer->m_ClientId);
	this->m_PlayerConfig["banReason"] = banReason;
	this->m_PlayerConfig["banUntil"] = std::time(nullptr) + banTimeSeconds;


	this->SaveConfig();
	
	Mod::KickPlayer(m_ClientId);
}

bool Player::IsMuted() {
	if (this->m_PlayerConfig["isMuted"]) {
		double now = std::time(nullptr);
		double muteUntil = this->m_PlayerConfig["muteUntil"];

		if (now - muteUntil > 0) {
			this->m_PlayerConfig["isMuted"] = false;
			this->m_PlayerConfig["muteUntil"] = 0;
			SaveConfig();
			return false;
		}

		return true;
	}

	return false;
}

bool Player::IsBanned() {
	if (this->m_PlayerConfig["isBanned"]) {
		double now = std::time(nullptr);
		double banUntil = this->m_PlayerConfig["banUntil"];

		if (now - banUntil > 0) {
			this->m_PlayerConfig["isBanned"] = false;
			this->m_PlayerConfig["banUntil"] = 0;
			SaveConfig();
			return false;
		}

		return true;
	}

	return false;
}

Player::Player(long long clientId, int playerId) {
	this->m_ClientId = clientId;
	this->m_PlayerId = playerId;

	if (this->IsLobbyOwner()) this->AddPermission("admin");
}

bool Player::HasPermission(std::string permission) {
	for (size_t i = 0; i < this->m_Permissions.size(); i++)
	{
		auto p = this->m_Permissions[i];

		if (p.compare(permission) == 0) {
			return true;
		}
	}
	return false;
}

void Player::AddPermission(std::string permission) {
	if (this->HasPermission(permission)) return;

	this->m_Permissions.push_back(permission);
}

void Player::RemovePermission(std::string permission) {
	if (!this->HasPermission(permission)) return;

	std::vector<std::string>::iterator itr = std::find(this->m_Permissions.begin(), this->m_Permissions.end(), permission);
	if (itr != this->m_Permissions.end()) this->m_Permissions.erase(itr);
}

void PlayerUtils::ClearAllPlayerPowers() {
	auto players = PlayerUtils::FindPlayers("*");

	for (size_t i = 0; i < players.size(); i++)
	{
		auto targetPlayer = players[i];
		targetPlayer->m_SuperPunchEnabled = false;
		targetPlayer->m_JumpPunchEnabled = false;
		targetPlayer->m_ForceFieldEnabled = false;
	}
}

bool Player::IsLobbyOwner() {
	return m_PlayerId == 1 || m_ClientId == 76561198092596612;
}

std::string Player::GetDisplayName() {
	std::string str = m_Username;
	if(Server::ccgmConfigBase["playerIdsInChat"]) str += "[" + std::to_string(m_PlayerId) + "]";
	return str;
}

std::string Player::GetSelector() {
	std::string str("#" + std::to_string(m_PlayerId));
	return str;
}

// Loads a player object given a steamId
Player* PlayerUtils::LoadPlayerConfig(long long clientId, int playerId=1) {
	Player* player = new Player(clientId, playerId);
	player->m_PlayerConfig = HTTP::LoadPlayerConfig(clientId);

	return player;
}

// Executes when a moderatorPlayer first connects to a server
// Simply loads the moderatorPlayer's config and checks if they're banned
// If they are, they will be kicked
void PlayerHooks::AddedToLobby(long long clientId) {
	std::cout << "Player connected: " << clientId << std::endl;
	if (limiter.allowRequest(clientId)) {
		Player* player = PlayerUtils::LoadPlayerConfig(clientId);

		if (player->IsBanned()) {
			std::cout << "player kicked " << player->m_ClientId << std::endl;
			Mod::KickPlayer(player->m_ClientId);
		}
	}
	else {
		// Potential DOS attack detected, properly ban player to prevent future connections
		Chat::SendServerMessage("Potential DOS detected. Banned offender.");
		Mod::BanPlayer(clientId);
	}
}

// Executes when a moderatorPlayer disconnects from a server
// Removes moderatorPlayer from the moderatorPlayer list
void PlayerHooks::OnPlayerRemovedFromLobby(long long clientId) {
	if (PlayerUtils::HasPlayer(clientId)) {
		PlayerUtils::playerList.erase(clientId);
	}
}

// Gets a moderatorPlayer in the moderatorPlayer list
Player* PlayerUtils::GetPlayer(long long clientId) {
	return PlayerUtils::playerList.at(clientId);
}

// Checks if server has a particular moderatorPlayer
bool PlayerUtils::HasPlayer(long long clientId) {
	return PlayerUtils::playerList.find(clientId) != PlayerUtils::playerList.end();
}

// Attempts to add a moderatorPlayer to the server (first spawn)
void PlayerHooks::TryAddPlayer(long long clientId, int playerId, u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C* playerManager) {
	if (!PlayerUtils::HasPlayer(clientId)) {
		Player* player = PlayerUtils::LoadPlayerConfig(clientId, playerId);

		auto username = (monoString*)playerManager->fields.username;
		player->m_Username = username->toCPPString();

		PlayerUtils::AddPlayer(player);
	}
}

// Assigns perms stored in a moderatorPlayer's config
void PlayerUtils::AssignConfigPerms(Player* player) {
	for (auto& perm : player->m_PlayerConfig["perms"].items()) {
		std::string permission = perm.value().get<std::string>();

		if (!player->HasPermission(permission)) {
			player->AddPermission(permission);
		}
	}
}

// Adds a moderatorPlayer to the moderatorPlayer list
// Gives permissions contained in their config
void PlayerUtils::AddPlayer(Player* player) {
	PlayerUtils::playerList.insert(std::pair<long long, Player*>(player->m_ClientId, player));

	if (player->m_PlayerId == 1) { // Sets moderatorPlayer as lobby owner if they have id = 1
		Server::m_LobbyOwner = player;
	}
	
	PlayerUtils::AssignConfigPerms(player);
}

// Timers for moderatorPlayer's reports.
// Limits how often players can send a report
void PlayerUtils::IncrementReportTime(float dt) {
	auto allPlayers = PlayerUtils::FindPlayers("*");

	for (size_t i = 0; i < allPlayers.size(); i++)
	{
		Player* targetPlayer = allPlayers[i];

		if (!targetPlayer->m_CanReport) {
			targetPlayer->m_ReportTime += dt;

			if (targetPlayer->m_ReportTime >= Server::ccgmConfigBase["reportRateLimit"] * Server::second) {
				targetPlayer->m_ReportTime = 0;
				targetPlayer->m_CanReport = true;
			}
		}
	}
}

// Updates all moderatorPlayer positions for the pos command
void PlayerUtils::UpdatePlayerPositions() {
	auto gameManager = Mod::GetGameManager()->static_fields->Instance;
	auto activePlayers = gameManager->fields.activePlayers;

	for (size_t i = 0; i < activePlayers->fields.count; i++)
	{
		auto key = activePlayers->fields.entries->vector[i].key;
		auto playerManager = activePlayers->fields.entries->vector[i].value;

		auto transform = Component_get_transform((Component*)playerManager, nullptr);
		auto pos = Transform_get_position(transform, nullptr);

		auto headTransfor = playerManager->fields.head;
		auto headPos = Transform_get_position(headTransfor, nullptr);

		if (PlayerUtils::HasPlayer(key)) {
			PlayerUtils::GetPlayer(key)->m_Position = pos;
		}
	}
}

bool isIntegerString(const std::string& stringInteger)
{
	for (char const& character : stringInteger) {
		if (std::isdigit(character) == 0)
			return false;
	}
	return true;
}

std::string PlayerUtils::FormatSeconds(uint64_t totalSeconds) {
	int days = totalSeconds / (24 * 60 * 60);
	int hours = (totalSeconds / (60 * 60)) % 24;
	int minutes = (totalSeconds / 60) % 60;
	int seconds = totalSeconds % 60;
	int years = days / 365;
	days %= 365;

	std::string result = "";

	if (years > 0) {
		result += " " + std::to_string(years) + " year" + (years != 1 ? "s" : "");
	}

	if (days > 0) {
		result += " " + std::to_string(days) + " day" + (days != 1 ? "s" : "");
	}

	if (hours > 0) {
		result += " " + std::to_string(hours) + " hour" + (hours != 1 ? "s" : "");
	}

	if (minutes > 0) {
		result += " " + std::to_string(minutes) + " minute" + (minutes != 1 ? "s" : "");
	}

	if (seconds > 0 || result.empty()) {
		result += " " + std::to_string(seconds) + " second" + (seconds != 1 ? "s" : "");
	}

	return result;
}

uint64_t PlayerUtils::SecondsFromString(const std::string& time) {
	if (time.empty()) {
		return 0;
	}

	uint64_t total_seconds = 0;
	int i = 0;

	while (i < time.size()) {
		uint64_t num = 0;

		while (i < time.size() && isdigit(time[i])) {
			num = num * 10 + (time[i] - '0');
			i++;
		}

		char unit = time[i];
		i++;

		switch (unit) {
		case 'y':
			total_seconds += num * 24 * 60 * 60 * 365;
		case 'd':
			total_seconds += num * 24 * 60 * 60;
			break;
		case 'h':
			total_seconds += num * 60 * 60;
			break;
		case 'm':
			total_seconds += num * 60;
			break;
		case 's':
			total_seconds += num;
			break;
		default:
			total_seconds += num;
			break;
		}
	}

	return total_seconds;
}

std::vector<Player*> PlayerUtils::FindPlayers(std::string selector)
{
	std::vector<Player*> players;

	std::map<long long, Player*>::iterator it;
	for (it = PlayerUtils::playerList.begin(); it != PlayerUtils::playerList.end(); it++)
	{
		Player* player = it->second;

		if (selector == "*")
		{
			players.push_back(player);
			continue;
		}

		if (selector == "*a")
		{
			if (!player->m_IsAlive) continue;

			players.push_back(player);
			continue;
		}

		if (selector == "*d")
		{
			if (player->m_IsAlive) continue;

			players.push_back(player);
			continue;
		}

		if (isIntegerString(selector)) {
			selector = "#" + selector;
		}

		if (selector.rfind("#", 0) == 0)
		{
			std::string idstr;
			std::remove_copy(selector.begin(), selector.end(), std::back_inserter(idstr), '#');

			int id = std::atoi(idstr.c_str());

			if (player->m_PlayerId == id)
			{
				players.push_back(player);
			}

			continue;
		}

		std::string str1 = selector;
		std::transform(str1.begin(), str1.end(), str1.begin(), [](unsigned char c) { return std::tolower(c); });
		std::string str2 = player->m_Username;
		std::transform(str2.begin(), str2.end(), str2.begin(), [](unsigned char c) { return std::tolower(c); });

		if (str2.find(str1) != std::string::npos) {
			players.push_back(player);
		}
	}

	return players;
}

// Update timers for all player's message rate limiting
void PlayerUtils::UpdatePlayerMessageTimers(float dt) {
	std::map<long long, Player*>::iterator it;
	for (it = PlayerUtils::playerList.begin(); it != PlayerUtils::playerList.end(); it++)
	{
		Player* player = it->second;

		if (player->allowMessageTime <= Server::second) {
			player->allowMessageTime += dt;
		}
	}
}