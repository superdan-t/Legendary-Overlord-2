/*

   Legendary-Overlord-2 by Daniel Tierney uses LightingControlMaster

   This edition of LEGO2 has no special addons, but it talks via serial and I2C.

   LightingControlMaster by Daniel Tierney (https://github.com/RandomShrub)

   Thanks to Simon Merrett for interrupt-based rotary encoder setup, who also credits Oleg Mazurov, Nick Gammon, rt, Steve Spence

*/

#include <EEPROM.h>
#include <Wire.h>

#define d_DimmerCount 50

#define d_Pin 0
#define d_Enabled 1
#define d_Bipolar 2
#define d_Inverse 3
#define d_Method 5

#define m_I2C_Addr 0

byte dataBuffer[50];
byte replyBuffer[50];
byte replySize = 0;
byte serialIndex = 0;
byte serialLength = 0;

byte i2cAddr = 0;
byte wireBuffer[32];
byte wireReply[32];
byte wireReplySize = 0;
byte wireIndex = 0;

struct Dimmer {
  byte pin;
  byte method;
  byte value; //Use setLevel(Dimmer dim, byte value). This will not update it unless you call runDimmers(true)
  byte function;
  byte data[5];
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

  initDimmers(true);

  i2cAddr = EEPROM.read(m_I2C_Addr);
  Wire.begin(i2cAddr);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

}

void loop() {

  if (millis() >= nextDimmerTick) {
    nextDimmerTick = millis() + 50;
    runDimmers(false);
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
