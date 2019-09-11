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
      //Get dimmer attributes: 0[get attribs] 1[to get] 2[count] 3+(dims...)
      if (buf[2] > maxSize || buf[1] == 8 && buf[2] * 5 > maxSize) {
        return 2;
      } else {
        for (byte i = 0; i < buf[2]; i++) {
          switch (buf[1]) {
            case 0:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
              break;
            case 1:
              //Pin
              replyBuf[i] = dimmers[buf[3 + i]].pin();
              *replySize++;
              break;
            case 2:
              //Method
              replyBuf[i] = dimmers[buf[3 + i]].method();
              *replySize++;
              break;
            case 3:
              //Enabled
              replyBuf[i] = dimmers[buf[3 + i]].enabled();
              *replySize++;
              break;
            case 4:
              //Bipolar
              replyBuf[i] = dimmers[buf[3 + i]].bipolar();
              *replySize++;
              break;
            case 5:
              //Inverse
              replyBuf[i] = dimmers[buf[3 + i]].inverse();
              *replySize++;
              break;
            case 6:
              //Initialized
              replyBuf[i] = dimmers[buf[3 + i]].initialized();
              *replySize++;
              break;
            case 7:
              //Effect
              replyBuf[i] = dimmers[buf[3 + i]].effect();
              *replySize++;
              break;
            case 8:
              //Data
              replyBuf[i] = dimmers[buf[3 + i]].getEffectData((byte)0); //I have no idea why this one in particular needs a cast to byte and the others don't but I'm going with it
              replyBuf[i + 1] = dimmers[buf[3 + i]].getEffectData(1);
              replyBuf[i + 2] = dimmers[buf[3 + i]].getEffectData(2);
              replyBuf[i + 3] = dimmers[buf[3 + i]].getEffectData(3);
              replyBuf[i + 4] = dimmers[buf[3 + i]].getEffectData(4);
              *replySize++;
              break;
          }
        }
      }
      break;
    default:
      return 1;

  }
}
