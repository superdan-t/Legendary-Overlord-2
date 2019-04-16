void processData(byte *buf, char inType) {

  if (buf[0] == 0) {
    replyBuffer[0] = 6;
  } else if (buf[0] == 1) {

    if (buf[2 + buf[1]] == 0) {

      //Do I want to look at this ever again? No. Absolutely not.
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


  }

}
