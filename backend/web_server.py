from database import database
from quart import Quart, request, abort
from bd_utilities import pick_possible_animation
from copy import deepcopy
from time import time
from os.path import exists, isfile
from threading import Lock
from os import mkdir
import os
#from pythonLib.authentication import Authenticator, InvalidNonceError
import json

# Config generator does not support numbers big enough for channel / user ids
# Update them to strings if user is from an older patch
# Rest of code is expecting integers so we'll convert them back here
def patch_ccgm_config(ccgmConfig):
    patchKeys = [
        "adminRoleId",
        "banLogsId",
        "moderatorRoleId",
        "ownerRoleId",
        "ownerId",
        "remoteCommandsId",
        "userReportsId",
        "ownerIds"
    ]

    for patchKey in patchKeys:
        if patchKey == "ownerIds":
            tempOwnerIds = []
            
            for ownerId in ccgmConfig[patchKey]:
                tempOwnerIds.append(int(ownerId))

            ccgmConfig[patchKey] = tempOwnerIds

        else:
            ccgmConfig[patchKey] = int(ccgmConfig[patchKey])

    return ccgmConfig

os.chdir(os.path.dirname(os.path.abspath(__file__)))

app = Quart(__name__)

playerReports = {}
serverCommands = {}
serverCommandsChaos = {}
serverCommandsDefault = {}
serverCommandsRace = {}
serverCommandsTag = {}
serverCommandsCustom = {}
serverBans = {}

with open("ccgm_config.json", "r") as f:
    ccgmConfig = patch_ccgm_config(json.load(f))

#authenticator = Authenticator(ccgmConfig)

remoteBans = {} # Rename to "remoteBansQueue"

defaultServerStatus = {
    "lobbyCode": "Na",
    "players": "Na",
    "maxPlayers": "Na",
    "map": "Na",
    "mode": "Na",
    "lastMessageOn": "Na",
    "status": "Offline"
}

serverStatus = {}

for server in ccgmConfig["servers"]:
    serverShortName = ccgmConfig["servers"][server]["shortName"]
    remoteBans[serverShortName] = {"bans": []}
    serverStatus[serverShortName] = deepcopy(defaultServerStatus)
    serverCommands[serverShortName] = {}

if not exists("chatLogs"):
    mkdir("chatLogs")

    for serverShortName in serverStatus:
        with open(f"chatLogs\\{serverShortName}.txt", "w") as f:
            continue

activeServerName = ""

chat_log_lock = Lock()

def save_chat_log(serverShortName, line):
    with chat_log_lock:
        with open(f"chatLogs\\{serverShortName}.txt", "a") as f:
            f.write(f"{line}\n")

def get_request(json_data):
    return json.loads(json_data["data"])

def decrypt_post(encryptedPostData):
    """ Not yet implemented into ccgm dll
    try:
        postData = authenticator.decrypt(encryptedPostData, authenticator.webServerKey)
    except Exception as e:
        print(e)
        return False
    
    return postData"""

    return encryptedPostData

@app.route("/chatLog", methods=["GET", "POST"])
async def chat_log():
    if request.method == "POST":
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        serverShortName = False

        for ssName in serverStatus:
            if ssName in postJson:
                serverShortName = ssName
                break

        if serverShortName:
            save_chat_log(serverShortName, postJson[serverShortName])
            return "OK", 200

    return "NOPE", 500


@app.route("/perm_set", methods=["POST"])
async def promote():
    if request.method == "POST":
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()
        steamId = postJson["steamId"]
        playerConfig = db.load_player_config(steamId)
        playerConfig["perms"] = []
        permission = postJson["perm"]

        if not permission == "userPerms":
            for perm in ccgmConfig[permission]:
                if not perm in playerConfig["perms"]:
                    playerConfig["perms"].append(perm.replace("!", ""))

        db.save_player_config(steamId, playerConfig)

        return "OK", 200

    return "NOPE", 500

@app.route("/is_mapmod_server", methods=["GET"])
async def is_mapmod_server():
    if request.method == "GET":
        return str(ccgmConfig["servers"][activeServerName]["gameModeSettings"]["customMaps"])
    return "NOPE", 500

@app.route("/ccgm_config", methods=["POST", "GET"])
async def ccgm_config():
    """data = decrypt_post(await request.get_data())

    if not data:
        return "AUTH FAILURE", 500"""

    return json.dumps(ccgmConfig)

@app.route("/remote_bans", methods=["POST"])
async def remote_bans():
    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        if "steamId" in postJson:
            for serverShortName in remoteBans:
                remoteBans[serverShortName]["bans"].append(postJson)

            return "OK", 200
    
        elif "shortName" in postJson:
            serverShortName = postJson["shortName"]
            remoteBanList = remoteBans[serverShortName]["bans"]
            remoteBans[serverShortName]["bans"] = []
            return {"bans": remoteBanList}

    return "NOPE", 500

@app.route("/discord_invite", methods=["GET", "POST"])
async def set_get_discord_invite():
    if request.method == "POST":
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        if "discordInvite" in postJson:
            discordInvite = postJson["discordInvite"]
            
            with open("discord_invite.txt", "w") as f:
                f.write(discordInvite)
                return "OK", 200
    
    elif request.method == "GET":
        with open("discord_invite.txt", "r") as f:
            return f.read(), 200

    return "NOPE", 500

