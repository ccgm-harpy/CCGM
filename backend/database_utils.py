import sqlite3
import json
import pyperclip as clipboard
from os import system
from time import sleep

con = sqlite3.connect("ccgm_database.db")
cur = con.cursor()

with open("ccgm_config.json", "r") as f:
    ccgmConfig = json.load(f)

adminPerms = ccgmConfig["adminPerms"]
moderatorPerms = ccgmConfig["moderatorPerms"]
emptyServerValues = {}

for serverName in ccgmConfig["servers"]:
    serverShortName = ccgmConfig["servers"][serverName]["shortName"]
    emptyServerValues[serverShortName] = 0

def get_steamIds():
    steamIds = []

    for steamId in cur.execute("SELECT steamId FROM playerConfig").fetchall():
        steamIds.append(steamId[0])

    return steamIds

def get_playerConfigs():
    playerConfigs = []

    for playerConfig in cur.execute("SELECT playerConfig FROM playerConfig").fetchall():
        pConfig = json.loads(playerConfig[0])

        defaultConfig = {
            "steamId": pConfig["steamId"],
            "points": 0,
            "wins": emptyServerValues,
            "kills": emptyServerValues,
            "deaths": emptyServerValues,
            "perms": [],
            "isBanned": False
        }

        for k in defaultConfig:
            if not k in pConfig:
                pConfig[k] = defaultConfig[k]

            if k in ["wins", "kills", "deaths"]:
                for s in defaultConfig[k]:
                    if not s in pConfig[k]:
                        pConfig[k][s] = defaultConfig[k][s]

        playerConfigs.append(json.dumps(pConfig))

    return playerConfigs
    
def load_player_config(steamId):
    playerConfig = cur.execute("SELECT playerConfig FROM playerConfig WHERE steamId=?", (steamId,)).fetchone()

    if playerConfig:
        playerConfig = json.loads(playerConfig[0])

        defaultConfig = {
            "steamId": playerConfig["steamId"],
            "points": 0,
            "wins": emptyServerValues,
            "kills": emptyServerValues,
            "deaths": emptyServerValues,
            "perms": [],
            "isBanned": False
        }
        
        for k in defaultConfig:
            if not k in playerConfig:
                playerConfig[k] = defaultConfig[k]

            if k in ["wins", "kills", "deaths"]:
                for s in defaultConfig[k]:
                    if not s in playerConfig[k]:
                        playerConfig[k][s] = defaultConfig[k][s]

        return playerConfig
    else:
        playerConfig = defaultConfig

        cur.execute("INSERT INTO playerConfig (steamId, playerConfig) VALUES (?, ?)", (steamId, json.dumps(playerConfig)))
        return playerConfig

def update_player_config(steamId, playerConfig):
    cur.execute("UPDATE playerConfig SET playerConfig = ? WHERE steamId = ?", (json.dumps(playerConfig), steamId,))
    con.commit()

def load_moderators():
    moderators = []
    query = cur.execute("SELECT playerConfig FROM playerConfig")

    while True:
        playerConfig = query.fetchone()

        if playerConfig:
            playerConfig = json.loads(playerConfig[0])
            steamId = playerConfig["steamId"]
            perms = playerConfig["perms"]

            if perms:
                moderators.append({"steam": f"https://steamcommunity.com/profiles/{steamId}", "perms": perms})
        
        else:
            break

    return moderators

def program_loop():
    while True:
        print("""Make a choice:
        0. Load player config
        1. Give player moderator perms
        2. Remove player's perms
        3. Ban player
        4. Unban player
        5. Give player admin perms
        6. Save player config (Advanced users only)
        7. Load moderators
        """.replace("        ", ""))

        choice = int(input("-> "))

        if choice == 7:
            moderators = {"moderators": load_moderators()}
            clipboard.copy(json.dumps(moderators, indent=1))
            print("Moderators copied to clipboard")
            sleep(3)
            system("cls")
            continue

        steamId = str(input("SteamID-> "))

        if str(input("Confirm?(y/n)-> ")).lower() != "y":
            system("cls")
            continue

        playerConfig = load_player_config(steamId)

        if choice == 0:
            playerConfigString = json.dumps(playerConfig)
            print(f"{playerConfigString}\n\nConfig copied to clipboard")
            clipboard.copy(playerConfigString)
            sleep(3)
            system("cls")

        elif choice == 1:
            for perm in moderatorPerms:
                if not perm in playerConfig["perms"]:
                    playerConfig["perms"].append(perm.replace("!", ""))

            print("Player given moderator perms...")
            sleep(3)
            system("cls")

        elif choice == 2:
            playerConfig["perms"] = []

            print("Player's perms removed...")
            sleep(3)
            system("cls")

        elif choice == 3:
            banReason = str(input("Ban reason-> "))

            playerConfig["banReason"] = banReason
            playerConfig["isBanned"] = True
            playerConfig["perms"] = []

            print("Player has been banned...")
            sleep(3)
            system("cls")
        
        elif choice == 4:
            playerConfig["isBanned"] = False

            if "banReason" in playerConfig:
                del playerConfig["banReason"]

            print("Player has been unbanned...")
            sleep(3)
            system("cls")
        
        elif choice == 5:
            for perm in adminPerms:
                if not perm in playerConfig["perms"]:
                    playerConfig["perms"].append(perm.replace("!", ""))

            print("Player given admin perms...")
            sleep(3)
            system("cls")

        elif choice == 6:
            output = clipboard.paste()
            print(f"{output}\n\nWriting changes...")
            playerConfig = json.loads(output)
            sleep(3)
            update_player_config(steamId, playerConfig)
            print("Player config updated")
            sleep(3)
            system("cls")

        else:
            print("Um... what?")
            sleep(3)
            system("cls")

        update_player_config(steamId, playerConfig)

if __name__ == "__main__":
    program_loop()
