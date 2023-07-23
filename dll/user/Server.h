#pragma once
#include "pch.h"
#include "Player.h"
#include <nlohmann/json.hpp>
#include <cstring>
#include "httplib.h"

class Server {
public:
	static int gameStartPlayerCount;
	static std::string m_Version;
	static Player* m_LobbyOwner;
	static float second;
	static float minute;
	static httplib::Client httpClient;
	static std::string activeServerName;
	static json ccgmConfig;
	static json ccgmConfigBase;
	static bool InLobby;
	static bool RoundStarted;
	static int GameMode;
	static int GameMap;
	static bool modLoadMap;

	static void Init();
	static void Update(float dt);
};