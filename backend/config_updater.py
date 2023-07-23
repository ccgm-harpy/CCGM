import json
import requests
#from pythonLib import cryptoTools
from os.path import isfile
from shutil import copy
from collections import abc

class ccgmConfigMissing(Exception):
    pass

"""
not implemented
def update_server_auth(ccgmConfig):
    if not "webServerAuthKey" in ccgmConfig:
        webServerPrivateKey = cryptoTools.generate_ec_key()
        ccgmConfig["webServerAuthKey"] = cryptoTools.ec_key_to_pem(webServerPrivateKey)

    if not "clientAuthKey" in ccgmConfig:
        clientAuthKey = cryptoTools.generate_ec_key()
        ccgmConfig["clientAuthKey"] = cryptoTools.ec_key_to_pem(clientAuthKey)

    if not "clientAuthToken" in ccgmConfig:
        clientAuthToken = cryptoTools.generate_token()
        ccgmConfig["clientAuthToken"] = clientAuthToken

    return ccgmConfig"""

def read_ccgm_config():
    if isfile("ccgm_config.json"):
        with open("ccgm_config.json", "r") as f:
            return json.load(f)
    else:
        raise ccgmConfigMissing

def copy_ccgm_config():
    copy("ccgm_config.json", "ccgm_config_backup.json")

def update_ccgm_config_servers(oldConfig, newConfig):
    for key, value in newConfig.items():
        if isinstance(value, abc.Mapping):
            oldConfig[key] = update_ccgm_config_servers(oldConfig.get(key, {}), value)
        else:
            if not key in oldConfig:
                oldConfig[key] = value

    return oldConfig

def update_ccgm_config_root(oldConfig, newConfig):
    for key, value in zip(newConfig.keys(), newConfig.values()):
        if not key in oldConfig:
            oldConfig[key] = value
    
    return oldConfig

def request_new_config():
    return json.loads(
        requests.get("https://pastebin.com/raw/NMLkj8CC").content
    )



def update_ccgm_config(ccgmConfig):
    newConfig = request_new_config()
    ccgmConfig = update_ccgm_config_root(ccgmConfig, newConfig)
    
    newServerConfig = newConfig["servers"]["my server name 1"]

    for serverName in ccgmConfig["servers"]:
        oldServerConfig = ccgmConfig["servers"][serverName]
        ccgmConfig["servers"][serverName] = update_ccgm_config_servers(oldServerConfig, newServerConfig)

    #ccgmConfig = update_server_auth(ccgmConfig)

    return ccgmConfig

def save_ccgm_config(ccgmConfig):
    with open("ccgm_config.json", "w") as f:
        f.write(json.dumps(ccgmConfig, indent=1))

def main():
    try:
        ccgmConfig = read_ccgm_config()
    except ccgmConfigMissing:
        print("Could not find your ccgm_config.json file!")
        print("Place me inside the same folder as your config!")
        input("Press Enter to continue-> ")
        return
    
    copy_ccgm_config()
    ccgmConfig = update_ccgm_config(ccgmConfig)
    save_ccgm_config(ccgmConfig)

    print("Your ccgm_config.json file has been updated!")
    input("You may now close the program.")

if __name__ == "__main__":
    main()