@app.route("/server_status", methods=["GET", "POST"])
async def server_status():
    if request.method == "POST":
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        for serverShortName in serverStatus:
            if serverShortName in postJson:
                serverStatus[serverShortName]["lastMessageOn"] = time()

                if "lobbyCode" in postJson[serverShortName]:
                    serverStatus[serverShortName]["lobbyCode"] = postJson[serverShortName]["lobbyCode"]

                if "maxPlayers" in postJson[serverShortName]:
                    serverStatus[serverShortName]["maxPlayers"] = postJson[serverShortName]["maxPlayers"]

                if "players" in postJson[serverShortName]:
                    serverStatus[serverShortName]["players"] = postJson[serverShortName]["players"]
                    
                if "map" in postJson[serverShortName]:
                    serverStatus[serverShortName]["map"] = postJson[serverShortName]["map"]

                if "mode" in postJson[serverShortName]:
                    serverStatus[serverShortName]["mode"] = postJson[serverShortName]["mode"]

                if "status" in postJson[serverShortName]:
                    serverStatus[serverShortName]["status"] = postJson[serverShortName]["status"]
        
        return "OK", 200

    elif request.method == "GET":
        return serverStatus
    
    return "NOPE", 500

@app.route("/load_block_drop_animation", methods=["POST"])
async def load_block_drop_animation():
    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        animationFile = pick_possible_animation(postJson["players"])
        
        if animationFile:
            with open(animationFile, "r") as f:
                return json.loads(f.read())
        else:
            return "NA", 500
    
    return "NOPE", 500

@app.route("/map_string", methods=["GET"])
async def load_map_string():
    if request.method == "GET":
        return ccgmConfig["servers"][activeServerName]["maps"]

@app.route("/mode_string", methods=["GET"])
async def load_mode_string():
    if request.method == "GET":
        return ccgmConfig["servers"][activeServerName]["modes"]

@app.route("/active_server_name", methods=["GET", "POST"])
async def load_active_server_name():
    global activeServerName

    if request.method == "GET":
        return activeServerName
    elif request.method == "POST":
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()
        activeServerName = postJson["activeServerName"]
        return "OK", 200
    return "NOPE", 500

@app.route("/server_short_name", methods=["GET"])
async def load_server_short_name():
    if request.method == "GET":
        return ccgmConfig["servers"][activeServerName]["shortName"]

    return "NOPE", 500

@app.route("/do_commands_<serverName>", methods=["POST", "GET"])
async def do_commands(serverName):
    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        if "cmd" in postJson:
            if "cmds" in serverCommands[serverName]:
                serverCommands[serverName]["cmds"].append(postJson)
            else:
                serverCommands[serverName]["cmds"] = []
                serverCommands[serverName]["cmds"].append(postJson)

            return "OK", 200
    
    elif request.method == "GET":
        serverCommandsString = json.dumps(serverCommands[serverName])

        if serverCommands[serverName]:
            serverCommands[serverName] = {}

        return serverCommandsString

    return "NOPE", 500

@app.route("/bans", methods=["POST", "GET"])
async def bans():
    global serverBans

    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        if "moderator" in postJson:
            if "bans" in serverBans:
                serverBans["bans"].append(postJson)
            else:
                serverBans["bans"] = []
                serverBans["bans"].append(postJson)

            return "OK", 200
    
    elif request.method == "GET":
        serverBansString = json.dumps(serverBans)

        if serverBans:
            serverBans = {}

        return serverBansString

    return "NOPE", 500

@app.route("/reports", methods=["POST", "GET"])
async def reports():
    global playerReports

    if request.method == 'POST':
        """if not decrypt_post(await request.get_data()):
            return "AUTH FAILURE", 500"""

    if request.method == "GET":
        reportJson = json.dumps(playerReports)
        playerReports = {}
        return reportJson
        
    return "NOPE", 500

@app.route("/report_player", methods=["POST"])
async def report_player():
    global playerReports

    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()

        if "report" in postJson:
            if "reports" in playerReports:
                playerReports["reports"].append(postJson)
            else:
                playerReports["reports"] = [
                    postJson
                ]

            return "OK", 200
    
    return "NOPE", 500

@app.route("/unban", methods=["POST"])
async def unban():
    if request.method == "POST":
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()
        steamId = postJson["steamId"]

        playerConfig = db.load_player_config_discord(steamId)

        if playerConfig:
            if playerConfig["isBanned"]:
                playerConfig["isBanned"] = False
                db.save_player_config(steamId, playerConfig)
                return "OK", 200
            else:
                return "NOT BANNED", 201

    return "NOPE", 500

@app.route("/player_info", methods=["POST"])
async def player_info():
    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()
        steamId = postJson["steamId"]
        playerConfig = db.load_player_config_discord(steamId)

        if playerConfig:
            response = app.response_class(response=json.dumps(playerConfig),
                status=200,
                mimetype='application/json'
            )

            return response
            
    return "NOPE", 500

@app.route("/load_player_config", methods=["POST"])
async def load_player_config():
    if request.method == 'POST':
        """jsonData = decrypt_post(await request.get_data())

        if not jsonData:
            return "AUTH FAILURE", 500"""
        
        postJson = await request.get_json()
        steamId = postJson["steamId"]
        playerConfig = db.load_player_config(steamId)

        return json.dumps(playerConfig), 200

    return "NOPE", 500

@app.route('/save_player_config', methods = ['POST'])
async def save_player_config():
    if request.method == 'POST':
        playerConfig = await request.get_json()
        steamId = playerConfig["steamId"]

        db.save_player_config(steamId, playerConfig)

        return "OK", 200
    return "NOPE", 500

if __name__ == '__main__':
    db = database()
    print("DATABASE CONNECTED")

    app.run(host="localhost", port=5555)
