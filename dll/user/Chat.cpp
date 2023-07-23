#include "pch-il2cpp.h"
#include "Chat.h"
#include "Server.h"
#include "Mod.h"
#include "Commands.h"
#include <string>
#include <nlohmann/json.hpp>
#include "httplib.h"
#include "gamemodes/BlockDrop.h"
#include "weapons.h"
#include "gamemodes/LightsOut.h"
#include "gamemodes/ReadyUp.h"
#include "gamemodes/Chaos.h"
#include "ChatCommands.h"
#include <ctime>
#include "HTTP.h"

// Vector of messages
// Useful for adding multiple messages for processing
std::vector<Message*> Chat::m_Messages;

// Reports in game chat information about a player being banned
void Chat::ReportPlayerBan(Player* moderatorPlayer, Player* targetPlayer, std::string banReason, std::string banTime, std::string discordUserName, std::string discordUserId) {
	if (discordUserName.size()) {
		Chat::SendServerMessage("PLAYER " + targetPlayer->GetSelector() + " WAS BANNED BY: " + static_cast<std::string>(discordUserName.size() ? discordUserName : moderatorPlayer->GetSelector()));
		Chat::SendServerMessage("BAN TIME: " + banTime);
		Chat::SendServerMessage("REASON: " + banReason);
	}
}

// Sends a message from server in game chat
Message* Chat::SendServerMessage(std::string text) {
	Message* message = new Message((long long)1, text);

	m_Messages.push_back(message);

	return message;
}

// Iterates through the messages vector and sends all of them
void Chat::SendAllMessagesInQuery() {
	for (size_t i = 0; i < m_Messages.size(); i++)
	{
		Message* message = m_Messages[i];
		std::string content = message->m_Content;

		if (message->m_Player != NULL) { // Make sure the message is from a player
			auto player = message->m_Player;

			std::string additionalContent = ""; // Additional content to be sent with a message

			// Check if a player is admin, mod or user and add it to additionalContent
			bool isAdmin = std::find(player->m_PlayerConfig["perms"].begin(), player->m_PlayerConfig["perms"].end(), "admin") != player->m_PlayerConfig["perms"].end();
			bool isMod = std::find(player->m_PlayerConfig["perms"].begin(), player->m_PlayerConfig["perms"].end(), "ban") != player->m_PlayerConfig["perms"].end();

			std::cout << isAdmin << isMod << std::endl;

			if (isAdmin) {
				additionalContent += "(Admin)";
			}
			else if (isMod) {
				additionalContent += "(Mod)";
			}
			else {
				additionalContent += "(User)";
			}

			// Add aliveState and playerIdsInChat to additionalContent if enabled in the ccgm_config
			if (Server::ccgmConfigBase["aliveStateInChat"]) additionalContent += player->m_IsAlive ? "" : "(dead)";
			if (Server::ccgmConfigBase["playerIdsInChat"]) additionalContent += "[" + std::to_string(player->m_PlayerId) + "] ";
			
			// Put everything together as the message content
			// Messages in chat will output like this:
			// (Perm)(aliveState)[playerId] content
			additionalContent += content;
			content = additionalContent;
		}

		// Check if the message is marked as private
		if (message->m_SendType == MessageSendType::FORCE_PRIVATE) {
			// Appends a message to chat as private
			Mod::AppendLocalChatMessage(2, "[PRIVATE]", content);
		}

		// Send a normal chat message
		if (message->m_SendType == MessageSendType::NORMAL || message->m_SendType == MessageSendType::FORCE_SEND) {
			Mod::SendChatMessage(message->m_FromClient, content);
		}
	}

	// We've processed all of the messages, so clear the vector out
	m_Messages.clear();
}

// Sends all messages in the messages list and updates message rate limiting timers
void Chat::Update(float dt) {
	Chat::SendAllMessagesInQuery();
	PlayerUtils::UpdatePlayerMessageTimers(dt);
}

// Creates a message object and forwards it for further processing
void Chat::ProcessRawMessage(long long clientId, std::string text, std::string discordUser, std::string sentById) {
	Message* message = new Message(clientId, text);

	// Make sure the message is from a player and they're not muted
	if (PlayerUtils::HasPlayer(clientId)) {
		message->m_Player = PlayerUtils::GetPlayer(clientId);

		if (message->m_Player->IsMuted()) return;
	}
	else {
		return;
	}

	// Make sure player isn't rate limited
	if (message->m_Player->allowMessageTime >= Server::second) {
		// Make sure the hideMessages bool matches the message send type
		if (message->m_Player->m_HideMessages) {
			if (message->m_SendType == MessageSendType::NORMAL) {
				message->m_SendType = MessageSendType::FORCE_PRIVATE;
			}
		}

		// Add message to vector
		m_Messages.push_back(message);

		// Send message to be processed and outputs any errors that happen
		try
		{
			ProcessMessage(message, discordUser, sentById);
		}
		catch (const std::runtime_error& re)
		{
			std::string errstr = re.what();
			Chat::SendServerMessage("Runtime Error: " + errstr);
		}
		catch (const std::exception& ex)
		{
			std::string errstr = ex.what();
			Chat::SendServerMessage("Error: " + errstr);

		}
		catch (...)
		{
			Chat::SendServerMessage("Error");
		}

		// Send all the messages stored in the messages vector
		SendAllMessagesInQuery();
	}
	
}

