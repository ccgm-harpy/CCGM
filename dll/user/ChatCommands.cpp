#include "pch-il2cpp.h"
#include "ChatCommands.h"
#include "Command.h"
#include "Chat.h"
#include "Player.h"
#include "Mod.h"
#include "Server.h"
#include "Message.h"
#include "gamemodes/Chaos.h"
#include "weapons.h"
#include "Commands.h"
#include "gamemodes/LightsOut.h"
#include "HTTP.h"

void ChatCommands::Position(Command* command) {
	std::string selector = "*";

	if (command->HasArg(0)) {
		selector = command->GetArgString(0);
	}

	auto players = PlayerUtils::FindPlayers(selector);
	for (size_t i = 0; i < players.size(); i++)
	{
		auto targetPlayer = players[i];
		
		Chat::SendServerMessage(targetPlayer->GetSelector() + "'s position: " + Mod::FormatVector(targetPlayer->m_Position).c_str());
	}
}

void ChatCommands::Light() {
	try {
		LightsOut::lightState = !LightsOut::lightState;
		Mod::ToggleLights(LightsOut::lightState);
	}
	catch (...) {
		Chat::SendServerMessage("Wrong gamemode");
	}
}

void ChatCommands::Kill(Command* command, Player* moderatorPlayer) {
	if (Server::GameMode != 18) {
		if (command->HasArg(0)) {
			auto selector = command->GetArgString(0);

			auto players = PlayerUtils::FindPlayers(selector);
			for (size_t i = 0; i < players.size(); i++)
			{
				auto targetPlayer = players[i];
				targetPlayer->m_IsAlive = false;
				Mod::KillPlayer(targetPlayer->m_ClientId);
				HTTP::KillCommandLog(moderatorPlayer, targetPlayer);
			}
		}
	}
	else {
		Chat::SendServerMessage("Cannot use !kill on this mode");
	}
}

void ChatCommands::KD(Command* command, Player* player) {
	std::string selector = "#" + std::to_string(player->m_PlayerId);

	if (command->HasArg(0)) {
		selector = command->GetArgString(0);
	}

	auto players = PlayerUtils::FindPlayers(selector);

	if (players.size()) {
		auto targetPlayer = players[0];
		std::string shortName = Server::ccgmConfig["shortName"];

		int kills = targetPlayer->m_PlayerConfig["kills"][shortName];
		int deaths = targetPlayer->m_PlayerConfig["deaths"][shortName];

		Chat::SendServerMessage(selector + " has " + std::to_string(kills) + " kills / " + std::to_string(deaths) + " deaths");
	}
	else {
		Chat::SendServerMessage("Syntax: !kd #playerNum");
	}
}

void ChatCommands::God(Player* moderatorPlayer) {
	moderatorPlayer->m_GodEnabled = !moderatorPlayer->m_GodEnabled;
	Chat::SendServerMessage(std::string(moderatorPlayer->m_GodEnabled ? "on" : "off"));
}

void ChatCommands::ForceField(Player* player) {
	player->m_ForceFieldEnabled = !player->m_ForceFieldEnabled;
	Chat::SendServerMessage(std::string(player->m_ForceFieldEnabled ? "on" : "off"));
}

void ChatCommands::SuperPunch(Player* player) {
	player->m_SuperPunchEnabled = !player->m_SuperPunchEnabled;
	Chat::SendServerMessage(std::string(player->m_SuperPunchEnabled ? "on" : "off"));
}

void ChatCommands::JumpPunch(Player* player) {
	player->m_JumpPunchEnabled = !player->m_JumpPunchEnabled;
	Chat::SendServerMessage(std::string(player->m_JumpPunchEnabled ? "on" : "off"));
}

void ChatCommands::Help() {
	if (Chaos::modeEnabled) {
		Chat::SendServerMessage("\"!report #playerNumber reason\"");
		Chat::SendServerMessage("\"!playerinfo #playerNumber\"");
		Chat::SendServerMessage("\"!points\" \"!weapons\" \"!kd\" \"!wins\"");
		Chat::SendServerMessage("\"!discord\"");
	}
	else
	{
		Chat::SendServerMessage("\"!report #playerNumber reason\"");
		Chat::SendServerMessage("\"!playerinfo #playerNumber\"");
		Chat::SendServerMessage("\"!kd\" \"!wins\" \"!discord\"");
	}
}

