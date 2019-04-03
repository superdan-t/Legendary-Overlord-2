/*
   LightingControlMaster by Daniel Tierney (https://github.com/RandomShrub)

   Thanks to Simon Merrett for interrupt-based rotary encoder setup, who also credits Oleg Mazurov, Nick Gammon, rt, Steve Spence

*/

#include <EEPROM.h>

String inputString = "";
boolean inputComplete = false;

const byte encoderPinA = 2;
const byte encoderPinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

struct Dimmer {
  byte pin;
  byte method;
  byte value; //Use setLevel(Dimmer dim, byte value). This will not update it unless you call runDimmers(true)
  byte function;
  byte data[4];
  boolean enabled;
  boolean bipolar;
  boolean inverse;
};
long nextDimmerTick;
long nextSecond;
Dimmer dimmers[50];

void setup() {

  Serial.begin(115200);
  Serial.println("Serial connected");

  dimmers[0].pin = 11;
  dimmers[0].enabled = true;
  dimmers[0].function = 1;
  dimmers[0].value = 255;
  dimmers[0].data[0] = 0;
  dimmers[0].data[1] = 1;
  dimmers[0].data[2] = 1;

  if (dimmers[0].enabled) {
    pinMode(dimmers[0].pin, OUTPUT);
  }

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), EncoderPinA, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), EncoderPinB, RISING);

  initDimmers(true);

}

void loop() {

  if (inputComplete && inputString.equals("view")) {
    Serial.println(dimmers[0].value);
    inputString = "";
    inputComplete = false;
  } else if (inputComplete) {
    dimmers[0].value = inputString.toInt();
    Serial.println("Set value to " + inputString);
    inputComplete = false;
    inputString = "";
  }

  if (encoderPos != oldEncPos) {
    Serial.print("Encoder pos: ");
    Serial.println(encoderPos);
    setLevel(&dimmers[0], dimmers[0].value + (oldEncPos - encoderPos) * 3);
    oldEncPos = encoderPos;
  }

  if (millis() >= nextDimmerTick) {
    nextDimmerTick = millis() + 50;
    runDimmers(false);
  }

  if (millis() >= nextSecond) {
    nextSecond = millis() + 1000;
  }

}

void serialEvent() {
  while (Serial.available()) {
    char c = (char) Serial.read();
    if (c != ';') {
      inputString += c;
    } else {
      inputComplete = true;
    }
  }
}

boolean pinIsValid(byte pin) {
  if (pin <= 3) {
    return false;
  } else {
    return true;
  }
}
