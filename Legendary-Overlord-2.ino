/*

   Legendary-Overlord-2 by Daniel Tierney uses LightingControlMaster

   LightingControlMaster by Daniel Tierney (https://github.com/RandomShrub)

   Thanks to Simon Merrett for interrupt-based rotary encoder setup, who also credits Oleg Mazurov, Nick Gammon, rt, Steve Spence

*/

#include <EEPROM.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal_I2C.h>

#define d_DimmerCount 50

#define d_Pin 0
#define d_Enabled 1
#define d_Bipolar 2
#define d_Inverse 3
#define d_Method 5

#define m_UdpPort 0
#define m_IPAddr 1
#define m_DisplayTimeout 5

const byte encoderPinA = 2;
const byte encoderPinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

const byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x91, 0x39};
byte ip[] = {192, 168, 1, 167};
byte socketPort = 0;
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];
EthernetUDP socket;

byte dataBuffer[50];
byte replyBuffer[50];
byte replySize = 0;
byte serialIndex = 0;
byte serialLength = 0;

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

  lcd.begin(16, 2);
  lcd.print("Hello, World!");
  lcd.setCursor(0, 1);
  lcd.print("Init..");

  initDimmers(true);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), EncoderPinA, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), EncoderPinB, RISING);

  screenHome = true;
  timeoutDuration = EEPROM.read(m_DisplayTimeout);
  socketPort = EEPROM.read(m_UdpPort);
  ip[0] = EEPROM.read(m_IPAddr);
  ip[1] = EEPROM.read(m_IPAddr + 1);
  ip[2] = EEPROM.read(m_IPAddr + 2);
  ip[3] = EEPROM.read(m_IPAddr + 3);

  beginNetwork();

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
    byte newData = Serial.read();
    if (0 == serialLength) {
      serialLength = newData;
    } else {
      dataBuffer[serialIndex++] = newData;
      if (serialIndex >= serialLength) {
        processData(dataBuffer, 'S');
        serialIndex = 0;
        serialLength = 0;
        if (replySize > 0) {
          Serial.write(replySize);
          for (byte i = 0; i < replySize; i++) {
            Serial.write(replyBuffer[i]);
          }
          replySize = 0; //Signals that replyBuffer isn't being used and can be overwritten. No need to clear the whole thing.
        }
      }
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
