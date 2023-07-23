import sqlite3
import json
from shutil import copyfile
from threading import Thread
from os.path import realpath, dirname
from time import sleep
from copy import deepcopy

class Leaderboard():
    def __init__(self):
        # Create a copy of database because
        # multiple scripts accessing the same db causes issues (at this speed anyways)
        # will fix later
        self.copy_database() 

        with open("ccgm_config.json") as f:
            self.ccgmConfig = json.load(f)

        self.servers = []
        self.emptyServerScores = {}
        self.emptyServerLists = {}
        self.emptyServerDicts = {}

        for server in self.ccgmConfig["servers"]:
            serverShortName = self.ccgmConfig["servers"][server]["shortName"]
            self.servers.append(serverShortName)
            self.emptyServerScores[serverShortName] = 0
            self.emptyServerLists[serverShortName] = []
            self.emptyServerDicts[serverShortName] = {}

        self.con = sqlite3.connect("ccgm_database_copy.db", check_same_thread=False)
        self.cur = self.con.cursor()
        self.query = None

        # Disabled my account's steam ids because an old version
        # had a win bug that would cause lobby owner to get a win
        # in the lobby
        self.disabledSteamIds = ["76561199151852899", "76561198872595651", "76561199286954011"]

    def copy_database(self):
        copyfile(f"ccgm_database.db", "ccgm_database_copy.db")

    def create_query(self):
        self.query = self.cur.execute("SELECT * FROM playerConfig")

    def get_next_user_config(self):
        if not self.query:
            self.create_query()

        result = self.query.fetchone()
            
        if result:
            if len(result) == 2:
                steamId, config = result[0], json.loads(result[1])

                defaultConfig = {
                    "steamId": steamId,
                    "points": 0,
                    "wins": self.emptyServerScores,
                    "kills": self.emptyServerScores,
                    "deaths": self.emptyServerScores,
                    "perms": [],
                    "isBanned": False
                }

                for k in defaultConfig:
                    if not k in config:
                        config[k] = defaultConfig[k]

                    if k in ["wins", "kills", "deaths"]:
                        for s in defaultConfig[k]:
                            if not s in config[k]:
                                config[k][s] = defaultConfig[k][s]
                    
                return steamId, config

        return False, False

    def write_top_wins(self):
        topSteamIds = deepcopy(self.emptyServerLists)
        topPlayerConfigs = deepcopy(self.emptyServerLists)

        for i in range(15):
            bestConfig = self.emptyServerDicts.copy()
            bestSteamId = self.emptyServerDicts.copy()
            mostWins = self.emptyServerScores.copy()

            while True:                
                _steamid, playerConfig = self.get_next_user_config()

                if not playerConfig:
                    self.create_query()
                    break

                for serverShortName in self.servers:
                    if not playerConfig["steamId"] in topSteamIds[serverShortName] and not playerConfig["steamId"] in self.disabledSteamIds:                
                        if not playerConfig["isBanned"]:
                            if serverShortName in playerConfig["wins"]:
                                if playerConfig["wins"][serverShortName] > mostWins[serverShortName]:
                                    mostWins[serverShortName] = playerConfig["wins"][serverShortName]
                                    bestSteamId[serverShortName] = playerConfig["steamId"]
                                    bestConfig[serverShortName] = playerConfig

            for serverShortName in self.servers:
                topSteamIds[serverShortName].append(bestSteamId[serverShortName])
                topPlayerConfigs[serverShortName].append(bestConfig[serverShortName])

        with open("wins.json", "w") as f:
            f.write(json.dumps(topPlayerConfigs, indent=1))

    def worker_thread(self):
        while True:
            try:
                self.copy_database()
                self.write_top_wins()
                sleep(10)
            except Exception as e:
                print(e)

    def start_worker_thread(self):
        t1 = Thread(target=self.worker_thread, daemon=True)
        t1.start()
    
    