// Process a message, if the message is a command submit it for command processing
void Chat::ProcessMessage(Message* message, std::string sentByUser, std::string sentById) {
	Player* player = message->m_Player;

	player->allowMessageTime = 0; // Reset their message ratelimiting timer
	std::string content = message->m_Content;

	// Log the chat message
	HTTP::ChatLog(player, content);

	std::cout << "[Chat] ProcessMessage " << player->m_Username << " (" << player->m_ClientId << "): " << content << "'" << std::endl;

	// Check if the message is a command
	if (content.rfind("!", 0) == 0) {
		// Create the command and send it for processing
		Command* command = new Command(content);
		ProcessCommand(player, message, command, sentByUser, sentById);
	}
}

// Check if a player has the permission required to perform a command
bool TestPlayerPermission(Player* player, CommandInfo* info) {
	if (player->HasPermission("admin")) return true; // Admins always have permission

	// Return true or false if player has permission to use a command
	int requiredPermissions = (int)info->m_Permissions.size();
	int playerMatchPerms = 0;

	for (size_t i = 0; i < requiredPermissions; i++)
	{
		if (player->HasPermission(info->m_Permissions[i])) {
			playerMatchPerms++;
		}
	}

	return requiredPermissions == playerMatchPerms;
}

// Register our commands to be used, we will use this information
// to quickly know if a command exists and the permissions required for that command
// Commands::RegisterCommand(commandName, requiredPerm, hidden);
// Empty perm requirement means everyone has permission to use it
void Chat::RegisterCommands() {
	Commands::RegisterCommand("r", "r", true);
	Commands::RegisterCommand("v", "v", true);
	Commands::RegisterCommand("win", "win", true);
	Commands::RegisterCommand("bc", "bc", true);
	Commands::RegisterCommand("perm", "perm", true);
	Commands::RegisterCommand("ban", "ban", true);
	Commands::RegisterCommand("kick", "kick", true);
	Commands::RegisterCommand("kill", "kill", true);
	Commands::RegisterCommand("time", "time", true);
	Commands::RegisterCommand("mute", "mute", true);
	Commands::RegisterCommand("god", "god", true);
	Commands::RegisterCommand("tp", "tp", true);
	Commands::RegisterCommand("help", "");
	Commands::RegisterCommand("page", "page", true);
	Commands::RegisterCommand("respawn", "respawn", true);
	Commands::RegisterCommand("jumppunch", "jumppunch", true);
	Commands::RegisterCommand("superpunch", "superpunch");
	Commands::RegisterCommand("forcefield", "forcefield", true);
	Commands::RegisterCommand("light", "light", true);
	Commands::RegisterCommand("pos", "pos", true);
	Commands::RegisterCommand("points", "");
	Commands::RegisterCommand("weapons", "");
	Commands::RegisterCommand("playerinfo", "");
	Commands::RegisterCommand("discord", "");
	Commands::RegisterCommand("wins", "");
	Commands::RegisterCommand("swap", "swap", true);
	Commands::RegisterCommand("kd", "");
	Commands::RegisterCommand("report", "");
	Commands::RegisterCommand("start", "start");
	Commands::RegisterCommand("mapmode", "mapmode");
	Commands::RegisterCommand("mapmodeforce", "mapmodeforce");
	Commands::RegisterCommand("chaos", "chaos");
	Commands::RegisterCommand("bird", "");
	Commands::RegisterCommand("dropweapon", "dropweapon");
	Commands::RegisterCommand("giveweapon", "giveweapon");
	Commands::RegisterCommand("dw", "dw");
	Commands::RegisterCommand("gw", "gw");
	Commands::RegisterCommand("purchaseweapon", "", true);
	Commands::RegisterCommand("unmute", "unmute");
	Commands::RegisterCommand("tempban", "tempban");
}

// Capitalizes a string
std::string Chat::Capitalize(std::string str) {
	for (auto& c : str) {
		c = std::toupper(c);
	}
	return str;
}

