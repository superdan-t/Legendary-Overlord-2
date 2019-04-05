/*
   LightingControlMaster by Daniel Tierney (https://github.com/RandomShrub)

   Thanks to Simon Merrett for interrupt-based rotary encoder setup, who also credits Oleg Mazurov, Nick Gammon, rt, Steve Spence

*/

#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define d_DimmerCount 50

#define d_Pin 0
#define d_Enabled 1
#define d_Bipolar 2
#define d_Inverse 3
#define d_Method 5

String inputString = "";
boolean inputComplete = false;

const byte encoderPinA = 2;
const byte encoderPinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
boolean screenHome;
byte homeMode;
byte timeoutDuration;
byte timeout;

struct Dimmer {
  byte pin;
  byte method;
  byte value; //Use setLevel(Dimmer dim, byte value). This will not update it unless you call runDimmers(true)
  byte function;
  byte data[4];
  boolean enabled;
  boolean bipolar;
  boolean inverse;
  boolean dirty;
};
long nextDimmerTick;
long nextSecond;
Dimmer dimmers[d_DimmerCount];

void setup() {

  Serial.begin(115200);
  Serial.println("Serial connected");

  lcd.begin(16, 2);
  lcd.print("Hello, World!");
  lcd.setCursor(0, 1);
  lcd.print("Init..");

  initDimmers();

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), EncoderPinA, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), EncoderPinB, RISING);

  screenHome = true;
  timeoutDuration = 60;

  lcd.setBacklight(0);
  lcd.noDisplay();

}

void loop() {

  if (millis() >= nextDimmerTick) {
    nextDimmerTick = millis() + 50;
    runDimmers(false);
  }

  if (millis() >= nextSecond) {
    updateHomeScreen();
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
