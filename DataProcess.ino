void processData(byte *buf, char inType) {

  if (buf[0] == 0) {
    replyBuffer[0] = 6;
    replySize = 1;
  } else if (buf[0] == 1) {

     //Do I want to look at this ever again? No. Absolutely not. And you don't either.
    if (buf[2 + buf[1]] == 0) {

      for (byte i = 0; i < buf[1]; i++) {
        if (i < buf[3 + buf[1]]) {
          setLevel(&dimmers[buf[2 + i]], buf[4 + buf[1] + i]);
        } else if (i >= buf[3 + buf[1]] && 0 != buf[3 + buf[1]]) {
          setLevel(&dimmers[buf[2 + i]], buf[3 + buf[1] + buf[3 + buf[1]]]);
        } else {
          setLevel(&dimmers[buf[2 + i]], 0);
        }
      }
    } else if (buf[2 + buf[1]] == 1) {
      /* Set dimmer data bytes. Uses adapted standard format, except each supplied "value" includes the 4 data bytes.
       *  (1 - dimmer set) (count) [Dimmers] (1 - data bytes) (count) [data 0, data 1, data 2, data 3] [data 0, data 1, ..] ...
       */
      for (byte i = 0; i < buf[1]; i++) {
        if (i < buf[3 + buf[1]]) {
          dimmers[buf[2 + i]].data[0] = buf[4 + buf[1] + i * 4];
          dimmers[buf[2 + i]].data[1] = buf[5 + buf[1] + i * 4];
          dimmers[buf[2 + i]].data[2] = buf[6 + buf[1] + i * 4];
          dimmers[buf[2 + i]].data[3] = buf[7 + buf[1] + i * 4];
        } else if (i >= buf[3 + buf[1]] && 0 != buf[3 + buf[1]]) {
          dimmers[buf[2 + i]].data[0] = buf[4 + buf[1] + (buf[3 + buf[1]] - 1) * 4];
          dimmers[buf[2 + i]].data[1] = buf[5 + buf[1] + (buf[3 + buf[1]] - 1) * 4];
          dimmers[buf[2 + i]].data[2] = buf[6 + buf[1] + (buf[3 + buf[1]] - 1) * 4];
          dimmers[buf[2 + i]].data[3] = buf[7 + buf[1] + (buf[3 + buf[1]] - 1) * 4];
        } else {
          dimmers[buf[2 + i]].data[0] = 0;
          dimmers[buf[2 + i]].data[1] = 0;
          dimmers[buf[2 + i]].data[2] = 0;
          dimmers[buf[2 + i]].data[3] = 0;
        }
      }

    } else if (buf[2 + buf[1]] == 2) {
      /*
       * Set dimmer functions. Uses standard format.
       */
      for (byte i = 0; i < buf[1]; i++) {
        if (i < buf[3 + buf[1]]) {
          dimmers[buf[2 + i]].function = buf[4 + buf[1] + i];
        } else if (i >= buf[3 + buf[1]] && 0 != buf[3 + buf[1]]) {
          dimmers[buf[2 + i]].function = buf[3 + buf[1] + buf[3 + buf[1]]];
        } else {
          dimmers[buf[2 + i]].function = 0;
        }
      }

    } else if (buf[2 + buf[1]] == 3) {
      /*
       * Set dimmer properties. Uses standard format
       */
      if (inType == 'I') {
        //For security, internet sources cannot change properties. Send back a negative acknowledge
        replyBuffer[0] = 21;
        replySize = 1;
        return;
      }
      for (byte i = 0; i < buf[1]; i++) {
        if (i < buf[4 + buf[1]]) {
          setDimmerProperty(&dimmers[buf[2 + i]], buf[3 + buf[1]], buf[5 + buf[1] + i]);
        } else if (i >= buf[4 + buf[1]] && 0 != buf[4 + buf[1]]) {
          setDimmerProperty(&dimmers[buf[2 + i]], buf[3 + buf[1]], buf[4 + buf[1] + buf[4 + buf[1]]]);
        } else {
          setDimmerProperty(&dimmers[buf[2 + i]], buf[3 + buf[1]], 0);
        }
      }
      bindAll();

    }

  } else if (buf[0] == 2) {
    /**
     * Put dimmer values in the reply buffer. This does not match the "set" format used above. It goes [value to get] (sub) [dimmers]
     */
     if (buf[1] == 0) {
      for (byte i = 0; i < buf[2]; i++) {
        replyBuffer[i] = dimmers[buf[3 + i]].value;
        replySize++;
      }
     } else if (buf[1] == 1) {
      for (byte i = 0; i < buf[2]; i++) {
        replyBuffer[i * 4] = dimmers[buf[3 + i]].data[0];
        replyBuffer[i * 4 + 1] = dimmers[buf[3 + i]].data[1];
        replyBuffer[i * 4 + 2] = dimmers[buf[3 + i]].data[2];
        replyBuffer[i * 4 + 3] = dimmers[buf[3 + i]].data[3];
        replySize += 4;
      }
     } else if (buf[1] == 2) {
      for (byte i = 0; i < buf[2]; i++) {
        replyBuffer[i] = dimmers[buf[3 + i]].function;
        replySize += 1;
      }
     } else if (buf[1] == 3) {
      for (byte i = 0; i < buf[3]; i++) {
        replyBuffer[i] = getDimmerProperty(&dimmers[buf[4 + i]], buf[2]);
      }
     }
    
    
  }

}
