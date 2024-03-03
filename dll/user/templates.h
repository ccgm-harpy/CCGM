#pragma once

#include "Injector.h"
#include "Mod.h"
#include "Server.h"
#include "Tips.h"
#include "Win.h"
#include "Chat.h"
#include "Player.h"
#include "HTTP.h"
#include <random>
#include "gamemodes/BlockDrop.h"
#include "gamemodes/CrabFight.h"
#include <iostream>
#include <string>
#include "gamemodes/LightsOut.h"
#include "gamemodes/SnowBrawl.h"
#include "gamemodes/Chaos.h"
#include "gamemodes/ReadyUp.h"
#include "weapons.h"

static InteropHelp_UTF8StringHandle* stringToUTF8StringHandle(std::string str)
{
	auto h = new InteropHelp_UTF8StringHandle();
	InteropHelp_UTF8StringHandle__ctor(h, (String*)il2cpp_string_new(str.c_str()), NULL);
	return h;
}

auto HF_DropMoney = new HookFunction<void, uint64_t, int32_t, int32_t, MethodInfo*>(0x012DA1F0);
void Template_DropMoney(uint64_t clientId, int32_t a, int32_t b, MethodInfo* method) {
	HF_DropMoney->original(clientId, a, b, method);
}

// 0x0039B940, void, Screen_SetResolution_1, (int32_t width, int32_t height, bool fullscreen, int32_t preferredRefreshRate, MethodInfo * method));
auto HF_Screen_SetResolution_1 = new HookFunction<void, int32_t, int32_t, bool, int32_t, MethodInfo*>(0x0039B940);
void Template_HF_Screen_SetResolution_1(int32_t width, int32_t height, bool fullscreen, int32_t preferredRefreshRate, MethodInfo* method) {
	width = 480;
	height = 480;
	fullscreen = false;
	preferredRefreshRate = 30;
	HF_Screen_SetResolution_1->original(width, height, fullscreen, preferredRefreshRate, method);
}

// 0x01399200, float, u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5_FindCrabDamage, (u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5* __this, MethodInfo* method));
auto HF_FindCrabDamage = new HookFunction<float, void*, void*>(0x01399200);
float Template_HF_FindCrabDamage(void* _this, void* method) {
	float realDamage = HF_FindCrabDamage->original(_this, method);
	
	CrabFight::requiredHitsFake = CrabFight::fakeHpStart / (realDamage * CrabFight::damageMultiplier);
	CrabFight::simulatedDamage = 100 / CrabFight::requiredHitsFake;
	CrabFight::realHp -= CrabFight::simulatedDamage;
	CrabFight::fakeHp -= CrabFight::fakeHpStart / CrabFight::requiredHitsFake;

	return CrabFight::simulatedDamage;
}

//0x00BEA420, void, u1099u10A5u109Fu10A8u1099u10A4u10A4u109Du109Du10A1u10A0_CrabDamage, (uint64_t u1099u10A8u109Du10A8u10A0u109Eu10A1u10A1u10A8u10A1u10A7, u10A5u109Cu10A4u1099u10A0u10A3u109Bu109Du10A4u10A6u109D* u109Du10A3u10A2u109Au10A7u109Au10A7u109Du109Cu109Cu10A2, MethodInfo* method));
auto HF_CrabDamage = new HookFunction<void, uint64_t, u10A5u109Cu10A4u1099u10A0u10A3u109Bu109Du10A4u10A6u109D*, void*>(0x00BEA420);
void Template_HF_CrabDamage(uint64_t clientId, u10A5u109Cu10A4u1099u10A0u10A3u109Bu109Du10A4u10A6u109D* a, void* method) {
	HF_CrabDamage->original(clientId, a, method);
}

//0x012D9ED0, void, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_CrabHp, (float u109Au10A4u10A4u10A0u109Bu109Eu109Au109Du10A7u109Du10A5, MethodInfo * method));
auto HF_CrabHp = new HookFunction<void, float, void*>(0x012D9ED0);
void Template_HF_CrabHp(float hp, void* method) {
	if (CrabFight::fakeHp < 0.0) {
		CrabFight::fakeHp = 0.0;
	}

	HF_CrabHp->original(CrabFight::fakeHp, method);
}

//DO_APP_FUNC(0x013999E0, void, u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5_SpawnCrab, (u10A2u10A5u109Fu10A2u109Fu10A4u10A0u10A7u10A8u10A7u10A5 * __this, MethodInfo * method));
auto HF_SpawnCrab = new HookFunction<void, void*, void*>(0x013999E0);
void Template_HF_SpawnCrab(void* _this, void* method) {
	HF_SpawnCrab->original(_this, method);
	
	if (CrabFight::crabAngry) {
		HF_SpawnCrab->original(_this, method);
	}
}

