#include <Arduino.h>

/*
 * Arduino code to control a Robot Parrot by randomly pushing one of its remote's 6 phrase buttons every time a human 
 * presence is detected. After detecting a human presence the system idles for IDLE_TIME_SECONDS before 
 * another detection can occur.
 */

#define IDLE_TIME_SECONDS 10 // One hour: 3600
#define BUTTON_HOLD_MS 250 // The duration of a button press on the remote.

#define SENSOR_PIN 2
#define RELAY_PIN_0 3
#define RELAY_PIN_COUNT 6

unsigned long StartTime = millis();
unsigned long IdleTimeMs = IDLE_TIME_SECONDS * 1000; // Ignore changes for this many milliseconds
unsigned long LastTriggerTime = StartTime - IdleTimeMs;
int RandomPresses[RELAY_PIN_COUNT];
int CurrentPress = RELAY_PIN_COUNT;

int getRelayPin(int relayNr);
void setRelay(int relayNr, int state);
bool isIdled();
void pressRandomButton();
int getRandomPress();
bool isMotionDetected();
void loadRandomPresses();

void setup() {
  // Set PIR sensor pin to input
  pinMode(SENSOR_PIN, INPUT);
  // Set relay pins to output and turn them off
  for (int i = 0; i < RELAY_PIN_COUNT; i++) {
    pinMode(getRelayPin(i), OUTPUT);
    setRelay(i, HIGH);
  }
  Serial.begin(9600);
  // Initialize the random number generator to the random floating value of Analog input pin 3.
  randomSeed(analogRead(A3));
}

void loop() {
  if (isMotionDetected())
    pressRandomButton();
}

bool isMotionDetected() {
  return !isIdled() && digitalRead(SENSOR_PIN) == HIGH;
}

int getRandomPress() {
  if (CurrentPress >= RELAY_PIN_COUNT)
    loadRandomPresses();
  return RandomPresses[CurrentPress++];
}

// Load a batch of unique button presses
void loadRandomPresses() {
  Serial.print("Loading presses: ");
  for (int i = 0; i < RELAY_PIN_COUNT; i++) {
    retry:
      int press = random(RELAY_PIN_COUNT);
      Serial.print(press);
      if (i == 0 && press == RandomPresses[RELAY_PIN_COUNT-1]) {
        // Don't allow a new batch to start with a repeat of the final press of the last batch
        Serial.print("←");
        goto retry;
      }
      for (int j = 0; j < i; j++) {
        if (press == RandomPresses[j]) {
          Serial.print("←");
          goto retry;
        }
      }
      RandomPresses[i] = press;
  }
  CurrentPress = 0;
  Serial.println();
}

void pressRandomButton() {
  int buttonNumber = getRandomPress();
  Serial.print("Button: ");
  Serial.println(buttonNumber);
  setRelay(buttonNumber, LOW);
  delay(BUTTON_HOLD_MS);
  setRelay(buttonNumber, HIGH);
  LastTriggerTime = millis();
}

int getRelayPin(int relayNr) {
  return RELAY_PIN_0 + relayNr;
}

void setRelay(int relayNr, int state) {
  return digitalWrite(getRelayPin(relayNr), state);
}

bool isIdled() {
  return millis() < LastTriggerTime + IdleTimeMs;
}