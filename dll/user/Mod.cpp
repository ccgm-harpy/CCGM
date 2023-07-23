#include "pch-il2cpp.h"

#include "Mod.h"
#include "Injector.h"
#include "SocketServer.h"
#include "Server.h"
#include "templates.h"
#include <random>
#include <nlohmann/json.hpp>
#include "Player.h"
#include "HTTP.h"
#undef max

using namespace nlohmann;

std::string Mod::m_Version = "1.3.1";

List_1_Map_* Mod::mapList;
List_1_GameModeData_* Mod::modeList;
bool Mod::blockMapContains = false;
bool Mod::blockGameModeDataContains = false;
bool Mod::blockGameModeDataGet = false;
bool Mod::blockGetAvailableModesString = false;
bool Mod::blockGetEnumerator = false;
std::vector<List_1_GameModeData_*> Mod::modeLists;
int Mod::buttonIndex = -1;
std::chrono::system_clock::time_point Mod::m_LastUpdatedTime;
bool Mod::mouseDown = false;
bool Mod::mouseUp = false;
int Mod::clickButton = 0;

void Mod::Init() {
	std::cout << "[Mod] Init v" << m_Version << std::endl;
	Server::activeServerName = HTTP::GetBody("/active_server_name");
	HTTP::PostServerStatus("Injected");
	Server::ccgmConfigBase = json::parse(HTTP::GetBody("/ccgm_config"));
	Server::ccgmConfig = Server::ccgmConfigBase["servers"][Server::activeServerName];
	Chaos::modeEnabled = Server::ccgmConfig["gameModeSettings"]["chaos"]["enabled"];
	std::string shortName = Server::ccgmConfig["shortName"];
	HTTP::PostMaxPlayers(Server::ccgmConfig["maxPlayers"]);

	Injector::Init();

	std::cout << "checking for compatibilityMode" << std::endl;

	if (Server::ccgmConfigBase["compatibilityMode"]) {
		std::cout << "compatibilityMode active" << std::endl;
		Injector::Inject(HF_Component_GetComponent_4, Component_GetComponent_4, &Template_HF_Component_GetComponent_4);
		Injector::Inject(Matchmaking_CreateLobby, SteamMatchmaking_CreateLobby, &Template_Matchmaking_CreateLobby);
	}
	else {
		Injector::Inject(HF_DropMoney, DropMoney, &Template_DropMoney);
		Injector::Inject(HF_VoiceChatOnOrOff, VoiceChatOnOrOff, &Template_VoiceChatOnOrOff);
		Injector::Inject(HF_SteamMatchmaking_SetLobbyType, SteamMatchmaking_SetLobbyType, &Template_SteamMatchmaking_SetLobbyType);
		Injector::Inject(ServerSend_ForceGiveItem, u10A8u1099u1099u1099u10A4u10A4u10A8u109Du109Au10A4u10A5_ForceGiveItem, &Template_ServerSend_ForceGiveItem);
		Injector::Inject(ServerSend_ForceGiveWeapon, u109Du10A8u10A4u109Bu1099u109Du10A2u10A1u10A3u10A0u10A3_ForceGiveWeapon, &Template_ServerSend_ForceGiveWeapon);
		Injector::Inject(HF_FindCrabDamage, u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5_FindCrabDamage, &Template_HF_FindCrabDamage);
		Injector::Inject(ChatBox_AddMessage, u10A0u10A5u10A7u10A0u1099u10A8u109Du10A6u10A0u10A8u10A7_AddMessage, &Template_ChatBox_AddMessage);
		Injector::Inject(HF_CrabDamage, u1099u10A5u109Fu10A8u1099u10A4u10A4u109Du109Du10A1u10A0_CrabDamage, &Template_HF_CrabDamage);
		Injector::Inject(HF_CrabHp, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_CrabHp, &Template_HF_CrabHp);
		Injector::Inject(HF_SpawnCrab, u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5_SpawnCrab, &Template_HF_SpawnCrab);
		Injector::Inject(HF_SpawnCrabSpikes, u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5_SpawnCrabSpikes, &Template_HF_SpawnCrabSpikes);
		Injector::Inject(HF_CrabDifficulty, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_CrabDifficulty, &Template_HF_CrabDifficulty);
		Injector::Inject(HF_SendCrabBall, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SendCrabBall, &Template_HF_SendCrabBall);
		Injector::Inject(HF_CrabTarget, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_CrabTarget, &Template_HF_CrabTarget);
		Injector::Inject(HF_ExplodePlayer, u10A4u109Bu10A5u109Eu10A3u10A3u109Bu10A2u109Du109Du10A2_u109Fu109Eu109Au10A0u109Fu10A8u109Du109Eu1099u109Bu10A0, &Template_HF_ExplodePlayer);
		Injector::Inject(Server_SendWinner, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SendWinner, &Template_Server_SendWinner);
		Injector::Inject(HF_Component_GetComponent_4, Component_GetComponent_4, &Template_HF_Component_GetComponent_4);

		Injector::Inject(HF_NativeMethods_ISteamMatchmaking_SetLobbyData, NativeMethods_ISteamMatchmaking_SetLobbyData, &Template_NativeMethods_ISteamMatchmaking_SetLobbyData);
		Injector::Inject(Matchmaking_SetLobbyMemberLimit, SteamMatchmaking_SetLobbyMemberLimit, &Template_Matchmaking_SetLobbyMemberLimit);
		Injector::Inject(Matchmaking_CreateLobby, SteamMatchmaking_CreateLobby, &Template_Matchmaking_CreateLobby);
		Injector::Inject(ServerSend_GameSpawnPlayer, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_GameSpawnPlayer, &Template_ServerSend_GameSpawnPlayer);
		Injector::Inject(PlayerManager_SetPlayer, u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C_SetPlayer, &Template_PlayerManager_SetPlayer);
		Injector::Inject(ServerSend_SendChatMessage, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SendChatMessage, &Template_ServerSend_SendChatMessage);
		Injector::Inject(ServerSend_GameOver, u1099u10A7u10A5u109Au109Bu10A0u10A2u10A2u10A5u10A7u109A_GameOver, &Template_ServerSend_GameOver);
		Injector::Inject(ServerSend_FallingBlocks, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SendBlockCrush, &Template_ServerSend_FallingBlocks);
		Injector::Inject(ServerSend_PlayRoundEnd, u109Au10A2u10A0u10A0u10A7u109Bu10A3u10A3u10A5u109Eu10A7_PlayRoundEnd, &Template_ServerSend_PlayRoundEnd);
		Injector::Inject(ServerSend_PlayRoundStart, u109Au10A2u10A0u10A0u10A7u109Bu10A3u10A3u10A5u109Eu10A7_PlayRoundStart, &Template_ServerSend_PlayRoundStart);
		Injector::Inject(Client_SpectatePlayer, u109Du10A8u10A0u109Bu109Bu10A0u109Cu109Cu10A6u109Au10A7_SendSpectating, &Template_Client_SpectatePlayer);
		Injector::Inject(ServerSend_SpectatorSpawn, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SpectatorSpawn, &Template_ServerSend_SpectatorSpawn);
		Injector::Inject(LobbyManager_AddPlayerToLobby, u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4_AddPlayerToLobby, &Template_LobbyManager_AddPlayerToLobby);
		Injector::Inject(LobbyManager_RemovePlayerFromLobby, u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4_RemovePlayerFromLobby, &Template_LobbyManager_RemovePlayerFromLobby);
		Injector::Inject(ChatBox_AppendMessage, u109Du10A6u109Eu10A3u10A7u10A2u10A3u10A8u10A8u109Eu10A1_AppendMessage, &Template_ChatBox_AppendMessage);
		Injector::Inject(ChatBox_Update, u109Du10A6u109Eu10A3u10A7u10A2u10A3u10A8u10A8u109Eu10A1_Update, &Template_ChatBox_Update);
		Injector::Inject(ServerSend_PunchPlayer, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_PunchPlayer, &Template_ServerSend_PunchPlayer);
		Injector::Inject(LobbyManager_BanPlayer, u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4_BanPlayer, &Template_LobbyManager_BanPlayer);
		Injector::Inject(ServerSend_PlayerDied, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_PlayerDied, &Template_ServerSend_PlayerDied);
		Injector::Inject(ServerSend_LoadMap, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_LoadMap_1, &Template_ServerSend_LoadMap);
		Injector::Inject(ServerSend_SendModeState, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SendModeState, &Template_ServerSend_SendModeState);
	}

	Server::Init();
	SocketServer::Connect();
}