//DO_APP_FUNC(0x012D9DE0, void, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_CrabDifficulty, (int32_t u109Bu10A7u10A2u109Du109Eu10A1u109Du10A6u10A6u109Cu10A4, MethodInfo * method));
auto HF_CrabDifficulty = new HookFunction<void, int32_t, void*>(0x012D9DE0);
void Template_HF_CrabDifficulty(int32_t difficulty, void* method) {
	if (CrabFight::crabAngry) {
		if (difficulty == 1) {
			Chat::SendServerMessage("!!! YOU MADE THE CRAB ANGRY !!!");
		}

		if (difficulty >= 1) {
			HF_CrabDifficulty->original(4, method);
		}
	}
	
	else {
		HF_CrabDifficulty->original(difficulty, method);
	}
}

Vector3 CrabBall_PlayerPosition(Vector3 ballPos) {
	int distance = 5;
	int distanceChoice = rand() % 3;

	if (distanceChoice == 0) {
		distance = 25;
	}
	else if (distanceChoice == 1) {
		distance = 12;
	}


	if (CrabFight::bullyPlayer) {
		if (rand() % 2 == 0) {
			ballPos.x = CrabFight::crabTarget->m_Position.x + rand() % distance;
		}
		else {
			ballPos.x = CrabFight::crabTarget->m_Position.x - rand() % distance;
		}

		if (rand() % 2 == 0) {
			ballPos.z = CrabFight::crabTarget->m_Position.z + rand() % distance;
		}
		else {
			ballPos.z = CrabFight::crabTarget->m_Position.z - rand() % distance;
		}
	}
	else {
		if (rand() % 2 == 0) {
			ballPos.x += rand() % distance;
		}
		else {
			ballPos.x -= rand() % distance;
		}

		if (rand() % 2 == 0) {
			ballPos.z += rand() % distance;
		}
		else {
			ballPos.z -= rand() % distance;
		}
	}
	

	return ballPos;
}

auto HF_SpawnCrabSpikes = new HookFunction<void, void*, Vector3, void*>(0x01399910);
void Template_HF_SpawnCrabSpikes(void* _this, Vector3 a, void* method) {
	if (CrabFight::bullyPlayer) {
		if (!CrabFight::crabTarget->m_IsAlive) {
			CrabFight::SelectTarget();
		}
	}
	
	if (CrabFight::crabAngry || CrabFight::bullyPlayer) {
		if (CrabFight::crabAngry) {
			json crabFightSettings = Server::ccgmConfig["gameModeSettings"]["crabFight"];

			for (int i = 0; i < rand() % crabFightSettings["maxAngerCrabBalls"] + 1; i++) {
				HF_SpawnCrabSpikes->original(_this, CrabBall_PlayerPosition(a), method);
			}
		}
		else if (CrabFight::bullyPlayer) {
			HF_SpawnCrabSpikes->original(_this, CrabBall_PlayerPosition(a), method);
		}
	}

	else {
		HF_SpawnCrabSpikes->original(_this, a, method);
	}
}

//DO_APP_FUNC(0x012DCF50, void, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_SendCrabBall, (Vector3 u10A2u109Fu109Au109Du10A4u10A8u10A6u10A4u10A2u10A5u109E, int32_t u10A4u1099u10A5u10A5u109Bu109Du10A5u109Du10A8u109Cu10A4, MethodInfo* method));
auto HF_SendCrabBall = new HookFunction<void, Vector3, int32_t, void*>(0x012DCF50);
void Template_HF_SendCrabBall(Vector3 a, int32_t b, void* method) {
	if (CrabFight::bullyPlayer) {
		if (!CrabFight::crabTarget->m_IsAlive) {
			CrabFight::SelectTarget();
		}
	}

	if (CrabFight::crabAngry || CrabFight::bullyPlayer) {
		if (CrabFight::crabAngry) {
			json crabFightSettings = Server::ccgmConfig["gameModeSettings"]["crabFight"];

			for (int i = 0; i < rand() % crabFightSettings["maxAngerCrabBalls"] + 1; i++) {
				HF_SendCrabBall->original(CrabBall_PlayerPosition(a), b, method);
			}
		}
		else if (CrabFight::bullyPlayer) {
			HF_SendCrabBall->original(CrabBall_PlayerPosition(a), b, method);
		}
	}

	else {
		HF_SendCrabBall->original(a, b, method);
	}
}


