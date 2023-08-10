import discord
import requests
import json
import steamUser as SteamUser
import time
from discord import Intents
from discord.ext import tasks, commands
import watch_dog

intents = Intents.default()
intents.message_content = True

with open("ccgm_config.json", "r") as f:
    ccgmConfig = json.load(f)

prefix = ccgmConfig["discordCommandPrefix"]

client = commands.Bot(command_prefix=prefix, intents=intents, help_command=None)


with open("help.json", "w") as f:
    f.write(requests.get("https://pastebin.com/raw/EkAeqSsN").content.decode("utf-8"))

with open("help.json", "r") as f:
    helpJson = json.load(f)
    helpOptions = "".join(f"{prefix}help {option}\n" for option in helpJson)

ownerId = ccgmConfig["ownerId"]
ownerIds = ccgmConfig["ownerIds"]

userReportsId = ccgmConfig["userReportsId"]
banLogsId = ccgmConfig["banLogsId"]
remoteCommandsId = ccgmConfig["remoteCommandsId"]

ownerRoleId = ccgmConfig["ownerRoleId"]
adminRoleId = ccgmConfig["adminRoleId"]
moderatorRoleId = ccgmConfig["moderatorRoleId"]

serverUrl = "http://127.0.0.1:5555"

STEAMAPIKEY = ccgmConfig["steamApiKey"]
TOKEN = ccgmConfig["discordBotToken"]
moderatorMention = f"<@&{moderatorRoleId}>"

serverUrl = "http://127.0.0.1:5555"

STEAMAPIKEY = ccgmConfig["steamApiKey"]
TOKEN = ccgmConfig["discordBotToken"]

steamUser = SteamUser.SteamUser(STEAMAPIKEY)

serverShortNames = []

for server in ccgmConfig["servers"]:
    serverShortName = ccgmConfig["servers"][server]["shortName"]
    serverShortNames.append(serverShortName)

serverShortNamesString = "\n".join(shortName for shortName in serverShortNames)

watchDog = watch_dog.WatchDog()

permKeyList = ["userPerms", "moderatorPerms", "adminPerms", "owner"]

def format_seconds(total_seconds):
    days = total_seconds / (24 * 60 * 60)
    hours = (total_seconds / (60 * 60)) % 24
    minutes = (total_seconds / 60) % 60
    seconds = total_seconds % 60
    years = days / 365
    days %= 365

    result = ""

    if years > 0:
        result += f" {int(years)} year{'s' if int(years) != 1 else ''}"

    if days > 0:
        result += f" {int(days)} day{'s' if int(days) != 1 else ''}"

    if hours > 0:
        result += f" {int(hours)} hour{'s' if int(hours) != 1 else ''}"

    if minutes > 0:
        result += f" {int(minutes)} minute{'s' if int(minutes) != 1 else ''}"

    if seconds > 0 or not result:
        result += f" {int(seconds)} second{'s' if int(seconds) != 1 else ''}"

    return result.strip()


def seconds_from_string(time_str):
    if not time_str:
        return 0

    total_seconds = 0
    i = 0

    while i < len(time_str):
        num = 0

        while i < len(time_str) and time_str[i].isdigit():
            num = num * 10 + int(time_str[i])
            i += 1

        unit = time_str[i]
        i += 1

        if unit == 'y':
            total_seconds += num * 24 * 60 * 60 * 365
        elif unit == 'd':
            total_seconds += num * 24 * 60 * 60
        elif unit == 'h':
            total_seconds += num * 60 * 60
        elif unit == 'm':
            total_seconds += num * 60
        elif unit == 's':
            total_seconds += num
        else:
            total_seconds += num

    return total_seconds

def is_commands_channel(ctx):
    return ctx.channel.id == remoteCommandsId or ctx.author.id == ownerId

def get_permissions_level(user):
    # 0 = User
    # 1 = Moderator
    # 2 = Admin
    # 3 = Owner
    perm = 0

    if isinstance(user, discord.Member):
        for role in user.roles:
            if role.id == ownerRoleId or user.id == ownerId or user.id in ownerIds:
                if 3 > perm:
                    perm = 3
                
            elif role.id == adminRoleId:
                if 2 > perm:
                    perm = 2
                
            elif role.id == moderatorRoleId:
                if 1 > perm:
                    perm = 1

    elif isinstance(user, discord.User):
        if user.id == ownerId:
            perm = 3

    return perm

