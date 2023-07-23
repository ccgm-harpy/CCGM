import sqlite3
import json
from os.path import isfile

class database():
    def __init__(self):
        with open("ccgm_config.json", "r") as f:
            self.ccgmConfig = json.load(f)

        self.serverShortNames = []
        self.emptyServerValues = {}

        for serverName in self.ccgmConfig["servers"]:
            serverShortName = self.ccgmConfig["servers"][serverName]["shortName"]
            self.serverShortNames.append(serverShortName)
            self.emptyServerValues[serverShortName] = 0

        self.databaseName = "ccgm_database.db"

        if not isfile(self.databaseName):
            self.create_database()

    def create_database(self):
        con, cur = self.load_database()

        cur.execute('CREATE TABLE playerConfig (steamId text, playerConfig text)')
        con.commit()

    def load_database(self):
        con = sqlite3.connect(self.databaseName, check_same_thread=False)
        cur = con.cursor()
        return con, cur

    def save_player_config(self, steamId, playerConfig):
        con, cur = self.load_database()

        if isinstance(playerConfig, dict):
            playerConfig = json.dumps(playerConfig)

        userExists = cur.execute("SELECT 1 FROM playerConfig WHERE steamId = ?", (steamId,)).fetchone() 

        if userExists:
            cur.execute("UPDATE playerConfig SET playerConfig = ? WHERE steamId = ?", (playerConfig, steamId,))
        else:
            cur.execute("INSERT INTO playerConfig (steamId, playerConfig) VALUES (?, ?)", (steamId, playerConfig,))

        con.commit()

    def load_player_config_discord(self, steamId):
        _, cur = self.load_database()

        steamId = str(steamId)

        defaultConfig = {
            "steamId": steamId,
            "points": 0,
            "wins": self.emptyServerValues,
            "kills": self.emptyServerValues,
            "deaths": self.emptyServerValues,
            "perms": [],
            "isBanned": False,
            "isMuted": False,
            "muteUntil": 0,
            "banUntil": 0
        }

        playerConfig = cur.execute("SELECT playerConfig FROM playerConfig WHERE steamId=?", (steamId,)).fetchone()

        if playerConfig:
            playerConfig = json.loads(playerConfig[0])
            
            for k in defaultConfig:
                if not k in playerConfig:
                    playerConfig[k] = defaultConfig[k]

                if k in ["wins", "kills", "deaths"]:
                    for s in defaultConfig[k]:
                        if not s in playerConfig[k]:
                            playerConfig[k][s] = defaultConfig[k][s]
            
        else:
            return False

        return playerConfig

    def load_player_config(self, steamId):
        _, cur = self.load_database()

        steamId = str(steamId)

        defaultConfig = {
            "steamId": steamId,
            "points": 0,
            "wins": self.emptyServerValues,
            "kills": self.emptyServerValues,
            "deaths": self.emptyServerValues,
            "perms": [],
            "isBanned": False,
            "isMuted": False,
            "muteUntil": 0,
            "banUntil": 0
        }

        playerConfig = cur.execute("SELECT playerConfig FROM playerConfig WHERE steamId=?", (steamId,)).fetchone()

        if playerConfig:
            playerConfig = json.loads(playerConfig[0])
            
            for k in defaultConfig:
                if not k in playerConfig:
                    playerConfig[k] = defaultConfig[k]

                if k in ["wins", "kills", "deaths"]:
                    for s in defaultConfig[k]:
                        if not s in playerConfig[k]:
                            playerConfig[k][s] = defaultConfig[k][s]
            
        else:
            playerConfig = defaultConfig

            self.save_player_config(steamId, playerConfig)

        return playerConfig