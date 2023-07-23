#pragma once

#include "pch.h"
#include <nlohmann/json.hpp>

using namespace nlohmann;

class Player {
public:
	long long m_ClientId;
	int m_PlayerId;
	std::string m_Username = "";
	int recievedFallingBlock = 0;
	json m_PlayerConfig;
	bool m_IsAlive = true;
	float allowMessageTime = 0;
	bool m_HideMessages = false;
	bool m_JumpPunchEnabled = false;
	bool m_SuperPunchEnabled = false;
	bool m_ForceFieldEnabled = false;
	bool m_GodEnabled = false;
	bool m_CanReport = true;
	float m_ReportTime = 0;
	Vector3 m_Position = Vector3({ 0, 0, 0 });
	std::vector<std::string> m_Permissions;

	Player(long long clientId, int playerId);
	bool HasPermission(std::string permission);
	void AddPermission(std::string permission);
	void RemovePermission(std::string permission);
	bool IsLobbyOwner();
	std::string GetDisplayName();
	std::string GetSelector();
	bool IsMuted();
	bool IsBanned();
	void Ban(Player* moderatorPlayer, std::string banReason, uint64_t banTimeSeconds, std::string discordUserName="", std::string discordUserId="");
	void SaveConfig();
};

class PlayerHooks {
public:
	static void AddedToLobby(long long clientId);
	static void TryAddPlayer(long long clientId, int playerId, u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C* playerManager);
	static void OnPlayerRemovedFromLobby(long long clientId);
	
};

class PlayerUtils {
public:
	static Vector3 m_SpawnPosition;
	static bool m_CanUpdateSpawnPosition;
	static std::map<long long, Player*> playerList;

	static void ClearAllPlayerPowers();
	static Player* GetPlayer(long long clientId);
	static bool HasPlayer(long long clientId);
	static void AddPlayer(Player* player);
	static void AssignConfigPerms(Player* player);
	static Player* LoadPlayerConfig(long long clientId, int playerId);
	static void IncrementReportTime(float dt);
	static void UpdatePlayerPositions();
	static std::vector<Player*> FindPlayers(std::string selector);
	static uint64_t SecondsFromString(const std::string& time);
	static std::string FormatSeconds(uint64_t total_seconds);
	static void UpdatePlayerMessageTimers(float dt);
};