async def get_id64(ctx, steam):
    try:
        steamId64 = steamUser.get_id64(steam)
    except SteamUser.InvalidIdOrUrl:
        await send_error(ctx, "Invalid Steam ID or URL.")
        return False
    except SteamUser.InvalidJson:
        await send_error(ctx, "Couldn't understand Steam response. Is Steam online?")
        return False
    except SteamUser.UserNotFound:
        await send_error(ctx, "Steam user couldn't be found! Double check your ID / URL.")
        return False
    
    return steamId64

async def get_steam_profile_info(ctx, steamId64):
    try:
        steamName, steamAvatarUrl = steamUser.get_profile_info(steamId64)
    except SteamUser.InvalidJson:
        await send_error(ctx, "Couldn't understand Steam response. Is Steam online?")
        return False, False
    except SteamUser.UserNotFound:
        await send_error(ctx, "Steam user couldn't be found! Double check your ID / URL.")
        return False, False
    
    return steamName, steamAvatarUrl

@client.hybrid_command(name="steamid", description="Loads a Steam user's SteamID64 and permanent url")
async def steamid(ctx, steam):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx) and perm:
        steamId64 = await get_id64(ctx, steam)

        if not steamId64:
            return
        
        steamName, steamAvatarUrl = await get_steam_profile_info(ctx, steamId64)

        if not steamName or not steamAvatarUrl:
            return
        
        embed = discord.Embed(
            description = f"",
            color = discord.Color(0x1A1918)
        )

        embed.set_thumbnail(url=steamAvatarUrl)
        embed.add_field(name=f"**STEAM ID64:**", value=str(steamId64), inline=False)
        embed.add_field(name=f"**PERMENANT URL:**", value=f"https://steamcommunity.com/profiles/{steamId64}", inline=False)
        embed.set_thumbnail(url=steamAvatarUrl)
        embed.set_author(name=steamName, icon_url="https://i.imgur.com/Mhagff9.png")

        await ctx.send(embed = embed)

@client.hybrid_command(name="permset", description="Updates a player's permissions on your servers")
async def permset(ctx, steam, permission):
    user = ctx.author
    perm = get_permissions_level(user)
    permission = permission.lower()
    permissionStr = f"{permission}Perms"

    if is_commands_channel(ctx) and perm == 3:
        steamId64 = await get_id64(ctx, steam)

        if not steamId64:
            return

        steamName, steamAvatarUrl = await get_steam_profile_info(ctx, steamId64)

        if not steamName or not steamAvatarUrl:
            return

        if steamName and steamAvatarUrl:
            if permissionStr in permKeyList[:-1]:
                requests.post(f"{serverUrl}/perm_set", json={"steamId": steamId64, "perm": permissionStr})
                
                embed = discord.Embed(
                    description = f"Player given **{permission}** perms",
                    color = discord.Color(0x00FF00)
                )

                embed.set_thumbnail(url=steamAvatarUrl)
                embed.set_author(name="Permissions Update", icon_url="https://i.ibb.co/ZLnYNLD/permissions.png")
                embed.add_field(name=f"✅ User:", value=steamName, inline=True)
                embed.set_footer(text="Note: Player cannot be in your server(s) during this process!")

                await ctx.send(embed = embed)
            else:
                permListString = '\n'.join(permKey.replace("Perms", "") for permKey in permKeyList[:-1])
                await send_error(ctx, f"Invalid perm selected, you can choose:\n>>> {permListString}")

@client.hybrid_command(name="help", description="Shows command information. You can choose 'game' or 'discord'")
async def help(ctx, *, help_type):
    user = ctx.author
    perm = get_permissions_level(user)

    if perm and is_commands_channel(ctx):
        if help_type:
            if help_type in helpJson:
                embed = discord.Embed(
                    color = discord.Color(0xFFFFFF),
                    title = f"{help_type.capitalize()} Help"
                )

                for helpItem in helpJson[help_type]:
                    title = helpItem["title"].format(prefix=prefix)
                    description = helpItem["description"].format(prefix=prefix)
                    embed.add_field(name=title, value=description, inline=False)

                await ctx.send(embed = embed)
            else:
                await send_error(ctx, f"Invalid help option! You can choose:\n>>> {helpOptions}")
        else:
            await send_error(ctx, f"What do you need help with? You can choose:\n>>> {helpOptions}")

