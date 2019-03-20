/*
 * LightingControlMaster by Daniel Tierney.
 * 
 * Thanks to Simon Merrett for interrupt-based rotary encoder setup, who also credits Oleg Mazurov, Nick Gammon, rt, Steve Spence
 * 
 */

String inputString = "";
boolean inputComplete = false;

static int pinA = 2;
static int pinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

struct Dimmer {
  byte pin;
  byte method;
  byte value;
  byte function;
  byte data[4];
  boolean enabled;
  boolean bipolar;
  boolean inverse;
};
long nextDimmerTick;
long nextSecond;
Dimmer dimmer[50];

void setup() {

  Serial.begin(115200);
  Serial.println("Serial connected");

  dimmer[0].pin = 11;
  dimmer[0].enabled = true;
  dimmer[0].function = 1;
  dimmer[0].value = 255;
  dimmer[0].data[0] = 0;
  dimmer[0].data[1] = 1;
  dimmer[0].data[2] = 1;

  if (dimmer[0].enabled) {
    pinMode(dimmer[0].pin, OUTPUT);
  }

  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinA), PinA, RISING);
  attachInterrupt(digitalPinToInterrupt(pinB), PinB, RISING);

}

void loop() {

  if (inputComplete && inputString.equals("view")) {
    Serial.println(dimmer[0].value);
    inputString = "";
    inputComplete = false;
  } else if (inputComplete) {
    dimmer[0].value = inputString.toInt();
    Serial.println("Set value to " + inputString);
    inputComplete = false;
    inputString = "";
  }

  if (encoderPos != oldEncPos) {
    Serial.print("Encoder pos: ");
    Serial.println(encoderPos);
    dimmer[0].value = dimmer[0].value + (oldEncPos - encoderPos) * 3;
    oldEncPos = encoderPos;
  }

  if (millis() >= nextDimmerTick) {
    nextDimmerTick = millis() + 50;
    runDimmers();
  }

  if (millis() >= nextSecond) {
    nextSecond = millis() + 1000;
  }

}

void PinA() {
  cli(); //stop interrupts happening before we read pin values
  if (digitalRead(pinA) && digitalRead(pinB) && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (digitalRead(pinA)) {
    bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  }
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  if (digitalRead(pinA) && digitalRead(pinB) && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (digitalRead(pinB)) {
    aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  }
  sei(); //restart interrupts
}

void runDimmers() {
  for (byte i = 0; i < 50; i++) {
    if (dimmer[i].enabled) {
      switch (dimmer[i].function) {
        case 1:
          if (dimmer[i].data[2] == dimmer[i].data[3]) {
            dimmer[i].data[3] = 0;

            if (abs(dimmer[i].data[0] - dimmer[i].value) < dimmer[i].data[1]) {
              dimmer[i].value = dimmer[i].data[0];
              dimmer[i].function = 0;
              dimmer[i].data[0] = 0;
              dimmer[i].data[1] = 0;
              dimmer[i].data[2] = 0;
            } else {
              if (dimmer[i].data[0] > dimmer[i].value) {
                dimmer[i].value += dimmer[i].data[1];
              } else {
                dimmer[i].value -= dimmer[i].data[1];
              }
            }

            break;
          } else {
            dimmer[i].data[3]++;
          }
      }
      byte value = dimmer[i].value;
      if (dimmer[i].inverse) {
        value = 255 - value;
      }
      if (dimmer[i].bipolar) {
        if (128 <= value) {
          value = 255;
        } else {
          value = 0;
        }
      }
      switch (dimmer[i].method) {
        case 0:
          analogWrite(dimmer[i].pin, value);
          break;
      }
    }
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
