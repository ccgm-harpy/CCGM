#pragma once

#pragma once
#include "pch.h"
#include "Player.h"
#include "Message.h"
#include "Command.h"

class Chat {
private:
	static void ProcessCommand(Player* player, Message* message, Command* command, std::string sentByUser = "", std::string sentById = "");
public:
	static std::vector<Message*> m_Messages;

	static Message* SendServerMessage(std::string text);
	static void SendAllMessagesInQuery();
	static void Update(float dt);
	static void ProcessRawMessage(long long clientId, std::string text, std::string discordUserName = "", std::string discordUserId = "");
	static void ProcessMessage(Message* message, std::string discordUserName = "", std::string discordUserId = "");
	static void RegisterCommands();
	static std::string Capitalize(std::string str);
	static void ReportPlayerBan(Player* moderatorPlayer, Player* targetPlayer, std::string banReason, std::string banTime, std::string discordUserName = "", std::string discordUserId = "");
};