@client.hybrid_command(name="discordinvite", description="Updates the Discord invite shown on your servers")
async def discordinvite(ctx, discord_invite):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx) and perm == 3:
        requests.post(f"{serverUrl}/discord_invite", json={"discordInvite": discord_invite})

        embed = discord.Embed(
            description = f"Discord invite url updated to: {discord_invite}",
            color = discord.Color(0xFF0000)
        )

        await ctx.send(embed = embed)

@client.hybrid_command(name="ban", description="Bans a player from your servers")
async def ban(ctx, steam, *, ban_reason, banTimeSeconds="36500d"):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx) and perm:
        steamId64 = await get_id64(ctx, steam)

        if not steamId64:
            return

        steamName, steamAvatarUrl = await get_steam_profile_info(ctx, steamId64)

        if not steamName or not steamAvatarUrl:
            return

        if ban_reason:
            playerConfig = requests.post(f"{serverUrl}/player_info", json={"steamId": steamId64})

            if playerConfig.status_code == 200:
                banTimeSeconds = seconds_from_string(banTimeSeconds)
                banTimeString = format_seconds(banTimeSeconds)
                playerConfig = playerConfig.json()

                if not playerConfig["isBanned"]:
                    playerConfig["isBanned"] = True
                    playerConfig["banBy"] = user.id
                    playerConfig["banReason"] = ban_reason
                    playerConfig["banUntil"] = banTimeSeconds + time.time()
                    
                    requests.post(f"{serverUrl}/save_player_config", json=playerConfig)
                    requests.post(f"{serverUrl}/remote_bans", json={
                        "steamId": int(steamId64), "discordUserName": user.display_name,
                        "banReason": ban_reason
                        })

                    title = f"PLAYER MANUAL BAN FROM OUR SERVERS"
                    description = f"**Ban reason:**\n{ban_reason}"

                    embed = discord.Embed(
                        description = description,
                        color = discord.Color(0xFF0000)
                    )

                    embed.set_thumbnail(url=steamAvatarUrl)
                    embed.set_author(name=title, icon_url="https://i.ibb.co/0DTZc2g/hammer.png")
                    embed.add_field(name=f"✅ Moderator:", value=f"<@{user.id}>", inline=True)
                    embed.add_field(name=f"🚫 Accused:", value=f"**Name:** {steamName}\n\n**Profile URL:**\nhttps://steamcommunity.com/profiles/{steamId64}", inline=True)

                    channel = client.get_channel(banLogsId)

                    await channel.send(embed = embed)
                    
                    await send_success(ctx, "Player has been banned from your servers!")
                else:
                    await send_error(ctx, "User is already banned!")
            else:
                await send_error(ctx, "User was not found in your database.")
        else:
            await send_error(ctx, "Must specify a ban reason!")

@tasks.loop(seconds=1, reconnect=True)
async def check_reports():
    try:
        reportsResponse = requests.get(f"{serverUrl}/reports").json()

        if "reports" in reportsResponse:
            for report in reportsResponse["reports"]:
                report = report["report"]
                name = report["name"]
                number = report["number"]
                steamId = report["steamId"]
                accusedName =  report["accusedName"]
                accusedNumber = report["accusedNumber"]
                accusedSteamId = report["accusedSteamId"]
                reportReason = report["reportReason"]
                serverName = report["serverName"]

                title = f"PLAYER REPORT ON SERVER: {serverName.upper()}"
                description = f"**Report reason:**\n{reportReason}"

                embed = discord.Embed(
                    description = description,
                    color = discord.Color(0xFF0000)
                )

                embed.set_author(name=title, icon_url="https://www.iconsdb.com/icons/preview/red/warning-xxl.png")
                embed.add_field(name=f"✅ Report From: #{number}", value=f"**Name:** {name}\n\n**Profile URL:**\nhttps://steamcommunity.com/profiles/{steamId}", inline=True)
                embed.add_field(name=f"🚫 Accused: #{accusedNumber}", value=f"**Name:** {accusedName}\n\n**Profile URL:**\nhttps://steamcommunity.com/profiles/{accusedSteamId}", inline=True)

                channel = client.get_channel(userReportsId)

                await channel.send(moderatorMention)

                await channel.send(embed = embed)

    except Exception as e:
        print(e)

