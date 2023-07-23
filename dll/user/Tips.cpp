#include "pch-il2cpp.h"

#include "Server.h"
#include "Tips.h"
#include "Chat.h"
#include "Mod.h"
#include "gamemodes/ReadyUp.h"
#include "HTTP.h"

float Tips::tipTime = 0;
int Tips::tipServerIndex = 1;
int Tips::tipIndex = 0;
bool Tips::addExtraLoadTime = true;
float Tips::requiredPlayersMessageTime = 0;
bool Tips::incrementTipTime = false;

void Tips::IncrementTipTime(float dt) {
	if(Tips::incrementTipTime) {
		Tips::tipTime += dt;
		Tips::SendTip();
	}

	if (ReadyUp::sendRequiredPlayersMessage && !ReadyUp::sentRequiredPlayersMessage) {
		Tips::requiredPlayersMessageTime += dt;
	}
}

void Tips::SendTip() {
	if (Tips::requiredPlayersMessageTime >= Server::second * 5) {
		std::cout << "Turning off!" << std::endl;
		Tips::requiredPlayersMessageTime = 0;
		ReadyUp::sendRequiredPlayersMessage = false;
		ReadyUp::sentRequiredPlayersMessage = true;
		Chat::SendServerMessage("(Need " + std::to_string(Server::ccgmConfigBase["winsPlayerRequirement"].get<int>()) + " players for auto start!)");
	}

	if (Tips::tipTime >= Server::second * 4 && !Server::InLobby && Tips::addExtraLoadTime) {
		float time = 10.0f;
		Mod::SetCurrentGameModeTime(time);
		Tips::addExtraLoadTime = false;
	}

	if (Tips::tipTime >= Server::second * 6) {
		Tips::incrementTipTime = false;
		Tips::tipTime = 0;
		auto tips = Server::ccgmConfig["tips"];

		if (Tips::tipIndex > tips.size()) {
			Tips::tipIndex = 0;
		}

		if (Tips::tipIndex < tips.size()) {
			std::string tip = tips[Tips::tipIndex];
			std::string replaceString = "{discordInvite}";
			auto discordInvite = HTTP::GetBody("/discord_invite");

			std::string line = "";

			for (auto c : tip) {
				if (c == '\n') {
					Chat::SendServerMessage(line);
					line = "";
				}
				else {
					line += c;
				}

				if (line.find(replaceString) != std::string::npos) {
					line.replace(line.find(replaceString), sizeof(replaceString) - 1, discordInvite);
				}
			}

			Chat::SendServerMessage(line);
		}
		
		if (Tips::tipIndex == tips.size()) {
			Chat::SendServerMessage("This server is powered by CCGM.");
			Chat::SendServerMessage("Visit dsc.gg/ccgm to host your own servers!");
		}

		Tips::tipIndex += 1;
	}
}