void ChatCommands::Points(Player* player) {
	int points = player->m_PlayerConfig["points"];
	Chat::SendServerMessage("#" + std::to_string(player->m_PlayerId) + " has: " + std::to_string(points) + " points (max 15)\nSay !weapons to buy a weapon.");
}

void ChatCommands::PlayerInfo(Command* command, Player* player) {
	auto selector = "#" + std::to_string(player->m_PlayerId);

	if (command->HasArg(0)) {
		selector = command->GetArgString(0);
	}

	auto players = PlayerUtils::FindPlayers(selector);

	if (players.size() > 0) {
		auto targetPlayer = players[0];
		Chat::SendServerMessage("steamcommunity.com/profiles/" + std::to_string(targetPlayer->m_ClientId));
	}
	else {
		Chat::SendServerMessage("Syntax: !playerinfo #playerNumber");
	}
}

void ChatCommands::Swap() {
	auto players = PlayerUtils::FindPlayers("*a");
	std::vector<Player*>targetPlayers;

	for (size_t i = 0; i < players.size(); i++)
	{
		targetPlayers.push_back(players[i]);

		if (targetPlayers.size() > 1) {
			Mod::RespawnPlayer(targetPlayers[0]->m_ClientId, targetPlayers[1]->m_Position);
			Mod::RespawnPlayer(targetPlayers[1]->m_ClientId, targetPlayers[0]->m_Position);
			targetPlayers.clear();
		}
	}
}

void ChatCommands::Start() {
	if (Server::InLobby) {
		int requiredPlayers = Server::ccgmConfig["requiredStartPlayers"];

		if (PlayerUtils::FindPlayers("*").size() < requiredPlayers) {
			if (Commands::allowStartTime >= Server::ccgmConfigBase["startRateLimit"] * Server::second) {
				Mod::StartGame();
			}
			else {
				float secondsRemaining = (Server::ccgmConfigBase["startRateLimit"] * Server::second - Commands::allowStartTime) / Server::second;
				std::stringstream secondsRemainingString;
				secondsRemainingString << std::fixed << std::setprecision(2) << secondsRemaining;
				Chat::SendServerMessage("You must wait " + secondsRemainingString.str() + " seconds before using !start. (When less than " + std::to_string(requiredPlayers) + " players)");
			}
		}
		else {
			Mod::StartGame();
		}
	}
	else {
		Chat::SendServerMessage("You can only start the game in lobby (mapmode 6 0)");
	}
}

void ChatCommands::Wins(Command* command, Player* moderatorPlayer) {
	auto selector = "#" + std::to_string(moderatorPlayer->m_PlayerId);

	if (command->HasArg(0)) {
		selector = command->GetArgString(0);
	}

	auto players = PlayerUtils::FindPlayers(selector);

	if (players.size() > 0) {
		auto targetPlayer = players[0];
		int wins = targetPlayer->m_PlayerConfig["wins"][Server::ccgmConfig["shortName"].get<std::string>()];

		Chat::SendServerMessage("Player #" + std::to_string(targetPlayer->m_PlayerId) + " has " + std::to_string(wins) + " wins.");
	}
	else {
		Chat::SendServerMessage("Syntax: !wins #playerNumber");
	}
}

void ChatCommands::Discord() {
	auto discordInvite = HTTP::GetBody("/discord_invite");
	Chat::SendServerMessage("You can join here: " + discordInvite);
}

