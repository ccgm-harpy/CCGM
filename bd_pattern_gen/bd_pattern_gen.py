import json
from os import system
from os.path import isfile
from time import sleep

class BlockDropAnimator():
    def __init__(self):
        self.animation = {"animation": []}
        self.sequenceTemplate = '{"speed": {}, "time": {}, "iteration_time": {}, "coords": {}}'
        self.animationFileName = ""
    
        self.currentSequenceTime = 0
        
        self.mode = 0
        self.modes = ["Choosing", "Making", "Editing"]
        self.sequenceNumber = 0
        self.coordsNumber = 0

    def get_sequence_size(self):
        return len(self.animation["animation"])

    def get_coords_size(self, index):
        return len(self.animation["animation"][index]["coords"])

    def cmd_clear(self):
        system("cls")
        if self.mode == 0:
            print("Choosing\n")
        else:
            title = "{}\n{} Animation | {}".format(
                '-' * 60,
                self.modes[self.mode],
                self.animationFileName
            )

            print(f"{title.upper()}\n")

    def save_animation_file(self):
        while True:
            self.cmd_clear()
            print("Save animation? (y/n)\n")
            choice = input("-> ")

            if choice.lower() in ["y", "n"]:
                if choice.lower() == "n":
                    break
                else:
                    self.cmd_clear()
                    fileName = str(input("File Name -> "))

                    if fileName.endswith(".json"):
                        fileName = fileName[:-5]

                    with open(f"{fileName}.json", "w") as f:
                        f.write(json.dumps(self.animation, indent=1))

                    self.cmd_clear()
                    print("Animation saved.")
                    sleep(3)
                    return
            else:
                self.cmd_clear()
                print("Invalid choice. Must be y or n")
                sleep(3)

    def load_animation_file(self):
        while True:
            self.cmd_clear()
            fileName = str(input("File Name -> "))

            if fileName.endswith(".json"):
                fileName = fileName[:-5]

            fileName = f"{fileName}.json"

            if isfile(fileName):
                with open(fileName, "r") as f:
                    try:
                        animation = json.load(f)
                        
                    except:
                        self.cmd_clear()
                        print("Loading animation file failed!")
                        sleep(3)
                    else:
                        self.animationFileName = fileName
                        self.cmd_clear()
                        print("Animation loaded.")
                        sleep(1.5)
                        return animation
            else:
                self.cmd_clear()
                print("File does not exist!")
                sleep(3)

    def prompt_make_edit(self):
        while True:
            self.cmd_clear()
            print("What would you like to do?")
            print("1. MAKE block drop animation")
            print("2. EDIT block drop animation\n")

            choice = input("-> ")

            try: choice = int(choice) 
            except ValueError: pass

            if not choice in [1,2]:
                self.cmd_clear()
                print("Invalid choice! Type 1 or 2.")
                sleep(3)
                continue

            break

        return choice

    def get_fall_speed(self):
        while True:
            self.cmd_clear()
            print("Block(s) fall speed (0.2 - 3.0)\n")

            fallSpeed = input("-> ")

            try: fallSpeed = float(fallSpeed) 
            except ValueError: pass

            if not isinstance(fallSpeed, float) or fallSpeed > 3 or fallSpeed < 0.2:
                self.cmd_clear()
                print("Invalid choice! Must be between 0.2 - 3.0")
                sleep(4)
                continue

            break

        return fallSpeed
            
    def get_sequence_time(self):
        while True:
            self.cmd_clear()
            print("Sequence start time (in milliseconds after round start / last sequence)\n")

            sequenceTime = input("-> ")

            try: sequenceTime = int(sequenceTime) 
            except ValueError: pass

            if not isinstance(sequenceTime, int):
                self.cmd_clear()
                print("Invalid choice! Must be time in milliseconds")
                sleep(4)
                continue

            break

        return sequenceTime

    def get_iteration_time(self):
        while True:
            self.cmd_clear()
            print("Iteration time (in milliseconds)\n")

            iterationTime = input("-> ")

            try: iterationTime = int(iterationTime) 
            except ValueError: pass

            if not isinstance(iterationTime, int):
                self.cmd_clear()
                print("Invalid choice! Must be time in milliseconds")
                sleep(4)
                continue

            break

        return iterationTime

    def get_coords(self, iterationTime, insertedCoords=False):
        if insertedCoords:
            coordsList = insertedCoords
        else:
            coordsList = []

        while True:
            insertCoords = False
            insertIndex = 0

            self.cmd_clear()

            for i, coords in enumerate(coordsList):
                print(f"#{i+1} DROPPING BLOCKS {json.dumps(coords)} AT TIME {self.currentSequenceTime + (i*iterationTime)}")

            print("\nBlock coords (If multiple, seperate by comma)")
            print("Type 'done' when you're done")
            print("Type 'del index' to delete coords at index")
            print("Type 'ins index cord1, cord2'... to insert coords at index\n")

            coordsString = str(input("-> "))

            if coordsString:
                if coordsString.lower() == "done":
                    return coordsList

                elif coordsString.startswith("del"):
                    try:
                        deleteIndex = int(coordsString.split(" ")[1].replace("#", ""))
                        del coordsList[deleteIndex-1]
                        continue
                    except (ValueError, IndexError):
                        self.cmd_clear()
                        print("Invalid! Syntax: 'del index'")
                        sleep(3)
                        continue

                elif coordsString.startswith("ins"):
                    try:
                        insertIndex = int(coordsString.split(" ")[1].replace("#", ""))
                        insertCoords = True
                        coordsString = "".join(coordsString.split(" ")[2::])

                    except (ValueError, IndexError):
                        self.cmd_clear()
                        print("Invalid! Syntax: 'ins index cord1, cord2'...")
                        sleep(3)
                        continue

                coords = []
                coordsStringList = coordsString.replace(" ", "").split(",")

                try:
                    for coord in coordsStringList:
                        coord = int(coord)
                        
                        if coord < 1 or coord > 16:
                            raise ValueError

                        coords.append(coord)
                    
                    if insertCoords:
                        coordsList.insert(insertIndex-1, coords)
                    else:
                        coordsList.append(coords)

                except ValueError:
                    self.cmd_clear()
                    print("Invalid choice! Must be 1-16 seperated by commas")
                    sleep(5)
                    continue
            
            else:
                self.cmd_clear()
                print("Invalid choice! Must be 1-16 seperated by commas")
                sleep(5)

    def get_sequence(self):
        self.currentSequenceTime += self.get_sequence_time()

        if self.sequenceNumber:
            lastSequenceIterationTime = self.animation["animation"][self.sequenceNumber-1]["iteration_time"]
            lastSeqenceLength = len(self.animation["animation"][self.sequenceNumber-1]["coords"]) - 1
            self.currentSequenceTime += lastSequenceIterationTime * lastSeqenceLength

        iterationTime = self.get_iteration_time()
        fallSpeed = self.get_fall_speed()
        coords = self.get_coords(iterationTime)

        sequence = {
            "speed": fallSpeed,
            "time": self.currentSequenceTime,
            "iteration_time": iterationTime,
            "coords": coords
        }

        return sequence

    def make_sequences(self):
        while True:
            self.sequenceNumber = self.get_sequence_size()

            sequence = self.get_sequence()

            if sequence:
                self.animation["animation"].append(sequence)
                self.cmd_clear()
                print("Add another sequence? (y,n)\n")

                choice = str(input("-> "))

                if not choice in "yn":
                    self.cmd_clear()
                    print("Invalid choice! Must be y or n")
                    sleep(3)
                    continue
                
                if choice == "n":
                    break

    def edit_sequences(self):
        while True:
            self.cmd_clear()

            for si, sequence in enumerate(self.animation["animation"]):
                if si:
                    print("")

                print(f"#{si+1} SEQUENCE AT TIME {sequence['time']} (block fall speed {sequence['speed']}) (next block time {sequence['iteration_time']})")

                for ci, coords in enumerate(self.animation["animation"][si]["coords"]):
                    print(f"    #{ci+1} DROPPING BLOCKS {json.dumps(coords)} AT TIME {sequence['time'] + (ci*sequence['iteration_time'])}")
            
            print("\nType 'done' when you're done")
            print("Type 'del index' to delete a sequence at index")
            print("Type 'ins index' to insert a new sequence at index")
            print("Type 'add' to add a new sequence")
            print("Type 'edit index' to edit a sequence at index\n")

            command = str(input("-> "))

            if command.startswith("done"):
                break

            elif command.startswith("del"):
                try:
                    deleteIndex = int(command.split(" ")[1].replace("#", ""))
                    self.animation["animation"][deleteIndex-1]
                except (ValueError, IndexError):
                    self.cmd_clear()
                    print("Invalid! Syntax: 'del index'")
                    sleep(3)
                    continue

                if len(self.animation["animation"]) > 1:
                    if not deleteIndex == len(self.animation["animation"]):
                        sequenceTime = self.animation["animation"][deleteIndex-1]["time"]
                        sequenceIterationTime = self.animation["animation"][deleteIndex-1]["iteration_time"]
                        sequenceSize = len(self.animation["animation"][deleteIndex-1]) - 1
                        sequenceEndTime = sequenceTime + (sequenceIterationTime * sequenceSize)

                        nextSequenceTime = self.animation["animation"][deleteIndex]["time"]
                        timeDifference = nextSequenceTime - sequenceEndTime

                        if not deleteIndex == 1:
                            previousSequenceTime = self.animation["animation"][deleteIndex-2]["time"]
                            previousSequenceIterationTime = self.animation["animation"][deleteIndex-2]["iteration_time"]
                            previousSequenceSize = len(self.animation["animation"][deleteIndex-2]) - 1
                            previousSequenceEndTime = previousSequenceTime + (previousSequenceIterationTime * previousSequenceSize)

                            timeDifference2 = sequenceTime - previousSequenceEndTime

                            subtractTime = (sequenceEndTime - sequenceTime) + timeDifference2

                            for i, _sequence in enumerate(self.animation["animation"][deleteIndex::]):
                                self.animation["animation"][i+deleteIndex]["time"] -= subtractTime

                        else:
                            subtractTime = (sequenceEndTime - sequenceTime) + timeDifference

                            for i, _sequence in enumerate(self.animation["animation"][deleteIndex::]):
                                self.animation["animation"][i+deleteIndex]["time"] -= subtractTime

                del self.animation["animation"][deleteIndex-1]

            elif command.startswith("ins"):
                try:
                    insertIndex = int(command.split(" ")[1].replace("#", ""))
                    self.animation["animation"][insertIndex-1]
                except (ValueError, IndexError):
                    self.cmd_clear()
                    print("Invalid! Syntax: 'ins index'")
                    sleep(3)
                    continue
                
                self.sequenceNumber = insertIndex-1

                if self.sequenceNumber:
                    self.currentSequenceTime = self.animation["animation"][self.sequenceNumber-1]["time"]
                else:
                    self.currentSequenceTime = 0
                
                insertSequence = self.get_sequence()

                if insertSequence:
                    if self.sequenceNumber == 0:
                        timeDifference = self.animation["animation"][self.sequenceNumber]["time"]
                    else:
                        previousSequenceTime = self.animation["animation"][self.sequenceNumber-1]["time"]
                        previousSequenceIterationTime = self.animation["animation"][self.sequenceNumber-1]["iteration_time"]
                        previousSequenceSize = len(self.animation["animation"][self.sequenceNumber-1]["coords"]) - 1
                        timeDifference = self.animation["animation"][self.sequenceNumber]["time"] - (previousSequenceTime + previousSequenceIterationTime*previousSequenceSize)

                    sequenceIterationTime = insertSequence["iteration_time"]
                    sequenceSize = len(insertSequence["coords"]) - 1

                    for i, _sequence in enumerate(self.animation["animation"][self.sequenceNumber::]):
                        self.animation["animation"][i+self.sequenceNumber]["time"] += (sequenceIterationTime*sequenceSize) + timeDifference

                    self.animation["animation"].insert(self.sequenceNumber, insertSequence)
            
            elif command.startswith("add"):
                self.currentSequenceTime = self.animation["animation"][-1]["time"]
                self.sequenceNumber = len(self.animation["animation"])

                sequence = self.get_sequence()
    
                self.animation["animation"].append(sequence)

            elif command.startswith("edit"):
                try:
                    editIndex = int(command.split(" ")[1].replace("#", ""))
                    self.animation["animation"][editIndex-1]
                except (ValueError, IndexError):
                    self.cmd_clear()
                    print("Invalid! Syntax: 'edit index'")
                    sleep(3)
                    continue

                self.currentSequenceTime = self.animation["animation"][editIndex-1]["time"]
                self.sequenceNumber = editIndex - 1

                lastSequenceTime = self.animation["animation"][editIndex-1]["time"]
                lastSequenceIterationTime = self.animation["animation"][editIndex-1]["iteration_time"]
                lastSequenceSize = len(self.animation["animation"][editIndex-1]["coords"]) - 1
                lastSequenceEndTime = lastSequenceIterationTime * lastSequenceSize

                sequence = self.edit_sequence()

                sequenceTime = sequence["time"]
                sequenceIterationTime = sequence["iteration_time"]
                sequenceSize = len(sequence["coords"]) - 1
                sequenceEndTime = sequenceIterationTime * sequenceSize

                addTime = (sequenceTime - lastSequenceTime) + (sequenceEndTime - lastSequenceEndTime)

                for i, _sequence in enumerate(self.animation["animation"][self.sequenceNumber+1::]):
                    self.animation["animation"][i+self.sequenceNumber+1]["time"] += addTime

                self.animation["animation"][self.sequenceNumber] = sequence

    def edit_sequence(self):
        sequence = self.animation["animation"][self.sequenceNumber]

        while True:
            self.cmd_clear()
            
            print(f"#{self.sequenceNumber+1} SEQUENCE AT TIME {sequence['time']} (block fall speed {sequence['speed']}) (next block time {sequence['iteration_time']})")

            for ci, coords in enumerate(sequence["coords"]):
                print(f"    #{ci+1} DROPPING BLOCKS {json.dumps(coords)} AT TIME {sequence['time'] + (ci*sequence['iteration_time'])}")

            print("\nType 'done' when you're done")
            print("Type 'time timeInMilliseconds' to change the sequence start time")
            print("Type 'speed speed0.2to3' to change the block fall speed")
            print("Type 'iterate timeInMilliseconds' to change the next block drop time")
            print("Type 'coords' to edit coords\n")

            command = str(input("-> "))

            if command.startswith("done"):
                return sequence

            elif command.startswith("time"):
                try: 
                    sequenceTime = int(command.split(" ")[1]) 
                except (ValueError, IndexError):
                    self.cmd_clear()
                    print("Invalid! Syntax: 'time timeInMilliseconds'")
                    sleep(3)
                    continue

                sequence["time"] = sequenceTime

            elif command.startswith("speed"):
                try: 
                    fallSpeed = float(command.split(" ")[1]) 

                    if fallSpeed > 3 or fallSpeed < 0.2:
                        raise ValueError

                except (ValueError, IndexError):
                    self.cmd_clear()
                    print("Invalid choice! Must be between 0.2 - 3.0")
                    sleep(4)
                    continue

                sequence["speed"] = fallSpeed

            elif command.startswith("iterate"):
                try: 
                    iterateTime = int(command.split(" ")[1]) 
                except (ValueError, IndexError):
                    self.cmd_clear()
                    print("Invalid! Syntax: 'iterate timeInMilliseconds'")
                    sleep(3)
                    continue

                sequence["iteration_time"] = iterateTime
            
            elif command.startswith("coords"):
                sequence["coords"] = self.get_coords(sequence["iteration_time"], insertedCoords=sequence["coords"])

    def loop(self):
        while True:
            self.animation = {"animation": []}
            self.sequenceNumber = 0
            self.coordsNumber = 0
            self.mode = 0
            self.mode = self.prompt_make_edit()

            if self.mode == 1:
                self.make_sequences()
                self.save_animation_file()

            elif self.mode == 2:
                self.animation = self.load_animation_file()
                self.edit_sequences()
                self.save_animation_file()


if __name__ == "__main__":
    BlockDropAnimator().loop()