void Mod::DropMoney(Player* targetPlayer) {
	HF_DropMoney->original(targetPlayer->m_ClientId, 1, 1, NULL);
}

void Mod::LoadMap(int mapID, int modeID) {
	void (*LoadMap)(int32_t, int32_t, const void*) = (void (*)(int32_t mapID, int32_t modeID, const void* method))(Injector::m_AssemblyBase + 19771072);
	LoadMap(mapID, modeID, NULL);
}

List_1_T_Enumerator_GameModeData_ Mod::GameModeData_GetEnumerator(List_1_GameModeData_* gameModeDataList, MethodInfo* method) {
	List_1_T_Enumerator_GameModeData_(*GameModeData_GetEnumerator)(List_1_GameModeData_*, const void*) = (List_1_T_Enumerator_GameModeData_(*)(List_1_GameModeData_ * gameModeDataList, const void* method))(Injector::m_AssemblyBase + 0x008EA430);
	return GameModeData_GetEnumerator(gameModeDataList, NULL);
}

void Mod::ExplodePlayer(long long clientId) {
	//auto lobbyManager = (*u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4__TypeInfo)->static_fields->Instance;
	auto gameManager = Mod::GetGameManager()->static_fields->Instance;
	auto activePlayers = gameManager->fields.activePlayers;
	
	for (size_t i = 0; i < activePlayers->fields.count; i++)
	{
		auto playerManager = activePlayers->fields.entries->vector[i].value;

		if (playerManager->fields.steamProfile.m_SteamID == clientId) {
			playerManager->fields.dead = true;
			playerManager->fields.justDied = true;
			auto gameMode = gameManager->fields.gameMode;
			HF_ExplodePlayer->original(gameMode, playerManager, NULL);
			break;
		}
	}
	Mod::KillPlayer(clientId);
}

