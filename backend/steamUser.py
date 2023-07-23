import requests
import json
from time import sleep

class UserNotFound(Exception):
    pass

class InvalidJson(Exception):
    pass

class InvalidProfileUrl(Exception):
    pass

class InvalidIdOrUrl(Exception):
    pass

class SteamUser():
    def __init__(self, STEAMAPIKEY):
        self.STEAMAPIKEY = STEAMAPIKEY

    def is_id64(self, id64):
        try:
            int(id64)
        except:
            return False
        return True

    def get_id64(self, message):
        if "steamcommunity.com/profiles/" in message:
            if message.endswith("/"):
                message = message[:-1]
                
            steam64 = message.split("/")[-1]

            if self.is_id64(steam64):
                return steam64
            else:
                raise InvalidIdOrUrl
        
        elif "steamcommunity.com/id/" in message:
            steam64 = message.split("steamcommunity.com/id/")[1]
            steam64 = steam64.split("/")[0]
            apiurl = f"https://api.steampowered.com/ISteamUser/ResolveVanityURL/v0001/?key={self.STEAMAPIKEY}&vanityurl={steam64}"

            try:
                response = json.loads(requests.get(apiurl).content.decode("utf-8"))
            except json.decoder.JSONDecoderError:
                raise InvalidJson
            
            try:
                if response["response"]["success"] == 1:
                    steam64 = response["response"]["steamid"]
                    return steam64
                else:
                    raise UserNotFound
            except KeyError:
                raise InvalidJson
            
        elif self.is_id64(message):
            return int(message)
        
        else:
            raise InvalidIdOrUrl
            

    def get_profile_info(self, steam64):
        apiurl = f"http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key={self.STEAMAPIKEY}&steamids={steam64}"
        
        try:
            while True:
                r = requests.get(apiurl)
                
                if r.status_code == 429:
                    sleep(5)
                    continue

                break

            response = json.loads(r.content)
        except json.decoder.JSONDecodeError:
            raise InvalidJson
        
        try:
            if response["response"]["players"]:
                user = response["response"]["players"][0]
                userName = user["personaname"]
                avatarUrl = user["avatarmedium"]
            else:
                raise UserNotFound
        except KeyError:
            raise InvalidJson

        return (userName, avatarUrl)