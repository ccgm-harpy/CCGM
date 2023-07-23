#pragma once

#include "pch.h"
#include "Injector.h"
#include "Player.h"

class Mod {
public:
	static std::string m_Version;
	static void Init();

	static void LoadMap(int mapID, int modeID);
	static float m_get_deltaTime();
	static void ExplodePlayer(long long clientId);
	static u10A4u109Bu10A5u109Eu10A3u10A3u109Bu10A2u109Du109Du10A2__Class* GetSomething();
	static void DropMoney(Player* targetPlayer);

	static void SendChatMessage(long long fromClient, std::string message);
	static void AppendLocalChatMessage(long long fromClient, std::string username, std::string message);
	static void SendDropItem(long long toClient, int objectId, int itemId, int ammo);
	static void ForceGiveItem(long long toClient, int objectId, int itemId);
	static void ForceGiveAllItem(int32_t itemId);
	static void ForceRemoveAllItem(int itemid);
	static void SendLocalInteract(int itemid);
	static void StartGame();
	static void SendInteract(long long clientId, int itemid);
	static void KillPlayer(long long clientId);
	static void BanPlayer(long long clientId);
	static void KickPlayer(long long clientId);
	static void RespawnPlayer(long long clientId, Vector3 position);
	static void RestartGame();
	static void SetCurrentGameModeTime(float time);
	static void SetBomber(long long clientId);
	static void TagPlayer(long long clientId);
	static void ToggleLights(bool on);
	static void GiveHat(long long clientId);
	static void SendWinner(long long clientId, long long money);
	static void SpectatePlayer(long long clientId);
	static void SendModeState(int32_t state);
	static void CreateLobby();
	static void Map_Remove(List_1_Map_* _this, Map* item);
	static int32_t Map_get_Count(List_1_Map_* map);
	static int32_t GameModeData_get_Count(List_1_GameModeData_* modeList);
	static bool Map_Contains(List_1_Map_* mapList, Map* mapItem);
	static bool GameModeData_Contains(List_1_GameModeData_* gameModeDataList, GameModeData* gameModeDataItem);
	static GameModeData* GameModeData_get_Item(List_1_GameModeData_* gameModeDataList, int32_t index);
	static String* GetAvailableModesString(void* _this);
	static void GameModeData_RemoveAt(int32_t index);
	static bool blockGetAvailableModesString;
	static bool blockMapContains;
	static bool blockGameModeDataContains;
	static bool blockGameModeDataGet;
	static bool blockGetEnumerator;
	static List_1_Map_* mapList;
	static List_1_GameModeData_* modeList;
	static std::vector<List_1_GameModeData_*> modeLists;
	static List_1_T_Enumerator_GameModeData_ GameModeData_GetEnumerator(List_1_GameModeData_* gameModeDataList, MethodInfo* method);
	static int buttonIndex;
	static std::chrono::system_clock::time_point m_LastUpdatedTime;
	static bool mouseDown;
	static bool mouseUp;
	static int clickButton;

	static void SendFallingBlock(float speed, int32_t coords, uint64_t steamId);
	static void SendFallingBlocks(float speed, int32_t coords);

	static u10A1u10A0u10A1u109Eu10A5u10A1u109Du10A8u10A5u1099u109A__Class* GetGameManager();
	static u10A0u10A4u10A8u10A1u10A8u109Au10A8u10A1u109Eu1099u109F__Class* GetSteamManager();
	static u109Du10A6u109Eu10A3u10A7u10A2u10A3u10A8u10A8u109Eu10A1__Class* GetChatBox();

	static std::string FormatStringVector(std::vector<std::string> vector) {

		std::string result = "";

		for (size_t i = 0; i < vector.size(); i++)
		{
			auto s = vector[i];
			result += s + ";";
		}

		return result;
	}

	static std::string FormatVector(Vector3 vector) {
		char str[256];
		sprintf_s(str, "%.3f, %.3f, %.3f", vector.x, vector.y, vector.z);
		return std::string(str);
	}

	/*
		static std::string FormatVector(Vector3 vector) {
		return std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z);
	*/

	static monoString* CreateMonoString(const char* str) {
		monoString* (*String_CreateString)(void* _this, const char* str) = (monoString * (*)(void*, const char*))(Injector::m_AssemblyBase + 8780720);
		return String_CreateString(NULL, str);
	}
};