void ChatCommands::Report(Command* command, Player* moderatorPlayer) {
	if (command->HasArg(0)) {
		auto selector = command->GetArgString(0);
		std::vector<Player*> accusedPlayers = PlayerUtils::FindPlayers(selector);

		if (accusedPlayers.size()) {
			if (command->HasArg(1)) {
				if (moderatorPlayer->m_CanReport) {
					moderatorPlayer->m_CanReport = false;
					std::string reportReason = command->GetArgTextReport(1);
					HTTP::ReportPlayer(moderatorPlayer, accusedPlayers[0], reportReason);
					Chat::SendServerMessage("Report sent to our Discord");
					Chat::SendServerMessage("If you want to join say !discord");
				}
				else {
					float timeRemaining = (Server::ccgmConfigBase["reportRateLimit"] * Server::second - moderatorPlayer->m_ReportTime) / Server::second;
					std::stringstream timeRemainingString;
					timeRemainingString << std::fixed << std::setprecision(2) << timeRemaining;
					Chat::SendServerMessage("You must wait " + timeRemainingString.str() + " seconds before using !report");
				}
			}
			else {
				Chat::SendServerMessage("Syntax: !report #playerNumber reason");
			}
		}
		else {
			Chat::SendServerMessage("Syntax: !report #playerNumber reason");
		}
	}
	else {
		Chat::SendServerMessage("Syntax: !report #playerNumber reason");
	}
}

void ChatCommands::MapMode(Command* command) {
	if (Commands::mapModeTime >= Server::ccgmConfigBase["mapModeRateLimit"] * Server::second) {
		Commands::mapModeTime = 0;

		if (command->HasArg(0) && command->HasArg(0)) {
			try {
				int setMap = command->GetArgInt(0);
				int setMode = command->GetArgInt(1);

				if (setMap >= 0 && setMap <= 61 && setMode >= 0 && setMode <= 18) {
					Server::GameMap = setMap;
					Server::GameMode = setMode;
					Server::modLoadMap = true;
				}
			}
			catch (...) {
				Chat::SendServerMessage("Syntax: !mapmode 0-61 0-18");
			}
		}
		else {
			Chat::SendServerMessage("Syntax: !mapmode 0-61 0-18");
		}
	}
	else {
		float timeRemaining = (Server::ccgmConfigBase["mapModeRateLimit"] * Server::second - Commands::mapModeTime) / Server::second;
		std::stringstream timeRemainingString;
		timeRemainingString << std::fixed << std::setprecision(2) << timeRemaining;
		Chat::SendServerMessage("You must wait " + timeRemainingString.str() + " seconds before using !mapmode");
	}
}

void ChatCommands::MapModeForce(Command* command) {
	if (command->HasArg(0) && command->HasArg(0)) {
		try {
			int setMap = command->GetArgInt(0);
			int setMode = command->GetArgInt(1);

			if (setMap >= 0 && setMap <= 61 && setMode >= 0 && setMode <= 18) {
				Server::GameMap = setMap;
				Server::GameMode = setMode;
				Server::modLoadMap = true;
				Mod::LoadMap(Server::GameMap, Server::GameMode);
			}
		}
		catch (...) {
			Chat::SendServerMessage("Syntax: !mapmode 0-61 0-18");
		}
	}
	else {
		Chat::SendServerMessage("Syntax: !mapmode 0-61 0-18");
	}
}

void ChatCommands::ToggleChaos() {
	Chaos::modeEnabled = !Chaos::modeEnabled;

	if (Chaos::modeEnabled) {
		Chat::SendServerMessage("Chaos mode enabled.");
	}
	else {
		PlayerUtils::ClearAllPlayerPowers();
		Chat::SendServerMessage("Chaos mode disabled.");
	}
}