//DO_APP_FUNC(0x012D9FC0, void, u10A0u10A8u10A0u10A4u10A8u10A0u109Au10A5u10A7u10A7u109C_CrabTarget, (uint64_t u10A4u10A4u10A3u10A1u10A4u10A8u10A0u109Au109Eu10A5u10A6, MethodInfo * method));
auto HF_CrabTarget = new HookFunction<void, uint64_t, void*>(0x012D9FC0);
void Template_HF_CrabTarget(uint64_t clientId, void* method) {
	if (CrabFight::bullyPlayer) {
		if (!CrabFight::crabTarget->m_IsAlive) {
			CrabFight::SelectTarget();
		}

		clientId = CrabFight::crabTarget->m_ClientId;
	}

	HF_CrabTarget->original(clientId, method);
}

auto HF_ExplodePlayer = new HookFunction<void, void*, void*, void*>(0x01184EA0);
void Template_HF_ExplodePlayer(void* a, void* b, void* c) {
	std::cout << "EXPLODING PLAYER" << std::endl;
	HF_ExplodePlayer->original(a, b, c);
}

auto HF_NativeMethods_ISteamMatchmaking_SetLobbyData = new HookFunction<void, void*, CSteamID, InteropHelp_UTF8StringHandle*, InteropHelp_UTF8StringHandle*, MethodInfo*>(0x00237020);
void Template_NativeMethods_ISteamMatchmaking_SetLobbyData(void* a, CSteamID b, InteropHelp_UTF8StringHandle* key, InteropHelp_UTF8StringHandle* value, MethodInfo* method)
{
	String* keyString = InteropHelp_PtrToStringUTF8(key->fields._._.handle, NULL);
	String* valueString = InteropHelp_PtrToStringUTF8(value->fields._._.handle, NULL);

	if (keyString->toCPPString() == "LobbyName")
	{
		value = stringToUTF8StringHandle(Server::activeServerName);
	}

	else if (keyString->toCPPString() == "Modes") {
		value = stringToUTF8StringHandle(Server::ccgmConfig["modes"]);
	}

	else if (keyString->toCPPString() == "Maps") {
		value = stringToUTF8StringHandle(Server::ccgmConfig["maps"]);
	}

	else if (keyString->toCPPString() == "PlayersIn") {
		HTTP::PostPlayersIn(std::stoi(valueString->toCPPString()));
	}

	HF_NativeMethods_ISteamMatchmaking_SetLobbyData->original(a, b, key, value, method);
}

// DO_APP_FUNC(0x002B4870, bool, SteamMatchmaking_SetLobbyType, (CSteamID steamIDLobby, ELobbyType__Enum eLobbyType, MethodInfo * method));
auto HF_SteamMatchmaking_SetLobbyType = new HookFunction<bool, CSteamID, ELobbyType__Enum, MethodInfo*>(0x002B4870);
void Template_SteamMatchmaking_SetLobbyType(CSteamID steamIDLobby, ELobbyType__Enum eLobbyType, MethodInfo* method) {
	int ccgmLobbyType = Server::ccgmConfig["lobbyType"];
	std::cout << "Lobby type set to "  << ccgmLobbyType << std::endl;
	HF_SteamMatchmaking_SetLobbyType->original(steamIDLobby, static_cast<ELobbyType__Enum>(ccgmLobbyType), method);
}

auto HF_VoiceChatOnOrOff = new HookFunction<void, bool, MethodInfo*>(0x01269D30);
void Template_VoiceChatOnOrOff(bool enabled, MethodInfo* method) {
	enabled = Server::ccgmConfig["voiceEnabled"];
	std::cout << "Voice enabled: " << enabled << std::endl;
	HF_VoiceChatOnOrOff->original(enabled, method);
}

auto Matchmaking_SetLobbyMemberLimit = new HookFunction<void, CSteamID, int32_t, void*>(0x002B47B0);
void Template_Matchmaking_SetLobbyMemberLimit(CSteamID steamIDLobby, int32_t cMaxMembers, void* method) {
	std::cout << "INVITE CODE " << std::to_string(steamIDLobby.m_SteamID) << std::endl;
	std::cout << "serverName " << Server::activeServerName << std::endl;
	HTTP::PostLobbyReady(steamIDLobby.m_SteamID, Server::ccgmConfig["maxPlayers"]);

	Matchmaking_SetLobbyMemberLimit->original(steamIDLobby, Server::ccgmConfig["maxPlayers"], method);
}

float menuTimer = 0;

void SendButtonClicks(float dt) {
	if (menuTimer <= 10000) {
		menuTimer += dt;
	}

	if (menuTimer > 5000 && menuTimer < 10000) {
		menuTimer = 50000;
		Mod::CreateLobby();
	}
}

auto HF_BaseInput_get_mousePosition = new HookFunction<Vector2, BaseInput*, MethodInfo*>(0x0041ED80);
Vector2 Template_HF_BaseInput_get_mousePosition(BaseInput* _this, MethodInfo* method) {
	Vector2 mousePosition = HF_BaseInput_get_mousePosition->original(_this, method);

	if (menuTimer <= 40000) {
		if (Mod::clickButton == 1) {
			mousePosition.x = 103;
			mousePosition.y = 287;
		}
		else if (Mod::clickButton == 2) {
			mousePosition.x = 122;
			mousePosition.y = 408;
		}
	}
	
	return mousePosition;
}