@tasks.loop(seconds=1, reconnect=True)
async def check_bans():
    try:
        bansResponse = requests.get(f"{serverUrl}/bans").json()

        if "bans" in bansResponse:
            for ban in bansResponse["bans"]:
                moderatorName = ban["moderator"]["name"]
                moderatorSteamId = ban["moderator"]["steamId"]
                accusedName = ban["accused"]["name"]
                accusedSteamId = ban["accused"]["steamId"]
                banReason = ban["reason"]
                banTime = ban["banTime"]

                title = f"PLAYER BANNED FROM OUR SERVERS"
                description = f"**Ban reason:**\n{banReason}\n\n**Ban time:**\n{banTime}"

                embed = discord.Embed(
                    description = description,
                    color = discord.Color(0xFF0000)
                )

                embed.set_author(name=title, icon_url="https://i.ibb.co/0DTZc2g/hammer.png")

                if len(moderatorSteamId) == 17:
                    embed.add_field(name=f"✅ Moderator:", value=f"**Name:** {moderatorName}\n\n**Profile URL:**\nhttps://steamcommunity.com/profiles/{moderatorSteamId}", inline=True)
                elif len(moderatorSteamId) == 18:
                    embed.add_field(name=f"✅ Moderator:", value=f"**Name:** {moderatorName}\n\n**Profile :**\n<@{moderatorSteamId}>", inline=True)

                embed.add_field(name=f"🚫 Accused:", value=f"**Name:** {accusedName}\n\n**Profile URL:**\nhttps://steamcommunity.com/profiles/{accusedSteamId}", inline=True)

                channel = client.get_channel(banLogsId)

                await channel.send(embed = embed)

    except Exception as e:
        print(e)


async def send_error(ctx, error):
    embed = discord.Embed(
        title = "ERROR",
        description = error,
        color = discord.Color(0xFF0000)
    )

    await ctx.send(embed = embed)

async def send_success(ctx, message):
    embed = discord.Embed(
        title = "SUCCESS",
        description = message,
        color = discord.Color(0x00FF00)
    )

    await ctx.send(embed = embed)

async def send_warn(ctx, message):
    embed = discord.Embed(
        title = "WARNING",
        description = message,
        color = discord.Color(0xFF7F00)
    )

    await ctx.send(embed = embed)

@client.hybrid_command(name="restart", description="Restarts one of your game servers")
async def restart(ctx, server_short_name):
    serverRestarted = False

    if is_commands_channel(ctx) and get_permissions_level(ctx.author) >= 1:
        if server_short_name in serverShortNames:
            if watchDog.serverWatchList[server_short_name]:
                await ctx.send("Restarting server. Please wait.")

                try:
                    serverRestarted = watchDog.restart_server(server_short_name)
                except watch_dog.InjectReadyTimeout:
                    await send_error(ctx, "Game start timeout or CCGM mapmod fork not installed.")
                    return
                except watch_dog.InjectTimeout:
                    await send_error(ctx, "Injecting timeout.")
                    return
                except watch_dog.CreateLobbyTimeout:
                    await send_error(ctx, "Lobby creation timeout. Maybe Steam needs reloaded?")
                    return

                if serverRestarted:
                    embed = discord.Embed(
                        description = f"The **{server_short_name}** server has been restarted",
                        color = discord.Color(0x00FF00)
                    )
                    await ctx.send(embed=embed)
                else:
                    await send_error(ctx, "Something went wrong while attempting to restart the server! Wait a minute, make sure it's not running. Then try again.")
            else:
                await send_error(ctx, f"Cannot restart a server that is not started!")
        else:
            await send_error(ctx, f"Invalid server name! You can choose from:\n{serverShortNamesString}")