void Mod::GameModeData_RemoveAt(int32_t index) {
	void (*GameModeData_RemoveAt)(List_1_GameModeData_ *, int32_t, const void*) = (void (*)(List_1_GameModeData_ * gameModeDataList, int32_t index, const void* method))(Injector::m_AssemblyBase + 0x0091C360);
	GameModeData_RemoveAt(Mod::modeList, index, NULL);
}

GameModeData* Mod::GameModeData_get_Item(List_1_GameModeData_* gameModeDataList, int32_t index) {
	GameModeData* (*GameModeData_get_Item)(List_1_GameModeData_*, int32_t, const void*) = (GameModeData* (*)(List_1_GameModeData_ * gameModeDataList, int32_t index, const void* method))(Injector::m_AssemblyBase + 0x008F29D0);
	return GameModeData_get_Item(gameModeDataList, index, NULL);
}

String* Mod::GetAvailableModesString(void* _this) {
	String* (*getAvailableModesString)(void*, const void*) = (String* (*)(void* _this, const void* method))(Injector::m_AssemblyBase + 0x010C5490);
	return getAvailableModesString(_this, NULL);
}

bool Mod::GameModeData_Contains(List_1_GameModeData_* gameModeDataList, GameModeData* gameModeDataItem) {
	bool (*GameModeData_Contains)(List_1_GameModeData_*, GameModeData*, const void*) = (bool (*)(List_1_GameModeData_ * mapList, GameModeData* gameModeDataItem, const void* method))(Injector::m_AssemblyBase + 0x00917B80);
	bool result = GameModeData_Contains(gameModeDataList, gameModeDataItem, NULL);
	return result;
}