void ChatCommands::ModeratorGiveWeapon(Command* command, Player* moderatorPlayer) {
	if (!command->HasArg(0)) {
		return;
	}

	std::string weaponName = command->HasArg(1) ? command->GetArgString(1) : command->GetArgString(0);
	int weaponId = Weapons::IsWeaponCommand(weaponName);

	if (!Weapons::ValidWeaponId(weaponId)) {
		Chat::SendServerMessage("Invalid weapon ID!");
		return;
	}

	if (weaponId == -1) {
		Chat::SendServerMessage("Invalid weapon name, say !weapons to see a valid list.");
		return;
	}

	std::vector<Player*> players;

	if (command->HasArg(1)) {
		std::string selector = command->GetArgString(0);
		players = PlayerUtils::FindPlayers(selector);

		if (players.empty()) {
			Chat::SendServerMessage("No players found! Double check command.");
			return;
		}
	}
	else {
		players.push_back(moderatorPlayer);
	}

	bool dropWeapon = command->Check("dropweapon") || command->Check("dw");

	for (Player* targetPlayer : players) {
		if (dropWeapon) {
			Weapons::DropWeapon(targetPlayer->m_ClientId, weaponId);
		}
		else {
			Weapons::DropWeapon(targetPlayer->m_ClientId, weaponId);
			Weapons::GiveWeapon(targetPlayer->m_ClientId, weaponId);
		}

		bool gaveToSelf = (targetPlayer == moderatorPlayer);
		HTTP::ModeratorWeaponCommandLog(moderatorPlayer, targetPlayer, weaponName, gaveToSelf);
	}
}


void ChatCommands::Ban(bool banTemporary, Command* command, Player* moderatorPlayer, std::string discordUserName, std::string discordUserId) {
	if (command->HasArg(0)) {
		auto selector = command->GetArgString(0);
		auto players = PlayerUtils::FindPlayers(selector);

		for (size_t i = 0; i < players.size(); i++)
		{
			auto targetPlayer = players[i];

			if (targetPlayer->HasPermission("admin")) {
				Chat::SendServerMessage("Can't ban this player");
				continue;
			}

			if (targetPlayer->m_ClientId == moderatorPlayer->m_ClientId) {
				Chat::SendServerMessage("No can do, you cannot ban yourself.");
				continue;
			}

			std::string banTimeShort = banTemporary ? command->GetArgString(1) : "36500d";
			uint64_t banTimeSeconds = PlayerUtils::SecondsFromString(banTimeShort);
			std::string banTimeString = PlayerUtils::FormatSeconds(banTimeSeconds);
			std::string banReason = banTemporary ? command->GetArgTextReport(3): command->GetArgTextReport(2);

			if (banReason.size() > 0) {
				targetPlayer->Ban(moderatorPlayer, banReason, banTimeSeconds, discordUserName, discordUserId);
			}
			else {
				Chat::SendServerMessage("Must specify a ban reason!");
			}
		}
	}
}

void ChatCommands::Vanish(Player* moderatorPlayer, bool isAdmin, Message* message) {
	if (isAdmin) {
		moderatorPlayer->m_HideMessages = !moderatorPlayer->m_HideMessages;

		message->m_Content = "Vanish " + std::string(moderatorPlayer->m_HideMessages ? "ENABLED" : "DISABLED");
		message->m_SendType = MessageSendType::FORCE_PRIVATE;

	}
}

void ChatCommands::Win(Command* command) {
	if (command->HasArg(0) && command->HasArg(1)) {
		auto selector = command->GetArgString(0);
		long long money = command->GetArgULong(1);

		auto players = PlayerUtils::FindPlayers(selector);

		if (players.size() > 0) {
			auto targetPlayer = players[0];

			Mod::SendWinner(targetPlayer->m_ClientId, money);
		}
	}
}