auto HF_Input_GetMouseButtonUp = new HookFunction<bool, BaseInput*, int32_t, MethodInfo*>(0x0041ECD0);
bool Template_HF_Input_GetMouseButtonUp(BaseInput* _this, int32_t button, MethodInfo* method) {
	bool buttonUp = HF_Input_GetMouseButtonUp->original(_this, button, method);

	if (Mod::mouseUp && button == 0) {
		return true;
	}

	return buttonUp;
}

auto HF_Input_GetMouseButtonDown = new HookFunction<bool, BaseInput*, int32_t, MethodInfo*>(0x0041ECC0);
bool Template_HF_Input_GetMouseButtonDown(BaseInput* _this, int32_t button, MethodInfo* method) {
	bool buttonDown = HF_Input_GetMouseButtonDown->original(_this, button, method);
	
	if (Mod::mouseDown && button == 0) {
		return true;
	}

	return buttonDown;
}

auto HF_Component_GetComponent_4 = new HookFunction<Button*, Component*, MethodInfo*>(0x00A5A350);
Button* Template_HF_Component_GetComponent_4(Component* _this, MethodInfo* method) {
	Button* button = HF_Component_GetComponent_4->original(_this, method);

	auto now = std::chrono::system_clock::now();
	std::chrono::duration<float, std::milli> diff = now - Mod::m_LastUpdatedTime;
	auto dt = diff.count();

	SendButtonClicks(dt > 16 ? 16 : dt);

	return button;
}

int toggleStartIndex = 0;

auto HF_Toggle_Set = new HookFunction<void, Toggle*, bool, bool, MethodInfo*>(0x008A43E0);
void Template_HF_Toggle_Set(Toggle* _this, bool value, bool sendCallback, MethodInfo* method) {
	HF_Toggle_Set->original(_this, value, sendCallback, method);
}


auto Matchmaking_CreateLobby = new HookFunction<void, int, int32_t, void*>(0x002B3550);
void Template_Matchmaking_CreateLobby(int eLobbyType, int32_t cMaxMembers, void* method) {
	Matchmaking_CreateLobby->original(eLobbyType, cMaxMembers, method);
}

auto Client_SpectatePlayer = new HookFunction<void, uint64_t, void*>(19283392);
void Template_Client_SpectatePlayer(uint64_t spectatingClientId, void* method) {
	std::cout << "CLIENT SPECTATING PLAYER " << spectatingClientId << std::endl;
	Client_SpectatePlayer->original(spectatingClientId, method);
}

auto ServerSend_SpectatorSpawn = new HookFunction<void, uint64_t, void*>(19787792);
void Template_ServerSend_SpectatorSpawn(uint64_t spawnClientId, void* method) {
	std::cout << "CLIENT SPAWNING SPECTATOR" << spawnClientId << std::endl;
	ServerSend_SpectatorSpawn->original(spawnClientId, method);
}

auto ServerSend_ForceGiveWeapon = new HookFunction<void, uint64_t, int32_t, int32_t, MethodInfo*>(0x0126A970);
void Template_ServerSend_ForceGiveWeapon(uint64_t clientId, int32_t a, int32_t b, MethodInfo* method) {
	ServerSend_ForceGiveWeapon->original(clientId, a, b, method);
}

// 0x01019DE0, void, u10A8u1099u1099u1099u10A4u10A4u10A8u109Du109Au10A4u10A5_ForceGiveItem, (u10A8u1099u1099u1099u10A4u10A4u10A8u109Du109Au10A4u10A5 * __this, ItemData * u109Bu10A2u10A8u10A3u109Du10A7u109Du109Cu109Au109Cu10A3, MethodInfo * method));
auto ServerSend_ForceGiveItem = new HookFunction<void, void*, ItemData*, MethodInfo*>(0x01019DE0);
void Template_ServerSend_ForceGiveItem(void* __this, ItemData* itemData, MethodInfo* method) {
	Weapon weapon = Weapons::list[itemData->fields.itemID];
	itemData->fields.currentAmmo = weapon.ammo;
	itemData->fields.maxAmmo = weapon.ammo;
	ServerSend_ForceGiveItem->original(__this, itemData, method);
}