bool Mod::Map_Contains(List_1_Map_* mapList, Map* mapItem) {
	bool (*Map_Contains)(List_1_Map_*, Map*, const void*) = (bool (*)(List_1_Map_ * mapList, Map* mapItem, const void* method))(Injector::m_AssemblyBase + 0x00917B80);
	bool result = Map_Contains(mapList, mapItem, NULL);
	return result;
}

int32_t Mod::GameModeData_get_Count(List_1_GameModeData_* modeList) {
	int32_t (*GameModeData_get_Count)(List_1_GameModeData_*, const void*) = (int32_t(*)(List_1_GameModeData_ * _this, const void* method))(Injector::m_AssemblyBase + 0x00250040);
	return GameModeData_get_Count(modeList, NULL);
}

int32_t Mod::Map_get_Count(List_1_Map_ * mapList) {
	int32_t (*Map_get_Count)(List_1_Map_*, const void*) = (int32_t (*)(List_1_Map_ * _this, const void* method))(Injector::m_AssemblyBase + 0x00250040);
	return Map_get_Count(mapList, NULL);
}

void Mod::Map_Remove(List_1_Map_* mapList, Map* item) {
	bool (*Map_Remove)(List_1_Map_*, Map*, const void*) = (bool (*)(List_1_Map_ * _this, Map * item, const void* method))(Injector::m_AssemblyBase + 0x008EDC20);
	Map_Remove(mapList, item, NULL);
}

void Mod::SpectatePlayer(long long clientId) {
	Client_SpectatePlayer->original(clientId, NULL);
}

void Mod::SendModeState(int32_t state) {
	void (*sendModeState)(int32_t, void*) = (void (*)(int32_t state, void* method))(Injector::m_AssemblyBase + 19781920);
	sendModeState(state, NULL);
}

void Mod::SendChatMessage(long long fromClient, std::string message) {
	//std::cout << "[Mod] SendChatMessage " << "(" << fromClient << "): '" << message << "'" << std::endl;

	ServerSend_SendChatMessage->original(fromClient, Mod::CreateMonoString(message.c_str()));
}

void Mod::CreateLobby() {
	Matchmaking_CreateLobby->original(2, 15, NULL);
}

monoString* createMonoString(const char* str) {
	monoString* (*String_CreateString)(void* _this, const char* str) = (monoString * (*)(void*, const char*))(Injector::m_AssemblyBase + 8780720);
	return String_CreateString(NULL, str);
}

float Mod::m_get_deltaTime() {
	float (*get_deltaTime)(void* method) = (float (*)(void* method))(Injector::m_AssemblyBase + 0x003A34C0);
	return get_deltaTime(NULL);
}

void Mod::AppendLocalChatMessage(long long fromClient, std::string username, std::string message) {
	//std::cout << "[Mod] AppendLocalChatMessage " << username << "(" << fromClient << "): '" << message << "'" << std::endl;

	ChatBox_AppendMessage->original(GetChatBox()->static_fields->Instance, fromClient, (String*)createMonoString(message.c_str()), (String*)createMonoString(username.c_str()), NULL);
	//ChatBox_AppendMessage->original(GetChatBox()->static_fields->Instance, fromClient, CreateMonoString(message.c_str()), CreateMonoString(username.c_str()));
}

void Mod::SendDropItem(long long toClient, int objectId, int itemId, int ammo) {
	//std::cout << "[Mod] SendDropItem toClient=" << toClient << ", objectId='" << objectId << "', itemId='" << itemId << "', ammo='" << ammo << "'" << std::endl;

	void (*DropItem)(uint64_t clientId, int32_t int1, int32_t int2, int32_t int3) = (void (*)(uint64_t clientId, int32_t int1, int32_t int2, int32_t int3))(Injector::m_AssemblyBase + 19767472);
	DropItem(toClient, objectId, itemId, ammo);
}

