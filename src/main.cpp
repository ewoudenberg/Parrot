#include <Arduino.h>

/*
 * Arduino code to control a Robot Parrot by randomly pushing one of its remote's 6 phrase buttons every time a human 
 * presence is detected. After detecting a human presence the system idles for IDLE_TIME_SECONDS before 
 * another detection can occur.
 */

#define IDLE_TIME_SECONDS 10 // One hour: 3600
#define BUTTON_HOLD_MS 250 // The duration of a button press on the remote.

#define PIR_SENSOR_IN 2
#define IR_REMOTE_IN 12
#define RELAY_PIN_0 3
#define BUTTON_COUNT 6

unsigned long StartTime = millis();
unsigned long IdleTimeMs = IDLE_TIME_SECONDS * 1000; // Ignore changes for this many milliseconds
unsigned long LastTriggerTime = StartTime - IdleTimeMs;
int RandomPresses[BUTTON_COUNT];
int CurrentPress = BUTTON_COUNT;

int getRelayPin(int relayNr);
void setRelay(int relayNr, int state);
bool isIdled();
void pressRandomButton();
int getRandomPress();
bool isMotionDetected();
void loadRandomPresses();
void IRsetup();
void IRloop();

void setup() {
  IRsetup();
  pinMode(PIR_SENSOR_IN, INPUT);
  // Set relay pins to output and turn them off
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(getRelayPin(i), OUTPUT);
    setRelay(i, HIGH);
  }
  Serial.begin(9600);
  // Initialize the random number generator to the random floating value of Analog input pin 3.
  randomSeed(analogRead(A3));
}

void loop() {
  IRloop();
  if (isMotionDetected())
    pressRandomButton();
}

bool isMotionDetected() {
  return !isIdled() && digitalRead(PIR_SENSOR_IN) == HIGH;
}

int getRandomPress() {
  if (CurrentPress >= BUTTON_COUNT)
    loadRandomPresses();
  return RandomPresses[CurrentPress++];
}

// Load a batch of unique button presses
void loadRandomPresses() {
  Serial.print("Loading presses: ");
  for (int i = 0; i < BUTTON_COUNT; i++) {
    retry:
      int press = random(BUTTON_COUNT);
      Serial.print(press);
      if (i == 0 && press == RandomPresses[BUTTON_COUNT-1]) {
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

#include <IRremote.h>

const int IR_RECEIVE_PIN = 12;  // Define the pin number for the IR Sensor
String lastDecodedValue = "";  // Variable to store the last decoded value
String decodeKeyValue(long result);

void IRsetup() {
  Serial.begin(9600);                                     // Start serial communication at 9600 baud rate
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  // Start the IR receiver
}

long LastIrReceived = millis();
#define IR_DEBOUNCE_MS 120

int getDebouncedIRCode() {
  if (IrReceiver.decode()) {
    int code = IrReceiver.decodedIRData.command;
    IrReceiver.resume();  // Enable receiving of the next value

    long deltaMs = millis() - LastIrReceived;
    LastIrReceived = millis();
    if (deltaMs > IR_DEBOUNCE_MS)
      return code;
  }
  return 0;
}

void IRloop() {
  int irCode = getDebouncedIRCode();
  if (irCode) {
    String decodedValue = decodeKeyValue(irCode);
    Serial.print(irCode);
    if (decodedValue != "ERROR") {
      Serial.print(" ");
      Serial.print(decodedValue);
    }
    Serial.println();
  }
}


String decodeKeyValue(long result)
{
  switch(result){
    case 0x16:
      return "0";
    case 0xC:
      return "1"; 
    case 0x18:
      return "2"; 
    case 0x5E:
      return "3"; 
    case 0x8:
      return "4"; 
    case 0x1C:
      return "5"; 
    case 0x5A:
      return "6"; 
    case 0x42:
      return "7"; 
    case 0x52:
      return "8"; 
    case 0x4A:
      return "9"; 
    case 0x9:
      return "+"; 
    case 0x15:
      return "-"; 
    case 0x7:
      return "EQ"; 
    case 0xD:
      return "U/SD";
    case 0x19:
      return "CYCLE";         
    case 0x44:
      return "PLAY/PAUSE";   
    case 0x43:
      return "FORWARD";   
    case 0x40:
      return "BACKWARD";   
    case 0x45:
      return "POWER";   
    case 0x47:
      return "MUTE";   
    case 0x46:
      return "MODE";       
    case 0x0:
      return "ERROR";   
    default :
      return "ERROR";
    }
}
