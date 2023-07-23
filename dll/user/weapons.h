#include "pch-il2cpp.h"
#include <string>
#include <map>

class Weapon {
public:
	int id;
	std::string name;
	std::string nameShort;
	std::string tip;
	int ammo;
	int price;

	Weapon() : id(0), name(""), nameShort(""), tip(""), ammo(0), price(0) {}
	Weapon(int id, std::string name, std::string nameShort, std::string tip, int ammo, int price) : id(id), name(name), nameShort(nameShort), tip(tip), ammo(ammo), price(price) {}
};


class Weapons {
public:
	static int m_UniqueObjectId;
	static std::map<int, Weapon> list;
	static Weapon SelectRandomWeapon();
	static bool WeaponBuyable(int weaponId);
	static bool ValidWeaponId(int weaponId);
	static int IsWeaponCommand(std::string weaponName);
	static void ConfigureWeapons();
	static void DropWeapon(long long toClient, int weaponId);
	static void GiveWeapon(long long toClient, int weaponId);
	static void ShowWeapons();
};