auto ServerSend_LoadMap = new HookFunction<void, int32_t, int32_t, MethodInfo*>(19771072);
void Template_ServerSend_LoadMap(int32_t map, int32_t mode, MethodInfo* method)
{
	
	std::cout << "[Mod] Load map " << map << ", mode=" << mode << std::endl;
	Chat::m_Messages.clear();
	Chaos::eventTime = 1;
	Tips::incrementTipTime = true;
	ReadyUp::sendRequiredPlayersMessage = false;
	ReadyUp::sentRequiredPlayersMessage = true;

	Chaos::eventInterval = 12000;

	Win::roundEnded = false;
	Win::wonRoundOrGame = false;
	Win::SpawnBombs = false;
	Chaos::givePointTime = 1;
	LightsOut::flashTime = 0;
	PlayerUtils::m_CanUpdateSpawnPosition = true;
	SnowBrawl::CanSpawnSnowballs = false;
	Server::RoundStarted = false;

	if (Server::ccgmConfig["gameModeSettings"]["chaos"]["enabled"]) {
		Chaos::ChanceStart(Server::ccgmConfig["gameModeSettings"]["chaos"]["roundProbability"]);
	}

	if (Server::ccgmConfig["gameModeSettings"]["customMaps"]) {
		Tips::addExtraLoadTime = true;
	}

	BlockDrop::Stop();
	CrabFight::Stop();

	if (mode == 0) {
		Server::InLobby = true;
	}
	else {
		if (Server::InLobby) {
			Server::gameStartPlayerCount = PlayerUtils::FindPlayers("*").size();
		}

		Server::InLobby = false;
	}

	if (!Server::modLoadMap && !Server::ccgmConfig["gameModeSettings"]["customMaps"]) {
		if (PlayerUtils::FindPlayers("*a").size() >= Server::ccgmConfigBase["mapModeSwapPlayerRequirement"]) {
			if (mode == 6 || mode == 5 || mode == 4) {
				int odds = Server::ccgmConfig["gameModeSettings"]["tagSwapProbability"];

				if (rand() % 100 + 1 <= odds) {
					int mapChoice = rand() % 7 + 1;

					if (mapChoice == 1) {
						map = 6;
					}
					else if (mapChoice == 2) {
						map = 17;
					}
					else if (mapChoice == 3) {
						map = 55;
					}
					else if (mapChoice == 4) {
						map = 56;
					}
					else if (mapChoice == 5) {
						map = 9;
					}
					else if (mapChoice == 6) {
						map = 14;
					}
					else {
						map = 57;
					}
				}
			}
			else if (mode == 16) {
				int odds = Server::ccgmConfig["gameModeSettings"]["snowBrawlSwapProbability"];

				if (rand() % 100 + 1 <= odds) {
					int mapChoice = rand() % 5 + 1;
					SnowBrawl::CanSpawnSnowballs = true;

					if (mapChoice == 1) {
						SnowBrawl::CanSpawnSnowballs = false;
						map = 6;
					}
					else if (mapChoice == 2) {
						map = 22;
					}
					else if (mapChoice == 3) {
						map = 3;
					}
					else if (mapChoice == 4) {
						map = 9;
					}
					else {
						map = 14;
					}
				}
			}
			else if (mode == 2) {
				int odds = Server::ccgmConfig["gameModeSettings"]["sumoModeProbability"];

				if (rand() % 100 + 1 <= odds) {
					int mapChoice = rand() % 10 + 1;
					if (mapChoice == 1) {
						map = 0;
					}
					else if (mapChoice == 2) {
						map = 2;
					}
					else if (mapChoice == 3) {
						map = 15;
					}
					else if (mapChoice == 4) {
						map = 20;
					}
					else if (mapChoice == 5) {
						map = 29;
					}
					else if (mapChoice == 6) {
						map = 35;
					}
					else if (mapChoice == 7) {
						map = 36;
					}
					else if (mapChoice == 8) {
						map = 38;
					}
					else if (mapChoice == 9) {
						map = 49;
					}
					else if (mapChoice == 10) {
						map = 51;
					}
				}
			}
		}
		Server::GameMode = mode;
		Server::GameMap = map;
	}
	else {
		mode = Server::GameMode;
		map = Server::GameMap;
		Server::modLoadMap = false;
	}
	
	HTTP::PostMapMode(map, mode);
	ServerSend_LoadMap->original(map, mode, method);
}

auto ServerSend_GameSpawnPlayer = new HookFunction<void, uint64_t, uint64_t, Vector3, int32_t, bool, void*, int32_t>(19769408);
void Template_ServerSend_GameSpawnPlayer(uint64_t toClientId, uint64_t spawnedClientId, Vector3 spawnPos, int32_t param_3, bool streamerMode, void* byteArray, int32_t numberId)
{
	if (PlayerUtils::m_CanUpdateSpawnPosition) {
		PlayerUtils::m_CanUpdateSpawnPosition = false;
		PlayerUtils::m_SpawnPosition = spawnPos;
	}

	if (PlayerUtils::HasPlayer(spawnedClientId)) {
		Player* player = PlayerUtils::GetPlayer(spawnedClientId);

		player->m_IsAlive = true;
	}

	ServerSend_GameSpawnPlayer->original(toClientId, spawnedClientId, spawnPos, param_3, streamerMode, byteArray, numberId);
}

