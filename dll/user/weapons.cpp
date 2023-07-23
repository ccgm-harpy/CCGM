#include "pch-il2cpp.h"
#include <algorithm>
#include <string>
#include <map>
#include "weapons.h"
#include <vector>
#include <random>
#include <stdexcept>
#include "Server.h"
#include "mod.h"
#include "Chat.h"

std::default_random_engine rng;

int Weapons::m_UniqueObjectId = 100;                    // Keeps tracks of object ids for spawning weapons

std::vector<int> buyableIds = {};
std::vector<int> dropableIds = {};

std::map<int, Weapon> Weapons::list = {
	{0, Weapon(0, "ak-47", "ak", "", 30, 0)},
	{1, Weapon(1, "glock", "glock", "", 20, 6)},
	{2, Weapon(2, "revolver", "revolver", "Use to jump higher or push players", 25, 1)},
	{3, Weapon(3, "dual shotgun", "dual", "Use to fly! LIMITED AMMO", 6, 10)},
	{4, Weapon(4, "bat", "bat", "", 1, 2)},
	{5, Weapon(5, "bomb", "bomb", "", 1, 0)},
	{6, Weapon(6, "katana", "katana", "", 1, 3)},
	{7, Weapon(7, "knife", "knife", "", 1, 1)},
	{8, Weapon(8, "pipe", "pipe", "", 1, 3)},
	{9, Weapon(9, "snowball", "snowball", "", 1, 0)},
	{10, Weapon(10, "stick", "stick", "", 1, 0)},
	{11, Weapon(11, "milk", "milk", "", 1, 0)},
	{12, Weapon(12, "pizza", "pizza", "", 1, 0)},
	{13, Weapon(13, "grenade", "grenade", "", 1, 0)}
};

void Weapons::ConfigureWeapons() {
	auto weaponsConfig = Server::ccgmConfig["gameModeSettings"]["chaos"]["weapons"];

	for (auto& pair : Weapons::list) {
		Weapon* weapon = &pair.second;
		auto weaponConfig = weaponsConfig[weapon->nameShort];

		weapon->ammo = weaponConfig["ammo"];
		weapon->price = weaponConfig["price"];

		if (weaponConfig["buyable"]) {
			buyableIds.push_back(weapon->id);
		}

		if (weaponConfig["dropable"]) {
			dropableIds.push_back(weapon->id);
		}
	}
}

void Weapons::ShowWeapons() {
	auto weaponsConfig = Server::ccgmConfig["gameModeSettings"]["chaos"]["weapons"];

	int c = 0;
	std::string out = "";

	for (auto& pair : Weapons::list) {
		Weapon weapon = pair.second;
		auto weaponConfig = weaponsConfig[weapon.nameShort];

		if (weaponConfig["buyable"]) {
			out += "!" + weapon.nameShort + " " + std::to_string(weapon.price) + "p ";

			if (c == 2) {
				Chat::SendServerMessage(out);
				out = "";
				c = 0;
			}
			else {
				c += 1;
			}
		}
	}

	if (out.size()) {
		Chat::SendServerMessage(out);
	}
}

int Weapons::IsWeaponCommand(std::string weaponName) {
	std::map<int, Weapon>::iterator wli;

	for (wli = Weapons::list.begin(); wli != Weapons::list.end(); wli++) {
		std::string weaponNameShort = wli->second.nameShort;
		int weaponId = wli->first;

		if (weaponName == weaponNameShort) {
			return weaponId;
		}

		try {
			weaponId = std::stoi(weaponName);
			return weaponId;
		}
		catch (std::invalid_argument) {
			continue;
		}
	}
	return -1;
}

Weapon Weapons::SelectRandomWeapon() {
	std::uniform_int_distribution<int> dist(0, dropableIds.size() - 1);
	int index = dist(rng);
	return Weapons::list[dropableIds[index]];
}

bool Weapons::WeaponBuyable(int weaponId) {
	auto iterator = std::find(buyableIds.begin(), buyableIds.end(), weaponId);
	return iterator != buyableIds.end();
}

bool Weapons::ValidWeaponId(int weaponId) {
	if (Weapons::list.find(weaponId) != Weapons::list.end()) {
		return true;
	}
	return false;
}

void Weapons::DropWeapon(long long toClient, int weaponId) {
	Weapon weapon = Weapons::list[weaponId];
	Mod::SendDropItem(toClient, weaponId, Weapons::m_UniqueObjectId++, weapon.ammo);
}

void Weapons::GiveWeapon(long long toClient, int weaponId) {
	Mod::ForceGiveItem(toClient, weaponId, Weapons::m_UniqueObjectId);
}