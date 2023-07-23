import regex
import secrets
from os import listdir

def get_min_max_players(fileName):
    minimum = int(regex.findall("(\d*)min", fileName)[0])
    maximum = int(regex.findall("(\d*)max", fileName)[0])

    return minimum, maximum

def get_animation_files(path):
    animationFiles = []

    for fileName in listdir(path):
        if fileName.endswith(".json"):
            try:
                get_min_max_players(fileName)
            except IndexError:
                continue
            
            animationFiles.append(f"animations\\{fileName}")

    return animationFiles

def get_possible_animations(playerCount):
    possible_animations = []

    for animation_file in get_animation_files("animations"):
        minimum, maximum = get_min_max_players(animation_file)

        if playerCount >= minimum and playerCount <= maximum:
            possible_animations.append(animation_file)

    return possible_animations

def pick_possible_animation(playerCount):
    try:
        return secrets.choice(get_possible_animations(playerCount))
    except IndexError:
        return False