auto Server_DamagePlayer = new HookFunction<void, uint64_t, int32_t, Vector3, int32_t, int32_t, const void*>(0x01261900);
void Template_Server_DamagePlayer(uint64_t a, int32_t b, Vector3 c, int32_t d, int32_t e, const void* method) {
	Server_DamagePlayer->original(a, b, c, d, e, method);
}

auto ServerSend_SendModeState = new HookFunction<void, int32_t, void*>(19781920);
void Template_ServerSend_SendModeState(int32_t state, void* method) {
	if (state == 2 && Server::GameMode == 15 && BlockDrop::enabled) {
		BlockDrop::stopAtEnd = true;
		ServerSend_SendModeState->original(1, method);
	}
	else {
		BlockDrop::enabled = false;
		ServerSend_SendModeState->original(state, method);
	}
}

auto ServerSend_PlayerDied = new HookFunction<void, long long, long long, Vector3, void*>(19775664);
void Template_ServerSend_PlayerDied(long long deadClient, long long damageDoerId, Vector3 damageDir, void* method)
{
	if (PlayerUtils::HasPlayer(deadClient))
	{
		Player* deadPlayer = PlayerUtils::GetPlayer(deadClient);
		if (deadPlayer->m_GodEnabled) return;

		deadPlayer->m_IsAlive = false;

		auto SuicideMessages = Server::ccgmConfigBase["killFeedMessages"]["playerSuicide"];
		auto KillFeedMessages = Server::ccgmConfigBase["killFeedMessages"]["playerKilledPlayer"];

		if (deadClient == damageDoerId)
		{
			int suicideMessageChoice = rand() % SuicideMessages.size();
			std::string suicideMessage = SuicideMessages[suicideMessageChoice];
			Chat::SendServerMessage("" + deadPlayer->GetDisplayName() + " " + suicideMessage);
		}
		else
		{
			if (PlayerUtils::HasPlayer(damageDoerId)) {
				int killMessageChoice = rand() % KillFeedMessages.size();
				std::string killMessage = KillFeedMessages[killMessageChoice];
				Player* killerPlayer = PlayerUtils::GetPlayer(damageDoerId);
				HTTP::PostPlayerKD(killerPlayer, deadPlayer, killMessage);
				Chat::SendServerMessage("" + killerPlayer->GetDisplayName() + " " + killMessage + " " + deadPlayer->GetDisplayName());
			}
			else {
				int suicideMessageChoice = rand() % SuicideMessages.size();
				std::string suicideMessage = SuicideMessages[suicideMessageChoice];
				Chat::SendServerMessage("" + deadPlayer->GetDisplayName() + " " + suicideMessage);
			}
		}


	}

	ServerSend_PlayerDied->original(deadClient, damageDoerId, damageDir, method); 
}

auto Server_SendWinner = new HookFunction<void, uint64_t, uint64_t, void*>(0x012DEC20);
void Template_Server_SendWinner(uint64_t clientId, uint64_t money, void* method) {
	if (!Win::modSentWinGame && !Win::wonRoundOrGame) {
		Win::wonRoundOrGame = true;
		Win::sayWinMistake = true;
	}

	Win::modSentWinGame = false;
	Win::winMistakePlayerId = clientId;

	Server_SendWinner->original(clientId, money, method);
}

auto ServerSend_GameOver = new HookFunction<void, void*, MethodInfo*>(13884704);
void Template_ServerSend_GameOver(void* _this, MethodInfo* method)
{
	if (!Win::wonRoundOrGame) {
		Win::roundEnded = true;
	}

	ServerSend_GameOver->original(_this, method);
}


auto ServerSend_PlayRoundEnd = new HookFunction<void, void*, void*>(19066000);
void Template_ServerSend_PlayRoundEnd(void* _this, void* method) {
	if (!Win::wonRoundOrGame) {
		Win::roundEnded = true;
	}

	if (Server::GameMode == 15) {
		BlockDrop::Stop();
	}
	
	ServerSend_PlayRoundEnd->original(_this, method);
}

