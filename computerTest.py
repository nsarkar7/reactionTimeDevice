from tkinter import *
import tkinter.messagebox
from enum import Enum
from dataclasses import dataclass
from datetime import datetime, timezone
import pygame
import threading
import time
import keyboard
import json


pygame.mixer.init()

root = tkinter.Tk()
root.title("Reaction Time Test - Computer")
root.geometry('800x600')

startTime = time.perf_counter()

class StimulusType(Enum):
    VISUAL = 0
    AUDITORY = 1

@dataclass
class EventCommand:
    stimulus: StimulusType
    time: int

@dataclass
class EventResult:
    eventNumber: int
    stimulus: StimulusType
    time: int
    reactionTime: int


events = [EventCommand(StimulusType.VISUAL, 5), EventCommand(StimulusType.VISUAL, 8)]
data = []

triggered = False

sound = pygame.mixer.Sound("beep.wav")

def setStimulus(stimulus, state):
    global soundChannel
    
    def update_bg(color):
        root.configure(bg=color)

    if stimulus == StimulusType.VISUAL and state == 1:
        root.after(0, update_bg, "green")
    elif stimulus == StimulusType.VISUAL and state == 0:
        root.after(0, update_bg, "red")
    elif stimulus == StimulusType.AUDITORY and state == 1:
        sound.play(-1)
    elif stimulus == StimulusType.AUDITORY and state == 0:
        sound.stop()

def Start():
    global startTime
    startTime = time.perf_counter()
    tkinter.messagebox.showinfo("start")

def triggerPress(event):
    global triggered
    triggered = True
    
def triggerRelease(event):
    global triggered
    if event.keysym == 'space':
        triggered = False

def saveData(data):
    with open('data.json','r+') as file:
        savedData = json.load(file)
        savedData["saved_trials"].append(data)
        file.seek(0)
        json.dump(savedData, file, indent=4)

def loop():
    global triggered

    while True:
        #print(time.time()
        index = -1
        if len(events) > 0:
            for event in events:
                index+= 1
                if time.perf_counter()-startTime > event.time:
                    reactionTime = -1
                    eventStartTime = time.perf_counter()
                    setStimulus(event.stimulus, 1)

                    while(time.perf_counter()-eventStartTime < 2):
                        if triggered:
                            print(time.time())
                            reactionTime = time.perf_counter()-eventStartTime
                            setStimulus(event.stimulus, 0)
                            breakEvent = True
                            break

                        time.sleep(0.01)

                    setStimulus(event.stimulus, 0)
                    data.append(EventResult(index, event.stimulus, event.time, reactionTime*1000))
                    del events[index]
        else:
            utc_time = datetime.now(timezone.utc)
            serializableData = []
            for result in data:
                serializableData.append({
                    "eventNumber": result.eventNumber,
                    "stimulus": result.stimulus.name,
                    "time": result.time,
                    "reactionTime": result.reactionTime
                    })

            saveData({"utc_time": utc_time.strftime("%Y-%m-%d %H:%M:%S UTC"), "participant_age": "", "results": serializableData})
            break
        time.sleep(0.01)

asyncThread = threading.Thread(target=loop)
asyncThread.start()

root.configure(bg="red")

StartButton = Button(root, text="Start Test", command=Start, pady=10)

StartButton.pack(side=BOTTOM)

root.bind("<space>", triggerPress)
root.bind("<KeyRelease>", triggerRelease)
root.mainloop()
