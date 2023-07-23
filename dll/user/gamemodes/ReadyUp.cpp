#include "pch-il2cpp.h"
#include "ReadyUp.h"
#include "Server.h"
#include "Chat.h"
#include "Mod.h"
#include "Tips.h"

bool sent120Warn = false;
bool sent60Warn = false;
bool sent30Warn = false;
bool sent10Warn = false;
bool sent3Warn = false;
float startTime = 0;

bool ReadyUp::sendRequiredPlayersMessage = false;
bool ReadyUp::sentRequiredPlayersMessage = false;

void ReadyUp::IncrementAutoStart(float dt) {
	if (Server::InLobby) {
		startTime += dt;

		if (PlayerUtils::FindPlayers("*").size() >= Server::ccgmConfigBase["winsPlayerRequirement"]) {
			if (startTime >= Server::minute * 2 + Server::second * 10 && !sent3Warn) {
				startTime = 0;
				Server::InLobby = false;
				sent3Warn = true;
				Chat::SendServerMessage("(STARTING GAME IN 3 SECONDS)");
				Mod::StartGame();
			}
			else if (startTime >= Server::minute * 2 && !sent10Warn) {
				sent10Warn = true;
				Chat::SendServerMessage("(STARTING GAME IN 10 SECONDS)");
			}
			else if (startTime >= Server::minute + Server::second * 40 && !sent30Warn) {
				sent30Warn = true;
				Chat::SendServerMessage("(STARTING GAME IN 30 SECONDS)");
			}
			else if (startTime >= Server::minute + Server::second * 10 && !sent60Warn) {
				sent60Warn = true;
				Chat::SendServerMessage("(STARTING GAME IN 1 MINUTE)");
			}
			else if (startTime >= Server::second * 10 && !sent120Warn) {
				sent120Warn = true;
				Chat::SendServerMessage("(STARTING GAME IN 2 MINUTES)");
			}
		}
		else {
			if (!ReadyUp::sentRequiredPlayersMessage && !ReadyUp::sendRequiredPlayersMessage) {
				ReadyUp::sendRequiredPlayersMessage = true;
			}
			ReadyUp::ResetTimers();
		}
	}
	else {
		Tips::requiredPlayersMessageTime = 0;
		ReadyUp::sendRequiredPlayersMessage = false;
		ReadyUp::ResetTimers();
	}
}

void ReadyUp::ResetTimers() {
	startTime = 0;
	sent60Warn = false;
	sent30Warn = false;
	sent10Warn = false;
	sent3Warn = false;
}