auto ServerSend_PlayRoundStart = new HookFunction<void, void*, void*>(19066064);
void Template_ServerSend_PlayRoundStart(void* _this, void* method) {
	/*if (Server::m_LobbyOwner->m_IsAlive) {
		Server::m_LobbyOwner->m_IsAlive = false;
		Mod::ExplodePlayer(Server::m_LobbyOwner->m_ClientId);
	}*/

	if (Server::GameMode == 15 && !BlockDrop::enabled) {
		BlockDrop::ChanceStart(Server::ccgmConfig["gameModeSettings"]["blockDropSequenceProbability"]);
	}
	else if (Server::GameMode == 18) {
		json crabFightSettings = Server::ccgmConfig["gameModeSettings"]["crabFight"];
		//CrabFight::BullyChanceStart(70);
		CrabFight::AngryChanceStart(crabFightSettings["angerProbability"]);
		CrabFight::RandomizeHealthChanceStart(crabFightSettings["randomizeHealthProbability"]);
		CrabFight::IncreaseDamageChanceStart(crabFightSettings["randomizeSnowballDamageProbability"]);
	}

	ServerSend_PlayRoundStart->original(_this, method);
}

auto ServerSend_FallingBlocks = new HookFunction<void, float, int32_t, uint64_t, void*>(19778576);
void Template_ServerSend_FallingBlocks(float speed, int32_t coords, uint64_t steamId, void* method) {
	// speed can be between 0 and 3
	// coords are janky, use blockDropCoords and notes folder "blockdrop_coords.png"
	
	if (BlockDrop::enabled) {
		if (BlockDrop::sendingBlock) {
			ServerSend_FallingBlocks->original(speed, coords, steamId, method);
		}
		/*else {
			// Sending to ourselves prevents others from getting blocks. This lets us send our own to their screen =]
			ServerSend_FallingBlocks->original(speed, coords, Server::m_LobbyOwner->m_ClientId, method);
		}*/
	}
	else {
		ServerSend_FallingBlocks->original(speed, coords, steamId, method);
	}
}

auto ServerSend_PunchPlayer = new HookFunction<void, uint64_t, uint64_t, Vector3>(19777456);
void Template_ServerSend_PunchPlayer(uint64_t a, uint64_t b, Vector3 dir)
{
	if (PlayerUtils::HasPlayer(a)) {
		auto player = PlayerUtils::GetPlayer(a);

		if (player->m_JumpPunchEnabled) {
			ServerSend_PunchPlayer->original(a, a, Vector3({ 0, 3, 0 }));
		}

		if (player->m_SuperPunchEnabled) {
			ServerSend_PunchPlayer->original(a, b, Vector3({ 0, 4, 0 }));
		}
	}

	if (PlayerUtils::HasPlayer(b)) {
		if (PlayerUtils::GetPlayer(b)->m_ForceFieldEnabled) {
			Vector3 newDir = Vector3({ dir.x * -3, dir.y * -3, dir.z * -3 });

			ServerSend_PunchPlayer->original(b, a, newDir);
			return;
		}
	}

	ServerSend_PunchPlayer->original(a, b, dir);
}

auto ServerSend_SendChatMessage = new HookFunction<void, uint64_t, monoString*>(19779104);
void Template_ServerSend_SendChatMessage(uint64_t fromClient, monoString* message)
{
	/*
	if (SocketServer::m_IsConnected) {

		long long currentLobby = Mod::GetSteamManager()->static_fields->Instance->fields.currentLobby.fields.m_SteamID;
		long long originalLobbyOwnerId = Mod::GetSteamManager()->static_fields->Instance->fields.originalLobbyOwnerId.fields.m_SteamID;

		//SocketServer::SendLobbyInfo(lobbyId);

		SocketServer::Emit("currentLobby:" + std::to_string(currentLobby));
		SocketServer::Emit("originalLobbyOwnerId:" + std::to_string(originalLobbyOwnerId));

		Mod::AppendLocalChatMessage(0, "currentLobby", std::to_string(currentLobby));
		Mod::AppendLocalChatMessage(0, "originalLobbyOwnerId", std::to_string(originalLobbyOwnerId));

	}
	*/


	if (Server::m_LobbyOwner->m_ClientId == fromClient) {
		if (!Server::m_LobbyOwner->m_HideMessages) {
			Mod::AppendLocalChatMessage(2, Server::m_LobbyOwner->GetDisplayName(), message->toCPPString().c_str());
		}
	}

	Chat::ProcessRawMessage(fromClient, message->toCPPString());
	//ServerSend_SendChatMessage->original(fromClient, message);
}

//-------------------------------------------------

auto LobbyManager_BanPlayer = new HookFunction<void, void*, uint64_t, const void*>(13701920);
void Template_LobbyManager_BanPlayer(void* _this, uint64_t steamId, const void* method)
{
	if (Mod::allowedBanId == steamId) {
		LobbyManager_BanPlayer->original(_this, steamId, method);
	}
}

auto LobbyManager_AddPlayerToLobby = new HookFunction<void, void*, void*>(13700096);
void Template_LobbyManager_AddPlayerToLobby(void* _this, void* CSteamID)
{
	uintptr_t ptr1 = (uintptr_t)_this;
	uintptr_t ptr2 = ptr1 + 0x18;
	long long lobbyId = *(long long*)ptr2;

	LobbyManager_AddPlayerToLobby->original(_this, CSteamID);
	PlayerHooks::AddedToLobby((long long)CSteamID);
}

