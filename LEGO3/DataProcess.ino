// returns 0: good, returns 1: unknown cmd, returns 2: reply too large
byte interpret(byte *buf, byte *replyBuf, byte *replySize, byte maxSize) {
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
      if (buf[2] > maxSize) {
        return 2;
      } else {
        for (byte i = 0; i < buf[2]; i++) {
          switch (buf[1]) {
            case 0:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
            case 1:
              //Pin
              replyBuf[i] = dimmers[buf[3 + i]].pin();
              *replySize++;
            case 2:
              //Method
              replyBuf[i] = dimmers[buf[3 + i]].method();
              *replySize++;
            case 3:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
            case 4:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
            case 5:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
            case 6:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
            case 7:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
            case 8:
              //Intensity
              replyBuf[i] = dimmers[buf[3 + i]].intensity();
              *replySize++;
              
          }
        }
      }
      break;


  }
}
