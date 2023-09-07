import discord
import json
from discord.ext import tasks, commands
from os.path import isfile
from discord import Webhook, Intents
from leaderboards import Leaderboard
from steamUser import SteamUser
import aiohttp
import requests
import os

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

intents = Intents.default()
intents.message_content = True

client = commands.Bot(command_prefix='*', intents=intents, help_command=None)

with open("ccgm_config.json", "r") as f:
    ccgmConfig = patch_ccgm_config(json.load(f))

STEAMAPIKEY = ccgmConfig["steamApiKey"]
TOKEN = ccgmConfig["discordInfoBotToken"]

steamUser = SteamUser(STEAMAPIKEY)

inviteCodesWebHook = ccgmConfig["inviteCodesWebHook"]
leaderboardWebHooks = {}
inviteColors = {}
shortToLongServerNames = {}

defaultServerStatus = {
    "lobbyCode": "Na",
    "players": 0,
    "maxPlayers": 0,
    "map": 0,
    "mode": 0,
    "lastMessageOn": "Na",
    "status": "Offline"
}

serverStatus = {}

for serverName in ccgmConfig["servers"]:
    serverShortName = ccgmConfig["servers"][serverName]["shortName"]
    webHook = ccgmConfig["servers"][serverName]["leaderboardWebHook"]
    inviteColor = ccgmConfig["servers"][serverName]["inviteColor"]
    leaderboardWebHooks[serverShortName] = webHook
    serverStatus[serverShortName] = defaultServerStatus
    inviteColors[serverShortName] = int(inviteColor.replace("#", "0x"), 16)
    shortToLongServerNames[serverShortName] = serverName

serverUrl = "http://127.0.0.1:5555"

leaderboard = Leaderboard()
leaderboard.start_worker_thread()

def get_top_wins():
    with open("wins.json", "r") as f:
        return json.load(f)

async def send_leader_board_wins():
    print("running leaderboards task")
    try:
        async with aiohttp.ClientSession() as session:
            def write_leaderboards_file(leaderboardMessages):
                with open("leaderboards.json", "w") as f:
                    f.write(json.dumps(leaderboardMessages))

            async def create_leaderboards_file():
                leaderboardMessages = {}

                for serverShortName in leaderboardSessions:
                    leaderboardMessages[serverShortName] = []

                for i in range(15):
                    for serverShortName in leaderboardWebHooks:
                        leaderboardWebHook = leaderboardSessions[serverShortName]
                        message = await leaderboardWebHook.send(embed=discord.Embed(description="Place Holder"), wait=True)
                        leaderboardMessages[serverShortName].append(message.id)

                write_leaderboards_file(leaderboardMessages)
                return leaderboardMessages

            async def read_leaderboards_file():
                updateLeaderboardsFile = False

                with open("leaderboards.json", "r") as f:
                    leaderboardMessages = json.load(f)

                for serverShortName in leaderboardWebHooks:
                    if not serverShortName in leaderboardMessages:
                        updateLeaderboardsFile = True
                        leaderboardWebHook = leaderboardSessions[serverShortName]
                        leaderboardMessages[serverShortName] = []

                        for i in range(15):
                            message = await leaderboardWebHook.send(embed=discord.Embed(description="Place Holder"), wait=True)
                            leaderboardMessages[serverShortName].append(message.id)
                
                if updateLeaderboardsFile:
                    write_leaderboards_file(leaderboardMessages)

                return leaderboardMessages

            leaderboardSessions = {}

            for serverShortName in leaderboardWebHooks:
                leaderboardWebHookString = leaderboardWebHooks[serverShortName]
                leaderboardSessions[serverShortName] = Webhook.from_url(leaderboardWebHookString, session=session)

            if not isfile("leaderboards.json"):
                leaderboardMessages = await create_leaderboards_file()
            else:
                leaderboardMessages = await read_leaderboards_file()

            try:
                topPlayerConfigs = get_top_wins()
            except (json.decoder.JSONDecodeError, FileNotFoundError):
                return 

            for serverShortName in leaderboardSessions:
                leaderboardWebHook = leaderboardSessions[serverShortName]

                for place, playerConfig in enumerate(topPlayerConfigs[serverShortName]):
                    place += 1

                    if not "steamId" in playerConfig:
                        break

                    steamId = playerConfig["steamId"]
                    steamUserName, steamAvatarUrl = steamUser.get_profile_info(steamId)

                    wins = playerConfig["wins"][serverShortName]

                    if place == 1:
                        embed = discord.Embed(
                            color = discord.Color(0xFFC107)
                        )
                        embed.set_thumbnail(url="https://cdn-icons-png.flaticon.com/512/889/889518.png")
                    elif place == 2:
                        embed = discord.Embed(
                            color = discord.Color(0xAAAAAA)
                        )
                        embed.set_thumbnail(url="https://i.ibb.co/ryQ0sWn/silver.png")
                    elif place == 3:
                        embed = discord.Embed(
                            color = discord.Color(0xB5663E)
                        )
                        embed.set_thumbnail(url="https://i.ibb.co/94gWyrq/bronze.png")
                    elif place == 15:
                        embed = discord.Embed(
                            color = discord.Color(0xFF00FF)
                        )
                        embed.set_thumbnail(url="https://i.ibb.co/1bhcJjV/you-tried.png")
                    else:
                        embed = discord.Embed(
                            color = discord.Color(0x000000)
                        )
                        embed.set_thumbnail(url="https://i.ibb.co/Kx5VJyb/top-15.png")

                    embed.set_author(name=steamUserName, icon_url=steamAvatarUrl)
                    embed.add_field(name=f"#{place} place ({wins} wins)", value=f"https://steamcommunity.com/profiles/{steamId}", inline=False)
                
                    await leaderboardWebHook.edit_message(leaderboardMessages[serverShortName][place-1], embed=embed)

    except Exception as e:
        print("Exception occured while trying to update leaderboards")
        print(e)
    else:
        print("Leaderboard updated without errors.")