@client.hybrid_command(name="stop", description="Stops one of your game servers")
async def stop(ctx, server_short_name):
    if is_commands_channel(ctx) and get_permissions_level(ctx.author) == 3:
        if server_short_name in serverShortNames:
            if watchDog.serverWatchList[server_short_name]:
                watchDog.serverWatchList[server_short_name] = False

                gameProcess = watchDog.find_game_process(watchDog.serverSandboxes[server_short_name])
                watchDog.kill_game_process(gameProcess)

                embed = discord.Embed(
                    description = f"The **{server_short_name}** server has been stopped",
                    color = discord.Color(0x00FF00)
                )
                await ctx.send(embed=embed)
            else:
                await send_error(ctx, f"Cannot stop a server that is not started!")
        else:
            await send_error(ctx, f"Invalid server name! You can choose from:\n{serverShortNamesString}")

@client.hybrid_command(name="start", description="Starts one of your game servers")
async def start(ctx, server_short_name):
    if is_commands_channel(ctx) and get_permissions_level(ctx.author) == 3:
        if server_short_name in serverShortNames or server_short_name == "all":
            serverStarted = False
            servers = []

            if server_short_name == "all":
                servers = serverShortNames.copy()
            else:
                servers.append(server_short_name)

            for ssName in servers:
                await ctx.send("Starting server. Please wait.")

                now = time.time()

                try:
                    serverStarted = watchDog.start_server(ssName)
                except watch_dog.InjectReadyTimeout:
                    await send_error(ctx, "Game start timeout or CCGM mapmod fork not installed")
                    return
                except watch_dog.InjectTimeout:
                    await send_error(ctx, "Injecting timeout")
                    return
                except watch_dog.CreateLobbyTimeout:
                    await send_error(ctx, "Lobby creation timeout. Maybe Steam needs reloaded?")
                    return

                if serverStarted:
                    embed = discord.Embed(
                        description = f"The **{ssName}** server has been started",
                        color = discord.Color(0x00FF00)
                    )
                    await ctx.send(embed=embed)
                else:
                    await send_error(ctx, f"Something went wrong while attempting to start the server! Wait a minute, make sure it's not running. Then try again.")
        else:
            await send_error(ctx, f"Invalid server name! You can choose from:\n{serverShortNamesString}")

@client.hybrid_command(name="unban", description="Unbans a player from your game servers")
async def unban(ctx, steam, *, unban_reason):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx):
        if perm >= 2:
            steamId64 = await get_id64(ctx, steam)

            if not steamId64:
                return

            steamName, steamAvatarUrl = await get_steam_profile_info(ctx, steamId64)

            if not steamName or not steamAvatarUrl:
                return
            
            statusCode = requests.post(f"{serverUrl}/unban", json={"steamId": steamId64}).status_code
            
            if statusCode == 200:
                embed = discord.Embed(
                    description = f"**Unban Reason:** {unban_reason}",
                    color = discord.Color(0x00FF00)
                )

                embed.set_thumbnail(url=steamAvatarUrl)
                embed.set_author(name="PLAYER UNBANNED FROM OUR SERVERS", icon_url="https://i.ibb.co/r2QnzRD/unban.png")
                embed.add_field(name=f"✅ Moderator: ", value=f"<@{ctx.author.id}>", inline=True)
                embed.add_field(name=f"✅ Unbanned:", value=f"**Name:** {steamName}\n\n**Profile URL:**\nhttps://steamcommunity.com/profiles/{steamId64}", inline=True)

                channel = client.get_channel(banLogsId)
                await channel.send(embed=embed)
                await send_success(ctx, "Player unbanned from our servers!")
            elif statusCode == 201:
                await send_error(ctx, "User not banned.")
            else:
                await send_error(ctx, "User not found.")

