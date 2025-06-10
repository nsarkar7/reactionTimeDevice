#include <vector>
#include "Arduino_BMI270_BMM150.h"
#include "NanoBLEFlashPrefs.h"

#define secs() (millis()/1000)

float xAccel, yAccel, zAccel;

enum stimulusType{
  HAPTIC,
  AUDITORY,
  VISUAL
};
//time in seconds
struct eventCommand{
  stimulusType type;
  int time;
};
//reactionTime in milliseconds
struct eventResult {
  int eventNumber;
  stimulusType type;
  int time;
  int reactionTime;
};

std::vector<eventCommand> events = {{VISUAL, 5}, {HAPTIC, 12}, {VISUAL, 18}, {AUDITORY, 24}, {HAPTIC, 28}, {AUDITORY, 520}};
std::vector<eventResult> data = {};
int eventIndex = -1;
bool written = true;

float getXAccel() {
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(xAccel, yAccel, zAccel);
  }
  return xAccel;
}

void setStimulus(stimulusType stimulus, int state){
  if(stimulus==0){
    digitalWrite(A1, state);
  }
  else if(stimulus==1){
    digitalWrite(D2, state);
  }
    else if(stimulus==2){
    digitalWrite(LED_BUILTIN, state);
  }
}

void setup() {
  pinMode(D2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A1, OUTPUT);
  
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("----09809");
}

void loop() {
  int index = -1;
  if(events.size()>0){
    written = false;
    for (eventCommand event : events) {
      index++;
      
      if(secs() >= event.time){
        long reactionTime = -1;
        long startTime = millis();

        eventIndex++;
        setStimulus(event.type, 1);

        while(millis() - startTime < 2000) {
          if(getXAccel()>500 || getXAccel()<-500){
              reactionTime = millis()-startTime;
              setStimulus(event.type, 0);
              break;            
          }
          delay(10);
        }
        setStimulus(event.type, 0);

        data.push_back(eventResult {eventIndex, event.type, event.time, reactionTime});
        events.erase(events.begin() + index);
      }
    }
  }
  else if(!written){
    
  }
  //Serial.println(secs());
  delay(1000);
  //Serial.println(secs());
}