// Processes a command
void Chat::ProcessCommand(Player* player, Message* message, Command* command, std::string discordUserName, std::string discordUserId) {
	// When discordUserName is set, this means the message is being processed from Discord
	if (discordUserName.size()) {
		SendServerMessage("Recieved command from Discord user: " + discordUserName);
	}
	
	// Sets the weapon ID associated with the command
	// If the command isn't associated with a weaponID, the value will be -1
	int commandWeaponId = Weapons::IsWeaponCommand(command->GetCmd());

	// If command has an associated weaponId
	if (commandWeaponId >= 0) {
		// Changes the command to be a "!purchaseweapon weaponID" command
		command = new Command("!purchaseweapon " + std::to_string(commandWeaponId));
	}

	// If the player has admin permissions
	bool isAdmin = player->HasPermission("admin");
	
	CommandInfo* cmdInfo;

	// Check if command exists and get it's cmdInfo
	if (Commands::GetCommandInfo(command->GetCmd(), cmdInfo)) {
		if (!cmdInfo->m_Enabled) {
			if (!isAdmin) return;
		}

		// Test whether player has permissions to use the command or not
		bool hasPermission = TestPlayerPermission(player, cmdInfo);

		// If the user has permission, find out what command was used and execute it
		if (hasPermission) { 
			if (command->Check("dropweapon") || command->Check("giveweapon") || command->Check("dw") || command->Check("gw")) {
				ChatCommands::ModeratorGiveWeapon(command, player);
			}

			if (command->Check("bird")) {
				SendServerMessage("Is the word.");
			}

			if (command->Check("chaos")) {
				ChatCommands::ToggleChaos();
			}

			if (command->Check("mapmodeforce")) {
				ChatCommands::MapModeForce(command);
			}

			if (command->Check("mapmode")) {
				ChatCommands::MapMode(command);
			}

			if (command->Check("report")) {
				ChatCommands::Report(command, player);
			}

			if (command->Check("discord")) {
				ChatCommands::Discord();
			}

			if (command->Check("wins")) {
				ChatCommands::Wins(command, player);
			}

			if (command->Check("start")) {
				ChatCommands::Start();
			}

			if (command->Check("swap")) {
				ChatCommands::Swap();
			}

			if (command->Check("playerinfo")) {
				ChatCommands::PlayerInfo(command, player);
			}

			if (command->Check("weapons") && Chaos::modeEnabled) {
				Weapons::ShowWeapons();
			}

			if (command->Check("points") && Chaos::modeEnabled) {
				ChatCommands::Points(player);
			}

			if (command->Check("help")) {
				ChatCommands::Help();
			}

			if (command->Check("jumppunch")) {
				ChatCommands::JumpPunch(player);
			}

			if (command->Check("superpunch")) {
				ChatCommands::SuperPunch(player);
			}

			if (command->Check("forcefield")) {
				ChatCommands::ForceField(player);
			}

			if (command->Check("god")) {
				ChatCommands::God(player);
			}

			if (command->Check("v")) {
				ChatCommands::Vanish(player, isAdmin, message);
			}

			if (command->Check("win")) {	
				ChatCommands::Win(command);
			}

			if (command->Check("r")) {
				Mod::RestartGame();
			}

			if (command->Check("purchaseweapon")) {
				ChatCommands::PurchaseWeapon(command, player);
			}

			if (command->Check("tp")) {
				ChatCommands::TP(command, player, isAdmin);
			}

			if (command->Check("respawn")) {
				ChatCommands::Respawn(command, player, isAdmin);
			}

			if (command->Check("perm")) {
				ChatCommands::Perm(command, player);
			}

			if (command->Check("time")) {
				ChatCommands::Time(command);
			}

			if (command->Check("bc")) {
				ChatCommands::BroadCast(command, message);
			}

			if (command->Check("ban")) {
				ChatCommands::Ban(false, command, player, discordUserName, discordUserId);
			}

			if (command->Check("tempban")) {
				ChatCommands::Ban(true, command, player, discordUserName, discordUserId);
			}

			if (command->Check("kick")) {
				ChatCommands::Kick(command, player);
			}

			if (command->Check("mute")) {
				ChatCommands::Mute(command, player);
			}

			if (command->Check("unmute")) {
				ChatCommands::Unmute(command, player);
			}
			
			if (command->Check("kd")) {
				ChatCommands::KD(command, player);
			}

			if (command->Check("kill")) {
				ChatCommands::Kill(command, player);
			}		

			if (command->Check("light")) {
				ChatCommands::Light();
			}

			if (command->Check("pos")) {
				ChatCommands::Position(command);
			}
		}
		else {
			// Home boy got no perms ;(
			SendServerMessage("No perm");
		}
	}
	else {
		// Command not registered
		SendServerMessage("Command '" + command->GetCmd() + "' not found");
	}
}