void ChatCommands::PurchaseWeapon(Command* command, Player* player) {
	if (!Chaos::modeEnabled) {
		Chat::SendServerMessage("Cannot spawn weapons outside of chaos mode!");
		return;
	}

	if (!command->HasArg(0)) {
		Chat::SendServerMessage("Missing weapon ID!");
		return;
	}

	int weaponId = command->GetArgInt(0);

	if (!Weapons::ValidWeaponId(weaponId)) {
		Chat::SendServerMessage("Invalid weapon ID.");
		return;
	}

	if (!Weapons::WeaponBuyable(weaponId)) {
		Chat::SendServerMessage("That weapon is not allowed!");
		return;
	}

	if (!player->m_IsAlive) {
		Chat::SendServerMessage("You're dead. Don't be buyin weapons fam!");
		return;
	}

	Weapon weapon = Weapons::list[weaponId];
	int points = player->m_PlayerConfig["points"];

	if (points - weapon.price <= 0) {
		Chat::SendServerMessage("Player #" + std::to_string(player->m_PlayerId) + ", you don't have enough points");
		return;
	}

	points -= weapon.price;
	player->m_PlayerConfig["points"] = points;
	player->SaveConfig();

	if (weapon.ammo > 1)
	{
		Chat::SendServerMessage("Player #" + std::to_string(player->m_PlayerId) + " purchased a " + weapon.name + " " + std::to_string(weapon.ammo) + " ammo | " + std::to_string(points) + " points remaining");
	}
	else {
		Chat::SendServerMessage("Player #" + std::to_string(player->m_PlayerId) + " purchased a " + weapon.name + " | " + std::to_string(points) + " points remaining");
	}

	if (!weapon.tip.empty()) {
		Chat::SendServerMessage("(" + weapon.tip + ")");
	}

	if (Server::ccgmConfigBase["chaosWeaponPurchaseToInventory"]) {
		Weapons::GiveWeapon(player->m_ClientId, weapon.id);
	}
	else {
		Weapons::DropWeapon(player->m_ClientId, weaponId);
	}
}

void ChatCommands::TP(Command* command, Player* moderatorPlayer, bool isAdmin) {
	std::string selector = moderatorPlayer->GetSelector();

	Vector3 position({ 0, 0, 0 });

	if (command->HasArg(3)) {

		if (isAdmin) {

			selector = command->GetArgString(0);

			position.x = command->GetArgFloat(1);
			position.y = command->GetArgFloat(2);
			position.z = command->GetArgFloat(3);
		}
	}
	else {
		if (command->HasArg(2)) {

			position.x = command->GetArgFloat(0);
			position.y = command->GetArgFloat(1);
			position.z = command->GetArgFloat(2);
		}
	}


	auto players = PlayerUtils::FindPlayers(selector);

	for (size_t i = 0; i < players.size(); i++)
	{
		auto targetPlayer = players[i];
		Mod::RespawnPlayer(targetPlayer->m_ClientId, position);
		HTTP::ModeratorTeleportCommandLog(moderatorPlayer, targetPlayer);
	}
}

void ChatCommands::Respawn(Command* command, Player* moderatorPlayer, bool isAdmin) {
	if (Server::GameMode != 18) {
		auto selector = moderatorPlayer->GetSelector();

		if (moderatorPlayer->HasPermission("respawn.others") || isAdmin) {
			if (command->HasArg(0)) {
				selector = command->GetArgString(0);
			}
		}

		auto players = PlayerUtils::FindPlayers(selector);
		for (size_t i = 0; i < players.size(); i++)
		{
			Vector3 position = PlayerUtils::m_SpawnPosition;

			auto targetPlayer = players[i];
			Mod::RespawnPlayer(targetPlayer->m_ClientId, position);
			HTTP::ModeratorRespawnCommandLog(moderatorPlayer, targetPlayer);
		}
	}
	else {
		Chat::SendServerMessage("Cannot use !respawn on this mode");
	}
}

void ChatCommands::Unmute(Command* command, Player* moderatorPlayer) {
	if (command->HasArg(0) && command->HasArg(1)) {
		auto selector = command->GetArgString(0);

		auto players = PlayerUtils::FindPlayers(selector);
		for (size_t i = 0; i < players.size(); i++)
		{
			auto targetPlayer = players[i];

			if (targetPlayer->m_PlayerConfig["isMuted"]) {
				targetPlayer->m_PlayerConfig["isMuted"] = false;
				targetPlayer->m_PlayerConfig["muteUntil"] = 0;
				targetPlayer->SaveConfig();

				HTTP::ModeratorUnmuteCommandLog(moderatorPlayer, targetPlayer);

				Chat::SendServerMessage("Unmuted player " + targetPlayer->GetSelector());
			}
			else {
				Chat::SendServerMessage("Player " + targetPlayer->GetSelector() + " is not muted!");
			}
		}
	}
}

