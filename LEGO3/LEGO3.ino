#include <Dimmers.h>

#define VER_MAJOR 3
#define VER_MINOR 0
#define VER_PATCH 0

#define EOT 4
#define DLE 16

Dimmer dimmers[51];

byte serialBuf[50];
byte serialIndex = 0;

void setup() {

  Serial.begin(115200);

}

void loop() {

}

void serialEvent() {
  while (Serial.available()) {
    byte in = Serial.read();
    if (in != EOT && in != DLE) {
      //Normal char received
      serialBuf[serialIndex] = in;
      serialIndex++;
    } else if (in == DLE) {
      //Escape char received
      serialBuf[serialIndex] = DLE;
    } else if (serialBuf[serialIndex] == DLE) {
      //Last char was escaped. in can only equal 4 or 16 at this point
      serialBuf[serialIndex] = in;
      serialIndex++;
    } else if (in == EOT) {
      //End of transmission
      byte replyBuf[50];
      byte replySize;
      if (interpret(serialBuf, replyBuf, &replySize, 50)) {
        for (byte i = 0; i < replySize; i++) {
          if (replyBuf[i] == EOT || replyBuf[i] == DLE) {
            Serial.write(DLE);
          }
          Serial.write(replyBuf[i]);
        }
        Serial.write(EOT);
      }
      
    }
  }
}