void Mod::ForceGiveItem(long long toClient, int objectId, int itemId) {
	/*std::cout << "[Mod] ForceGiveItem toClient=" << toClient << ", objectId='" << objectId << "', itemId='" << itemId << "'" << std::endl;
	void (*ForceGiveItem)(uint64_t clientId, int32_t int1, int32_t int2) = (void (*)(uint64_t clientId, int32_t int1, int32_t int2))(Injector::m_AssemblyBase + 0x0126A970);
	ForceGiveItem(toClient, objectId, itemId);*/
	ServerSend_ForceGiveWeapon->original(toClient, objectId, itemId, NULL);

}

void Mod::ForceGiveAllItem(int32_t itemId) {
	void (*ForceGiveAllItem)(int32_t itemId) = (void (*)(int32_t itemId))(Injector::m_AssemblyBase + 19310208);
	ForceGiveAllItem(itemId);
}


void Mod::SendLocalInteract(int itemid) {
	void (*TryInteract)(int32_t itemId) = (void (*)(int32_t itemId))(Injector::m_AssemblyBase + 19285136);
	TryInteract(itemid);
}

void Mod::SendFallingBlocks(float speed, int32_t coords) {
	BlockDrop::sendingBlock = true;

	for (Player* player : PlayerUtils::FindPlayers("*")) {
		SendFallingBlock(speed, coords, player->m_ClientId);
	}

	BlockDrop::sendingBlock = false;
}

void Mod::SendFallingBlock(float speed, int32_t coords, uint64_t steamId) {
	void (*SendBlock)(float, int32_t, uint64_t, const void*) = (void (*)(float speed, int32_t coords, uint64_t steamId, const void* method))(Injector::m_AssemblyBase + 19778576);
	SendBlock(speed, coords, steamId, NULL);
} 

void Mod::ForceRemoveAllItem(int itemId) {
	auto alivePlayers = PlayerUtils::FindPlayers("*a");

	for (size_t i = 0; i < alivePlayers.size(); i++)
	{
		auto targetPlayer = alivePlayers[i];
		uint64_t clientId = targetPlayer->m_ClientId;
		void (*ForceRemoveAllItem)(uint64_t clientId, uint32_t itemId) = (void(*)(uint64_t clientId, uint32_t itemId))(Injector::m_AssemblyBase + 19312224);
		ForceRemoveAllItem(clientId, itemId);
	}
}

void Mod::SendInteract(long long clientId, int itemid) {
	void (*TryInteract)(uint64_t clientId, int32_t itemId) = (void (*)(uint64_t clientId, int32_t itemId))(Injector::m_AssemblyBase + 19285136);
	TryInteract(clientId, itemid);
}

void Mod::KillPlayer(long long clientId) {
	std::cout << "[Mod] KillPlayer clientId=" << clientId << std::endl;

	ServerSend_PlayerDied->original(clientId, clientId, Vector3({ 0, 1, 0 }), NULL);
	//Server_DamagePlayer->original(clientId, 30, Vector3{ 0.0,0.0,0.0 }, 1, 128, NULL);
}

void Mod::BanPlayer(long long clientId) {
	std::cout << "[Mod] BanPlayer clientId=" << clientId << std::endl;

	auto lobbyManager = (*u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4__TypeInfo)->static_fields->Instance;

	u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4_BanPlayer(lobbyManager, clientId, NULL);
}

void Mod::KickPlayer(long long clientId) {
	std::cout << "[Mod] KickPlayer clientId=" << clientId << std::endl;

	auto lobbyManager = (*u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4__TypeInfo)->static_fields->Instance;

	u109Bu10A2u10A4u10A6u109Du10A3u109Cu1099u109Du109Cu10A4_KickPlayer(lobbyManager, clientId, NULL);
}

