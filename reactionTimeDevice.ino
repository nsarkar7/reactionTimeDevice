#include <vector>
#include "Arduino_BMI270_BMM150.h"
#include "NanoBLEFlashPrefs.h"

#define secs() (millis()/1000)

NanoBLEFlashPrefs flash;

float xAccel, yAccel, zAccel;

enum stimulusType{
  HAPTIC,
  AUDITORY,
  VISUAL
};

const char* stimulusNames[] = {"HAPTIC", "AUDITORY", "VISUAL"};
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

struct FlashData {
  int numberEvents;
  eventResult results[50];
};

std::vector<eventCommand> events = {{HAPTIC, 5}, {AUDITORY, 8}, {VISUAL, 15},{HAPTIC, 20},{HAPTIC, 25},{AUDITORY, 28}};
std::vector<eventResult> data = {};

int eventIndex = -1;
bool written = false;
bool printed = false;

FlashData readData;

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
void displayResults(eventResult results[], int numberEvents) {
  Serial.println("Results:");
  for (int i=0; i<numberEvents; i++) {
    Serial.print("Trial #: ");
    Serial.print(results[i].eventNumber);
    Serial.print(", Type: ");
    Serial.print(stimulusNames[results[i].type]);
    Serial.print(", Event Time: ");
    Serial.print(results[i].time);
    Serial.print(" sec, Reaction Time: ");
    Serial.print(results[i].reactionTime);
    Serial.println(" msec");
  }
}

void setup() {
  pinMode(D2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A1, OUTPUT);
  
  Serial.begin(9600);
  delay(2000);
  //while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    digitalWrite(D2, HIGH);
    //while (1);
  }
  digitalWrite(D2, LOW);
}

void loop() {
  int index = -1;
  if(Serial){
    if(!printed){
      Serial.println("--> r - Read Saved Data, c - Clear Saved data");
      printed = true;
    }
    String cmd = Serial.readString();
    cmd.trim();

    if(cmd == "r"){
      int rc = flash.readPrefs(&readData, sizeof(readData));
      if(rc == 0){
        Serial.println("Saved Data:");
        displayResults(readData.results, readData.numberEvents);
        Serial.println("--> Command Successfully Executed");
      }
      else{
        Serial.println("--> No Saved Data Available");
      }
    }
    else if(cmd == "c"){
      Serial.println("--> Marking Data Invalid");
      int rc = flash.deletePrefs();
      Serial.println(flash.errorString(rc));
      Serial.println("--> Running Garbage Collection");
      int rc2 = flash.garbageCollection();
      Serial.println(flash.errorString(rc2));
      Serial.println("--> Clear Complete");
    }

  }
  else if(events.size()>0){
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
    FlashData storedData;

    storedData.numberEvents = data.size();
    for (int i=0; i<storedData.numberEvents; i++){
      storedData.results[i] = data[i];
    }
    int rc = flash.writePrefs(&storedData, sizeof(storedData));

    if(rc == 0){
      //Success sequence
      Serial.println("Successfully Flashed");
      digitalWrite(A1, HIGH);
      delay(200);
      digitalWrite(A1, LOW);
      digitalWrite(D2, HIGH);
      delay(200);
      digitalWrite(D2, LOW);
      digitalWrite(A1, HIGH);
      delay(200);
      digitalWrite(A1, LOW);
      digitalWrite(D2, HIGH);
      delay(200);
      digitalWrite(D2, LOW);
      digitalWrite(A1, HIGH);
      delay(1000);
      digitalWrite(A1, LOW);
    }
    else{
      //Cant write error sequence
      Serial.println(rc);
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(A1, HIGH);
      delay(200);
      digitalWrite(A1, LOW);
      delay(200);
      digitalWrite(A1, HIGH);
      delay(200);
      digitalWrite(A1, LOW);
      delay(200);
      digitalWrite(A1, HIGH);
      delay(200);
      digitalWrite(A1, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }
    written = true;
  }
  delay(100);
}