void ChatCommands::Kick(Command* command, Player* moderatorPlayer) {
	if (command->HasArg(0)) {
		auto selector = command->GetArgString(0);
		auto players = PlayerUtils::FindPlayers(selector);

		for (size_t i = 0; i < players.size(); i++)
		{
			auto targetPlayer = players[i];

			if (targetPlayer->HasPermission("admin")) {
				Chat::SendServerMessage("Can't kick this player");
				continue;
			}

			if (targetPlayer->m_ClientId == moderatorPlayer->m_ClientId) {
				Chat::SendServerMessage("Maybe if you try kicking yourself again it'll work.");
				continue;
			}

			Mod::KickPlayer(targetPlayer->m_ClientId);

			HTTP::ModeratorKickCommandLog(moderatorPlayer, targetPlayer);
		}
	}
}

void ChatCommands::Mute(Command* command, Player* moderatorPlayer) {
	if (command->HasArg(0) && command->HasArg(1)) {
		auto selector = command->GetArgString(0);
		auto time = command->GetArgString(1);
		uint64_t muteTime = PlayerUtils::SecondsFromString(time);
		auto players = PlayerUtils::FindPlayers(selector);

		for (size_t i = 0; i < players.size(); i++)
		{
			auto targetPlayer = players[i];

			if (targetPlayer->HasPermission("admin")) {
				Chat::SendServerMessage("Can't mute this player");
				continue;
			}

			targetPlayer->m_PlayerConfig["isMuted"] = true;
			targetPlayer->m_PlayerConfig["muteUntil"] = std::time(nullptr) + muteTime;
			targetPlayer->SaveConfig();

			HTTP::ModeratorMuteCommandLog(moderatorPlayer, targetPlayer);

			Chat::SendServerMessage("Muted player " + targetPlayer->GetSelector() + " for " + PlayerUtils::FormatSeconds(muteTime));
		}
	}
}

void ChatCommands::BroadCast(Command* command, Message* message) {
	std::string text = command->GetArgText(0);

	message->m_SendType = MessageSendType::FORCE_PRIVATE;

	Message* msg = Chat::SendServerMessage(text);
	msg->m_SendType = MessageSendType::FORCE_SEND;
}

void ChatCommands::Time(Command* command) {
	if (!command->HasArg(0)) return;

	float time = command->GetArgFloat(0);

	Mod::SetCurrentGameModeTime(time);
}

void ChatCommands::Perm(Command* command, Player* moderatorPlayer) {
	if (command->HasArg(0) && command->HasArg(1) && command->HasArg(2)) {
		auto addOrSubtract = command->GetArgString(0);
		auto selector = command->GetArgString(1);
		auto perm = command->GetArgString(2);

		auto players = PlayerUtils::FindPlayers(selector);

		int count = 0;

		if (addOrSubtract.rfind("add", 0) == 0) {
			for (size_t i = 0; i < players.size(); i++)
			{
				auto targetPlayer = players[i];

				if (!targetPlayer->HasPermission(perm)) {
					targetPlayer->AddPermission(perm);
					targetPlayer->m_PlayerConfig["perms"].push_back(perm);
					targetPlayer->SaveConfig();
					count++;

					HTTP::ModeratorPermCommandLog(moderatorPlayer, targetPlayer, perm);

					Chat::SendServerMessage("Perm given to " + targetPlayer->GetSelector());
				}
			}
		}

		if (addOrSubtract.rfind("del", 0) == 0) {
			for (size_t i = 0; i < players.size(); i++)
			{
				auto targetPlayer = players[i];

				if (targetPlayer->HasPermission(perm)) {
					targetPlayer->RemovePermission(perm);
					int permIndex = 0;

					for (auto& playerPerm : targetPlayer->m_PlayerConfig["perms"].items()) {
						std::string playerPermString = playerPerm.value().get<std::string>();

						if (perm == playerPermString) {
							targetPlayer->m_PlayerConfig["perms"].erase(permIndex);
							HTTP::ModeratorPermCommandLog(moderatorPlayer, targetPlayer, perm, true);
						}

						permIndex += 1;
					}

					targetPlayer->SaveConfig();
					count++;
					Chat::SendServerMessage("Perm removed from " + targetPlayer->GetSelector());
				}
			}
		}
	}
}