void Mod::RespawnPlayer(long long clientId, Vector3 position) {
	std::cout << "[Mod] RespawnPlayer clientId=" << clientId << Mod::FormatVector(position) << std::endl;

	void (*Respawn)(uint64_t, Vector3, const void*) = (void (*)(uint64_t toClient, Vector3 spawnPos, const void* method))(Injector::m_AssemblyBase + 19778288);
	Respawn(clientId, position, NULL);
	auto player = PlayerUtils::GetPlayer(clientId);
	player->m_IsAlive = true;
}

void Mod::StartGame() {
	void (*SendReadyPlayer)(uint64_t, bool, const void*) = (void (*)(uint64_t clientId, bool ready, const void* method))(Injector::m_AssemblyBase + 19782816);

	auto allPlayers = PlayerUtils::FindPlayers("*");

	for (size_t i = 0; i < allPlayers.size(); i++)
	{
		Player* targetPlayer = allPlayers[i];
		SendReadyPlayer(targetPlayer->m_ClientId, true, NULL);
	}
}

void Mod::RestartGame() {
	std::cout << "[Mod] RestartGame" << std::endl;

	void (*Restart)() = (void (*)())(Injector::m_AssemblyBase + 19788976);
	Restart();
}

void Mod::SetCurrentGameModeTime(float time) {
	auto gameMode = GetGameManager()->static_fields->Instance->fields.gameMode;

	void (*GameMode__SetGameModeTimer)(void* _this, float time, uint64_t param_2) = (void (*)(void* _this, float time, uint64_t param_2))(Injector::m_AssemblyBase + 17156720);

	GameMode__SetGameModeTimer(gameMode, time, 0);
}

void Mod::SetBomber(long long clientId) {
	void (*Fn)(uint64_t param1, uint64_t param2) = (void (*)(uint64_t, uint64_t))(Injector::m_AssemblyBase + 19787040);
	Fn(clientId, clientId);
	Fn(clientId, 0);
}

void Mod::TagPlayer(long long clientId) {
	void (*Fn)(uint64_t param1, uint64_t param2) = (void (*)(uint64_t, uint64_t))(Injector::m_AssemblyBase + 19790432);
	Fn(clientId, clientId);
}

void Mod::ToggleLights(bool on) {
	void (*Fn)(bool param1) = (void (*)(bool))(Injector::m_AssemblyBase + 19790688);
	Fn(on);
}

void Mod::GiveHat(long long clientId) {
	void (*Fn)(uint64_t param1, uint64_t param2) = (void (*)(uint64_t, uint64_t))(Injector::m_AssemblyBase + 19770272);
	Fn(clientId, clientId);
}

void Mod::SendWinner(long long clientId, long long money) {
	Win::modSentWinGame = true;
	void (*Fn)(uint64_t param1, uint64_t param2) = (void (*)(uint64_t, uint64_t))(Injector::m_AssemblyBase + 19786784);
	Fn(clientId, money);
}

u10A4u109Bu10A5u109Eu10A3u10A3u109Bu10A2u109Du109Du10A2__Class* Mod::GetSomething() {
	return (*u10A4u109Bu10A5u109Eu10A3u10A3u109Bu10A2u109Du109Du10A2__TypeInfo);
}

u10A1u10A0u10A1u109Eu10A5u10A1u109Du10A8u10A5u1099u109A__Class* Mod::GetGameManager() {
	return (*u10A1u10A0u10A1u109Eu10A5u10A1u109Du10A8u10A5u1099u109A__TypeInfo);
}

u10A0u10A4u10A8u10A1u10A8u109Au10A8u10A1u109Eu1099u109F__Class* Mod::GetSteamManager() {
	return (*u10A0u10A4u10A8u10A1u10A8u109Au10A8u10A1u109Eu1099u109F__TypeInfo);
}

u109Du10A6u109Eu10A3u10A7u10A2u10A3u10A8u10A8u109Eu10A1__Class* Mod::GetChatBox() {
	return (*u109Du10A6u109Eu10A3u10A7u10A2u10A3u10A8u10A8u109Eu10A1__TypeInfo);
}
