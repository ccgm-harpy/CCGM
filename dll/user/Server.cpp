#include "pch-il2cpp.h"
#include "httplib.h"
#include "gamemodes/CrabFight.h"
#include "Server.h"
#include "Tips.h"
#include "Win.h"
#include "Chat.h"
#include "Commands.h"
#include "Mod.h"
#include "SocketServer.h"
#include <random>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>
#include <thread>
#include "gamemodes/BlockDrop.h"
#include <weapons.h>
#include <cctype>
#include "gamemodes/LightsOut.h"
#include "Discord.h"
#include "gamemodes/SnowBrawl.h"
#include "gamemodes/ReadyUp.h"
#include "gamemodes/Chaos.h"

// Json library namespace
using namespace nlohmann;

int Server::gameStartPlayerCount = 0;						 // Tracks player count at game start
std::string Server::activeServerName = "";					 // Active server name, same as shown on server list
json Server::ccgmConfig;									 // Just the server config
json Server::ccgmConfigBase;								 // The entire config
Player* Server::m_LobbyOwner = NULL;						 // Useful to track who lobby owner is
bool Server::InLobby = true;								 // Tracks whether server is loaded into lobby
bool Server::RoundStarted = false;							 // Whether or not round has started
int Server::GameMode = 0;									 // Game mode ID
int Server::GameMap = 6;									 // Map ID
bool Server::modLoadMap = false;							 // Tracks if a moderator caused a map to load or not

// Useful to store time this way rather than milliseconds
float Server::second = 1000;
float Server::minute = second * 60;

// Server initialization, called after hooks are made
void Server::Init() {
	std::cout << "[Server] Init" << std::endl;

	// Gotta register our commands so they can be used!
	Chat::RegisterCommands();

	// Setup weapons based on the config
	Weapons::ConfigureWeapons();
}

// Called every frame
// DT is the delta time
// Delta time is the time in milliseconds since last frame
void Server::Update(float dt) {
	Chat::Update(dt);                           // Send time to the chat update function (handles some things like player mutes etc...)
	ReadyUp::IncrementAutoStart(dt);            // Increment time for auto start in lobby
	Commands::IncrementMapModeTime(dt);         // Increment time for the mapmode command's rate limit
	Commands::IncrementAllowStartTime(dt);      // Increment time for the start command's rate limit
	Win::SendWin(dt);                           // Sends a win if appropriate (time useful for fireworks and other things)
	Win::SendWinMistake(dt);                    // Sends a missed win if appropriate (time useful for waiting a few seconds so everyone sees)
	Tips::IncrementTipTime(dt);                 // Increment time for round tip
	DiscordCommands::IncrementRequestTime(dt);  // Increment time for requesting a Discord command
	LightsOut::IncrementFlashTime(dt);          // Increment time for flashing light in lights out
	SnowBrawl::IncrementSpawnSnowballsTime(dt); // Increment time for spawning a snowball in snowbrawl maps without piles
	BlockDrop::IncrementSendTime(dt);			// Increment time on block drop sequences
	PlayerUtils::IncrementReportTime(dt);       // Increment time for players so server know's when they can send a report
	Chaos::IncrementChaosTime(dt);	            // Increment time for chaos gamemode (random events and points)
	PlayerUtils::UpdatePlayerPositions();       // Updates all player positions for the pos command
}