@client.hybrid_command(name="baninfo", description="Loads you a player's ban information")
async def baninfo(ctx, steam):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx) and perm:
        steamId64 = await get_id64(ctx, steam)

        if not steamId64:
            return

        playerConfig = requests.post(f"{serverUrl}/player_info", json={"steamId": steamId64})

        if playerConfig.status_code == 200:
            playerConfig = playerConfig.json()

            if playerConfig["isBanned"]:
                title = f"PLAYER BANNED FROM OUR SERVERS"
                banReason = playerConfig["banReason"]
                moderatorId = "Unknown"

                if "banBy" in playerConfig:
                    moderatorId = str(playerConfig["banBy"])

                    if len(moderatorId) == 17:
                        moderatorId = f"https://steamcommunity.com/profiles/{moderatorId}"
                    elif len(moderatorId) == 18:
                        moderatorId = f'<@{moderatorId}>'

                description = f"**Ban reason:**\n{banReason}"

                embed = discord.Embed(
                    description = description,
                    color = discord.Color(0xFF0000)
                )

                embed.set_author(name=title, icon_url="https://i.ibb.co/0DTZc2g/hammer.png")
                embed.add_field(name=f"✅ Moderator:", value=moderatorId, inline=True)

                await ctx.send(embed = embed)
            else:
                if "banReason" in playerConfig:
                    banReason = playerConfig["banReason"]
                    await send_warn(ctx, f"Player not banned but has been banned before!\n**Reason:** {banReason}")
                else:
                    await send_success(ctx, "User is not banned.")
        else:
            await send_error(ctx, "User was not found in your database.")
            
@client.hybrid_command(name="playerinfo", description="Loads a player's config and displays it")
async def playerinfo(ctx, steam):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx) and perm:
        steamId64 = await get_id64(ctx, steam)

        if not steamId64:
            return

        playerConfig = requests.post(f"{serverUrl}/player_info", json={"steamId": steamId64})

        if playerConfig.status_code == 200:
            playerConfig = playerConfig.json()
            await send_success(ctx, f"```json\n{json.dumps(playerConfig, indent=1)}```")
        else:
            await send_error(ctx, "User was not found in your database.")

def str_replace_index(string, index, value):
    stringList = list(string)
    stringList[index] = value
    return "".join(stringList)

@client.hybrid_command(name="cmd", description="Sends a command to one of your servers")
async def cmd(ctx, server_short_name, *, command):
    user = ctx.author
    args = command.split(" ")
    cmdDiscord = " ".join(args)
    cmdGame = str_replace_index(cmdDiscord, 0, "!") if cmdDiscord.startswith(prefix) else cmdDiscord
    perm = get_permissions_level(user)
    userPermKey = permKeyList[perm]

    print(f"Recieved command from {user.id} -> {''.join(args)}")

    if is_commands_channel(ctx) and perm:
        if (server_short_name in serverShortNames or server_short_name.lower() == "all"):            

            allowCommand = False
            commandDiscord = args[0]
            commandGame = str_replace_index(commandDiscord, 0, "!")

            if userPermKey == "owner":
                allowCommand = True
            else:
                allowCommand = commandGame.lower() in ccgmConfig[userPermKey]

            if allowCommand:
                cmdJson = {"cmd": cmdGame, "sentByUser": f"{user.name}#{user.discriminator}", "sentById": f"{user.id}"}

                if server_short_name.lower() == "all":
                    for ServerName in serverShortNames:
                        requests.post(f"{serverUrl}/do_commands_{ServerName}", json=cmdJson)
                else:
                    requests.post(f"{serverUrl}/do_commands_{server_short_name}", json=cmdJson)

                embed = discord.Embed(
                    description = f"Command sent to **{server_short_name}** server\n> `-> {cmdDiscord}`",
                    color = discord.Color(0x000000)
                )

                embed.set_author(name=f"{ctx.author.name}#{ctx.author.discriminator} sent a command", icon_url=ctx.author.avatar.url)
                await ctx.send(embed = embed)
            else:
                userPermsString = "\n".join(p for p in ccgmConfig[userPermKey])
                await send_error(ctx, f"You only have perms for:\n{userPermsString}")
        else:
            await send_error(ctx, f"Invalid server name! You can choose from:\n{serverShortNamesString}")

@client.command()
async def sync(ctx):
    user = ctx.author
    perm = get_permissions_level(user)

    if is_commands_channel(ctx) and perm == 3:
        await client.tree.sync()
        await send_success(ctx, "Slash commands have been synced!\n**Notice:** This command is heavily rate limited. Avoid using it too often.")
        await ctx.defer()

@client.event
async def on_ready():
    print("CONNECTED TO DISCORD")
    check_reports.start()
    check_bans.start()

if __name__ == '__main__':
    client.run(TOKEN)

    