auto LobbyManager_RemovePlayerFromLobby = new HookFunction<void, void*, void*>(13705168);
void Template_LobbyManager_RemovePlayerFromLobby(void* _this, void* CSteamID)
{
	LobbyManager_RemovePlayerFromLobby->original(_this, CSteamID);
	PlayerHooks::OnPlayerRemovedFromLobby((long long)CSteamID);
}

//-------------------------------------------------


auto ChatBox_AddMessage = new HookFunction<void, void*, String*, Color, void*>(0x00F26A20);
void Template_ChatBox_AddMessage(void* _this, String* message, Color c, void* method) {
	std::cout << "message: " << message->toCPPString() << std::endl;
	std::cout << "color: " << c.r  << ", " << c.g << ", " << c.b << ", " << c.a << std::endl;
	ChatBox_AddMessage->original(_this, message, c, method);
}

auto ChatBox_AppendMessage = new HookFunction<void, void*, uint64_t, String*, String*, void*>(15681968);
void Template_ChatBox_AppendMessage(void* _this, uint64_t fromClient, String* message, String* username, void* method)
{
	//if (fromClient == 0) return;

	ChatBox_AppendMessage->original(_this, fromClient, message, username, method);
}

/*auto Send_SetCosmetics = new HookFunction<u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C*, u10A4u10A5u1099u10A3u1099u109Fu10A1u10A5u10A2u10A8u10A0_u109Au109Au10A4u109Fu10A0u1099u109Du109Eu109Eu10A1u109E__Array, void*>(15599600);
void Template_Send_SetCosmetics(u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C* a, u10A4u10A5u1099u10A3u1099u109Fu10A1u10A5u10A2u10A8u10A0_u109Au109Au10A4u109Fu10A0u1099u109Du109Eu109Eu10A1u109E__Array b, void* method) {
	u10A4u10A5u1099u10A3u1099u109Fu10A1u10A5u10A2u10A8u10A0_u109Au109Au10A4u109Fu10A0u1099u109Du109Eu109Eu10A1u109E c = **b.vector;
	std::cout << c.fields.u10A1u10A3u10A2u109Du10A5u109Du109Fu109Au1099u109Cu10A0.m_iDefinition.m_SteamItemDef << std::endl;
	std::cout << c.fields.u10A1u10A3u10A2u109Du10A5u109Du109Fu109Au1099u109Cu10A0.m_itemId.m_SteamItemInstanceID << std::endl;
	std::cout << c.fields.u10A1u10A3u10A2u109Du10A5u109Du109Fu109Au1099u109Cu10A0.m_unFlags << std::endl;
	std::cout << c.fields.u10A1u10A3u10A2u109Du10A5u109Du109Fu109Au1099u109Cu10A0.m_unQuantity << std::endl;
	Send_SetCosmetics->original(a, b, method);
}*/


auto ChatBox_Update = new HookFunction<void, void*, void*>(15689360);
void Template_ChatBox_Update(void* _this, void* method)
{
	ChatBox_Update->original(_this, method);
	//auto now = std::chrono::system_clock::now();
	//std::chrono::duration<float, std::milli> diff = now - Server::m_LastUpdatedTime;
	//auto dt = diff.count();
	float dt = Mod::m_get_deltaTime() * 1000;
	Server::Update(dt);

	//Server::Update(dt > 16 ? 16 : dt); //CHANGE LATER
	
	//Server::m_LastUpdatedTime = now;
}

//-------------------------------------------------



//-------------------------------------------------


/*auto ServerHandle_TryInteract = new HookFunction<uint64_t, Packet*>(12506752);
void Template_ServerHandle_TryInteract(uint64_t fromClient, Packet* packet)
{
	char buffer[256];
	sprintf_s(buffer, "Interact from= %lld", fromClient);
	Mod::SendChatMessage(1, buffer);

	ServerHandle_TryInteract->original(fromClient, packet);
}*/


//-------------------------------------------------



//-------------------------------------------------

auto PlayerManager_SetPlayer = new HookFunction<void, u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C*, uint64_t, int32_t, bool, const void*>(15599840);
void Template_PlayerManager_SetPlayer(u10A5u109Au109Eu109Eu1099u10A8u10A8u109Au109Du109Fu109C* playerManager, uint64_t steamId, int32_t playerId, bool streamerMode, const void* method)
{
	PlayerManager_SetPlayer->original(playerManager, steamId, playerId, streamerMode, method);

	PlayerHooks::TryAddPlayer(steamId, playerId, playerManager);
}