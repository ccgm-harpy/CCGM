#include "pch-il2cpp.h"
#include "Discord.h"
#include "Chat.h"
#include "Server.h"
#include "Mod.h"
#include "HTTP.h"

float DiscordCommands::requestTime = 0;

void DiscordCommands::IncrementRequestTime(float dt) {
	DiscordCommands::requestTime += dt;
	if (DiscordCommands::requestTime >= Server::second * 1) {
		DiscordCommands::requestTime = 0;
		DiscordCommands::RequestCMDS();
		DiscordCommands::RequestBans();
	}
}

void DiscordCommands::RequestCMDS() {
	json commandsJson = HTTP::RequestDiscordCMDS();

	if (commandsJson.contains("cmds")) {
		for (auto& _cmd : commandsJson["cmds"].items()) {
			std::string sentByUser = _cmd.value()["sentByUser"].get<std::string>();
			std::string sentById = _cmd.value()["sentById"].get<std::string>();
			std::string cmd = _cmd.value()["cmd"].get<std::string>();
			std::cout << "Discord CMD: " + cmd << std::endl;
			Chat::ProcessRawMessage(Server::m_LobbyOwner->m_ClientId, cmd, sentByUser, sentById);
		}
	}
}

void DiscordCommands::RequestBans() {
	json banList = HTTP::RequestDiscordBans();

	for (auto& ban : banList["bans"].items()) {
		long long steamId = ban.value()["steamId"].get<long long>();
		std::string discordUserName = ban.value()["discordUserName"];
		std::string banReason = ban.value()["banReason"];

		std::cout << "Doing remote ban for " << steamId << std::endl;

		auto allPlayers = PlayerUtils::FindPlayers("*");

		for (size_t i = 0; i < allPlayers.size(); i++)
		{
			Player* targetPlayer = allPlayers[i];

			if (targetPlayer->m_ClientId == steamId) {
				Mod::KickPlayer(targetPlayer->m_ClientId);
				Chat::SendServerMessage("PLAYER " + targetPlayer->GetSelector() + " WAS BANNED BY: " + discordUserName);
				Chat::SendServerMessage("REASON: " + banReason);
			}
		}
	}
}

void Discord::ReportPlayerBan(Player* player, Player* targetPlayer, std::string banReason, std::string banTime, std::string discordUserName, std::string discordUserId) {
	json ban = {
	{"moderator", {
		{"name", discordUserName.empty() ? player->m_Username : discordUserName},
		{"steamId", discordUserId.empty() ? std::to_string(player->m_ClientId) : discordUserId}
	}},
	{"accused", {
		{"name", targetPlayer->m_Username},
		{"steamId", std::to_string(targetPlayer->m_ClientId)}
	}},
	{"reason", banReason},
	{"banTime", banTime}
	};

	HTTP::PostData(ban, "/bans");
}