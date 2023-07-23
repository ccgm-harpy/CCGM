#pragma once
#include "pch.h"
#include "Command.h"
#include "Player.h"
#include "Message.h"

class ChatCommands {
public:
	static void Ban(bool banTemporary, Command* command, Player* moderatorPlayer, std::string discordUserName="", std::string discordUserId="");
	static void Kick(Command* command, Player* moderatorPlayer);
	static void Mute(Command* command, Player* moderatorPlayer);
	static void BroadCast(Command* command, Message* message);
	static void Time(Command* command);;
	static void Perm(Command* command, Player* moderatorPlayer);
	static void Unmute(Command* command, Player* moderatorPlayer);
	static void Respawn(Command* command, Player* moderatorPlayer, bool isAdmin);
	static void TP(Command* command, Player* moderatorPlayer, bool isAdmin);
	static void PurchaseWeapon(Command* command, Player* player);
	static void Win(Command* command);
	static void Vanish(Player* player, bool isAdmin, Message* message);
	static void ModeratorGiveWeapon(Command* command, Player* player);
	static void ToggleChaos();
	static void MapModeForce(Command* command);
	static void MapMode(Command* command);
	static void Report(Command* command, Player* moderatorPlayer);
	static void Discord();
	static void Wins(Command* command, Player* moderatorPlayer);
	static void Start();
	static void Swap();
	static void PlayerInfo(Command* command, Player* player);
	static void Points(Player* player);
	static void Help();
	static void JumpPunch(Player* player);
	static void SuperPunch(Player* player);
	static void ForceField(Player* player);
	static void God(Player* moderatorPlayer);
	static void KD(Command* command, Player* player);
	static void Kill(Command* command, Player* moderatorPlayer);
	static void Light();
	static void Position(Command* command);
};
	