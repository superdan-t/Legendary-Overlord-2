// returns 0: good, returns 1: unknown cmd, returns 2: reply too large
byte interpret(byte buf[], byte replyBuf[], byte *replySize, byte maxSize) {
  replySize = 0;
  switch (buf[0]) {
    case 0:
      //Reply to a ping with acknowledge
      replyBuf[0] = 6;
      *replySize = 1;
      break;
    case 1:
      //Get system information
      replyBuf[0] = VER_MAJOR;
      replyBuf[1] = VER_MINOR;
      replyBuf[2] = VER_PATCH;
      *replySize = 3;
      break;
    case 2:
      replyBuf[0] = FastLED.getBrightness();
      *replySize = 1;
      break;
    case 3:
      //Set the brightness
      FastLED.setBrightness(buf[1]);
      break;
    case 4:
      //Kill all effect threads
      for (byte i = 0; i < 16; i++) {
        killEffectThread(i);
      }
      break;
   case 5:
      //Kill effect thread with ID
      killEffectWithID(buf[1]);
      break;
    default:
      return 1;
  }
  return 0;
}

void serialEvent() {
  while (Serial.available()) {
    byte in = Serial.read();
    if (in != EOT && in != DLE) {
      //Normal char received
      serialBuf[serialIndex] = in;
      serialIndex++;
    } else if (in == DLE && serialBuf[serialIndex] != DLE) {
      //Escape char received, not interpreted yet
      serialBuf[serialIndex] = DLE;
    } else if (serialBuf[serialIndex] == DLE) {
      //Last char was escaped. in can only equal 4 or 16 at this point
      serialBuf[serialIndex] = in;
      serialIndex++;
    } else if (in == EOT) {
      //End of transmission
      byte replyBuf[SERIAL_MAX_SIZE];
      byte replySize;
      if (interpret(serialBuf, replyBuf, &replySize, SERIAL_MAX_SIZE) == 0) {
        for (byte i = 0; i < replySize; i++) {
          if (replyBuf[i] == EOT || replyBuf[i] == DLE) {
            Serial.write(DLE);
          }
          Serial.write(replyBuf[i]);
        }
        Serial.write(EOT);
      }
      //Clear the input buffer so it doesn't interfere with future operations
      for (byte i = 0; i < SERIAL_MAX_SIZE; i++) {
        serialBuf[i] = 0;
      }
      
    }
  }
}
