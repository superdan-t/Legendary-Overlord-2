// returns 0: good, returns 1: unknown cmd, returns 2: reply too large
byte interpret(byte buf[], byte replyBuf[], byte *replySize, byte maxSize) {
  *replySize = 0;
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
    case 6:
      //Set a static color
      if (buf[1] <= 4) {
        statics[buf[1]][0] = buf[2];
        statics[buf[1]][1] = buf[3];
        statics[buf[1]][2] = buf[4];
      }
      break;
    case 7:
      //Set a segment of the strand to a static color (non inclusive of upper bound)
      if (buf[1] < TOTAL_LEN && buf[2] <= TOTAL_LEN) {
        for (byte i = buf[1]; i < buf[2]; i++) {
          leds[i] = CRGB(statics[buf[3]][0], statics[buf[3]][1], statics[buf[3]][2]);
        }
        FastLED.show();
      }
      break;
    case 8:
      //Set a segment of the strand using a generator
      if (buf[1] < TOTAL_LEN && buf[2] <= TOTAL_LEN) {
        for (byte i = buf[1]; i < buf[2]; i++) {
          leds[i] = getGeneratorWithID(buf[3])(buf[2] - i, buf[2] - buf[1]);
        }
        FastLED.show();
      }
      break;
    case 9:
      //Create a new effect
      EffectController newEffect;
      newEffect.effect = getEffectWithID(buf[1]);
      newEffect.generator = getGeneratorWithID(buf[2]);
      newEffect.threadID = buf[3];
      newEffect.effectStart = buf[4];
      newEffect.effectEnd = buf[5];
      for (byte i = 0; i < 8; i++) {
        newEffect.data[i] = buf[4 + i];
      }
      registerEffect(&newEffect);
      break;
    case 10:
      //Set a system variable
      switch (buf[1]) {
        case 0:
          EEPROM.update(ADDR_ETH, buf[2]);
          EEPROM.update(ADDR_ETH + 1, buf[3]);
          EEPROM.update(ADDR_ETH + 2, buf[4]);
          EEPROM.update(ADDR_ETH + 3, buf[5]);
          Ethernet.setLocalIP(IPAddress(buf[2], buf[3], buf[4], buf[5]));
      }
    default:
      return 1;
  }
  return 0;
}

void checkSerial() {
  while (Serial.available()) {
    //Serial.print("A: ");
    //Serial.println(Serial.available());
    byte in = Serial.read();
    //Serial.println(in);
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
      byte replySize = 0;
      if (interpret(serialBuf, replyBuf, &replySize, SERIAL_MAX_SIZE) == 0) {
        if (replySize > 0) {
          for (byte i = 0; i < replySize; i++) {
            if (replyBuf[i] == EOT || replyBuf[i] == DLE) {
              Serial.write(DLE);
            }
            Serial.write(replyBuf[i]);
          }
          Serial.write(EOT);
        }
      }
      //Clear the input buffer so it doesn't interfere with future operations
      serialIndex = 0;
      for (byte i = 0; i < SERIAL_MAX_SIZE; i++) {
        serialBuf[i] = 0;
      }

    }
  }
}
