import sandboxie
import requests
import psutil
from os import system
import subprocess
from os.path import dirname
from time import sleep
from threading import Thread, Lock
from time import time
import win32gui
import win32process
import ctypes
import json

IsWindowVisible = ctypes.windll.user32.IsWindowVisible

defaultServerStatus = {
    "lobbyCode": "Na",
    "players": "Na",
    "maxPlayers": "Na",
    "map": "Na",
    "mode": "Na",
    "status": "Offline"
}

class InjectReadyTimeout(Exception):
    pass

class InjectTimeout(Exception):
    pass

class CreateLobbyTimeout(Exception):
    pass

class WatchDog():
    def __init__(self):
        self.injectTimeout = 30
        self.DLLName = "CCGM_LIVE.dll"
        self.sandbox = sandboxie.Sandboxie(install_dir=r"C:\Program Files\Sandboxie-Plus")
        self.serverUrl = "http://127.0.0.1:5555"

        with open("ccgm_config.json", "r") as f:
            self.ccgmConfig = json.load(f)

        self.serverSandboxes = {}
        self.serverWatchList = {}
        self.serverRootNames = {}
        
        self.startServerLock = Lock()
        self.restartServerLock = Lock()

        for server in self.ccgmConfig["servers"]:
            serverShortName = self.ccgmConfig["servers"][server]["shortName"]
            serverSandboxName = self.ccgmConfig["servers"][server]["sandboxName"]
            self.serverSandboxes[serverShortName] = serverSandboxName
            self.serverWatchList[serverShortName] = False
            self.serverRootNames[serverShortName] = server

        self.check_for_game_processes()

    def check_for_game_processes(self):
        for serverShortName in self.serverSandboxes:
            sandboxName = self.serverSandboxes[serverShortName]

            gameProcess = self.find_game_process(sandboxName)
            
            if gameProcess:
                self.serverWatchList[serverShortName] = True
                Thread(target=self.watch_worker, args=[serverShortName,], daemon=True).start()

    def http_post(self, path, Json):
        return requests.post(f"{self.serverUrl}{path}", json=Json)

    def http_get(self, path):
        return requests.get(f"{self.serverUrl}{path}")

    def get_hwnds_for_pid(self, pid):
        def callback(hwnd, hwnds):
            _, found_pid = win32process.GetWindowThreadProcessId(hwnd)
            if found_pid == pid:
                hwnds.append(hwnd)

            return True

        hwnds = []
        win32gui.EnumWindows(callback, hwnds)
        return hwnds 

    def find_game_process(self, box):
        if not box:
            processIterator = psutil.process_iter()
        else:
            processIterator = self.sandbox.running_processes(box=box)

        for p in processIterator:
            if isinstance(p, int):
                try:
                    gameProcess = psutil.Process(p)
                except psutil.NoSuchProcess:
                    continue
            else:
                gameProcess = p

            if gameProcess.name() == "Crab Game.exe":
                return gameProcess
        else:
            return False

    def inject(self, box):
        gameProcess = self.find_game_process(box)

        if gameProcess:
            injectCommand = f'"{dirname(__file__)}/Injector.exe" --process-id {gameProcess.pid} --inject "{dirname(__file__)}/{self.DLLName}"'
            
            if not box:
                system(f'start "" {injectCommand}')
            else:
                self.sandbox.start(injectCommand, box=box, wait=False)
            
            return gameProcess

        else:
            print("GAME WASN'T FOUND! RESTARTING")
            return False

    def start_crab_game(self, box):
        # Using steam.exe rather than starting regularly is slow
        # However, multiplayer doesn't work without the game being started by Steam.
        startGameCommand = f'"C:\Program Files (x86)\Steam\steam.exe" -applaunch 1782210'

        if not box:
            subprocess.Popen(["C:\Program Files (x86)\Steam\steam.exe", "-applaunch", "1782210"], creationflags=subprocess.DETACHED_PROCESS)
        else:
            self.sandbox.start(startGameCommand, box=box, wait=False)

    def signal_offline(self, serverShortName):
        self.http_post("/server_status", {serverShortName: defaultServerStatus})

    def server_hung(self, serverShortName):
        serverStatuses = self.http_get("/server_status").json()

        serverStatus = serverStatuses[serverShortName]
        current_time = time()
        lastMessageOn = serverStatus["lastMessageOn"]

        if lastMessageOn == "Na":
            return False
        
        if current_time - lastMessageOn > 60 * 2:
            return True
        
        return False
    
    def server_0_players_bug(self, serverShortName):
        serverStatuses = requests.get(f"{self.serverUrl}/server_status").json()
        players = serverStatuses[serverShortName]["players"]

        return players == 0

    def signal_server_name(self, activeServerName):
        self.http_post("/active_server_name", {"activeServerName": activeServerName})

    def watch_worker(self, serverShortName):
        while True:
            sleep(5)

            watchServer = self.serverWatchList[serverShortName]
            sandboxName = self.serverSandboxes[serverShortName]
            gameProcess = self.find_game_process(sandboxName)

            if not watchServer:
                self.signal_offline(serverShortName)
                break

            if not gameProcess:
                print("GAME WASN'T FOUND! RESTARTING")
                serverRestarted = self.restart_server(serverShortName)

                if serverRestarted:
                    break

            if self.server_hung(serverShortName):
                print("PROCESS HANG DETECTED, RESTARTING")
                serverRestarted = self.restart_server(serverShortName)
                
                if serverRestarted:
                    break

            if self.server_0_players_bug(serverShortName):
                print(f"0 players bug detected for {serverShortName}, restarting!")
                serverRestarted = self.restart_server(serverShortName)
                
                if serverRestarted:
                    break

    def wait_until_status(self, serverShortName, statuses):
        now = time()

        while time() < now + self.injectTimeout:
            sleep(1)
            serverStatus = self.http_get("/server_status").json()
            
            if not isinstance(statuses, list):
                statuses = [statuses]
            
            for status in statuses:
                if serverStatus[serverShortName]["status"] == status:
                    return True
            
        return False

    def start_server(self, serverShortName):
        with self.startServerLock:
            self.http_post("/server_status", {serverShortName: {"status": "Starting"}})

            sandboxName = self.serverSandboxes[serverShortName]
            serverRootName = self.serverRootNames[serverShortName]

            self.signal_server_name(serverRootName)
            self.start_crab_game(sandboxName)
            
            injectReady = self.wait_until_status(serverShortName, "ReadyForInject")

            if not injectReady:
                print("Game start timeout or CCGM mapmod fork not installed")
                self.signal_offline(serverShortName)
                raise InjectReadyTimeout

            gameProcess = self.inject(sandboxName)

            injected = self.wait_until_status(serverShortName, statuses=["Injected", "Online"])

            if not injected:
                print("Injecting timeout")
                self.signal_offline(serverShortName)
                raise InjectTimeout
            
            lobbyCreated = self.wait_until_status(serverShortName, "Online")

            if not lobbyCreated:
                print("Lobby creation timeout. Maybe Steam needs reloaded?")
                self.signal_offline(serverShortName)
                raise CreateLobbyTimeout

            if gameProcess:
                self.serverWatchList[serverShortName] = True
                Thread(target=self.watch_worker, args=[serverShortName,], daemon=True).start()
                return True
            else:
                self.signal_offline(serverShortName)
                return False

    def kill_game_process(self, gameProcess):
        system(f"taskkill /f /pid {gameProcess.pid}")

    def restart_server(self, serverShortName):
        with self.restartServerLock:
            self.signal_offline(serverShortName)
            sandboxName = self.serverSandboxes[serverShortName]

            self.serverWatchList[serverShortName] = False
            
            gameProcess = self.find_game_process(sandboxName)

            if gameProcess:
                self.kill_game_process(gameProcess)

            return self.start_server(serverShortName)