@tasks.loop(minutes=10, reconnect=True)
async def send_logs():
    if not os.path.isdir("chatLogs"):
        return

    logs = [f"chatLogs\\{log}" for log in os.listdir("chatLogs") if log.endswith(".txt")]

    if not logs:
        return

    logsToSend = []

    for log in logs:
        with open(log, "r") as f:
            logLines = f.read().split("\n")
        
        if len(logLines) >= ccgmConfig["sendLogAfterLines"]:
            logsToSend.append(log)
            
    if not logsToSend:
        return

    channelId = ccgmConfig["logChannel"]

    if channelId != 0:
        channel = client.get_channel(channelId)
    else:
        channel = await client.fetch_user(ccgmConfig["ownerId"])

    embed = discord.Embed(
        title = "Server Logs",
        color = discord.Color(0x000000),
        description = f"Here are your server logs that have reached {ccgmConfig['sendLogAfterLines']} lines"
    )

    filesToSend = []

    for log in logsToSend:
        filesToSend.append(discord.File(log))

    await channel.send(embed=embed, files=filesToSend)

    for log in logsToSend:
        with open(log, "w") as f:
            continue
            

@tasks.loop(seconds=5, reconnect=True)
async def check_invites():
    try:
        async with aiohttp.ClientSession() as session:
            inviteCodeWebHook = Webhook.from_url(inviteCodesWebHook, session=session)

            def write_invite_codes_file(inviteCodeMessages):
                with open("invite_codes.json", "w") as f:
                    f.write(json.dumps(inviteCodeMessages))

            async def create_invite_codes_file():
                inviteCodeMessages = {}

                for serverShortName in leaderboardWebHooks:
                    message = await inviteCodeWebHook.send(embed=discord.Embed(description="Place Holder"), wait=True)
                    inviteCodeMessages[serverShortName] = message.id

                write_invite_codes_file(inviteCodeMessages)

            async def read_invite_codes_file():
                updateInviteCodesFile = False

                with open("invite_codes.json", "r") as f:
                    inviteCodeMessages = json.load(f)

                for serverShortName in inviteColors:
                    if not serverShortName in inviteCodeMessages:
                        updateInviteCodesFile = True
                        message = await inviteCodeWebHook.send(embed=discord.Embed(description="Place Holder"), wait=True)
                        inviteCodeMessages[serverShortName] = message.id

                if updateInviteCodesFile:
                    write_invite_codes_file(inviteCodeMessages)

                return inviteCodeMessages

            if not isfile("invite_codes.json"):
                inviteCodeMessages = await create_invite_codes_file()
            else:
                inviteCodeMessages = await read_invite_codes_file()
                
            with open("maps_modes.json", "r") as f:
                mapsModesList = json.load(f)

            invitesResponse = requests.get(f"{serverUrl}/server_status").json()

            for serverShortName in inviteCodeMessages:
                invitesResponse[serverShortName]["lastMessageOn"] = serverStatus[serverShortName]["lastMessageOn"]

                if not invitesResponse[serverShortName] == serverStatus[serverShortName]:
                    serverStatus[serverShortName] = invitesResponse[serverShortName]
                    inviteMessageId = inviteCodeMessages[serverShortName]
                    inviteCode = invitesResponse[serverShortName]["lobbyCode"]
                    maxPlayers = invitesResponse[serverShortName]["maxPlayers"]
                    players = invitesResponse[serverShortName]["players"]
                    gameServerMap = str(invitesResponse[serverShortName]["map"])
                    gameServerMode = str(invitesResponse[serverShortName]["mode"])
                    status = str(invitesResponse[serverShortName]["status"])

                    for mapName in mapsModesList["maps"]:
                        if str(mapsModesList["maps"][mapName]) == gameServerMap:
                            gameServerMap = mapName
                            break

                    for modeName in mapsModesList["modes"]:
                        if str(mapsModesList["modes"][modeName]) == gameServerMode:
                            gameServerMode = modeName
                            break

                    gameServerMap = " ".join([word.capitalize() for word in gameServerMap.split(" ")])
                    gameServerMode = " ".join([word.capitalize() for word in gameServerMode.split(" ")])

                    embed = discord.Embed(
                        title = f"{shortToLongServerNames[serverShortName]}",
                        color = discord.Color(inviteColors[serverShortName])
                    )

                    embed.add_field(name = "**Invite Code:**", value=inviteCode, inline=True)
                    embed.add_field(name = "**Players:**", value=f"{players}/{maxPlayers}", inline=True)
                    embed.add_field(name="\u200B", value="\u200B")
                    embed.add_field(name = "**Map:**", value=gameServerMap, inline=True)
                    embed.add_field(name = "**Mode:**", value=gameServerMode, inline=True)
                    embed.add_field(name="\u200B", value="\u200B")
                    embed.add_field(name = "**Status:**", value=status, inline=True)

                    await inviteCodeWebHook.edit_message(message_id=inviteMessageId, embed=embed)
    except Exception as e:
        print(e)

@tasks.loop(minutes=5, reconnect=True)
async def update_leaderboards():
    await send_leader_board_wins()
    return True

@client.event
async def on_ready():
    print("CONNECTED TO DISCORD")

    update_leaderboards.start()
    check_invites.start()
    send_logs.start()

if __name__ == '__main__':
    client.run(TOKEN)