void processData(byte *buf, char inType) {

  if (buf[0] == 0) {
    replyBuffer[0] = 6;
    replySize = 1;
} else if (buf[0] == 1) {

    //Set local dimmers
    //buf[1] = sub-command, buf[2] = count of dimmers, buf[3] = count of values, then lists

    for (byte i = 0; i < buf[2]; i++) {
      //Run for each dimmer
      switch (buf[1]) {
        case 0:
          //Output level
          if (buf[2] <= buf[3] || i < buf[3]) {
            setLevel(&dimmers[buf[4 + i]], buf[4 + buf[2] + i]);
          } else {
            setLevel(&dimmers[buf[4 + i]], buf[3 + buf[2] + buf[3]]);
          }
          break;
        case 1:
          //Data values
          if (buf[2] <= buf[3] || i < buf[3]) {
            dimmers[buf[4 + i]].data[0] = buf[4 + buf[2] + i * 5];
            dimmers[buf[4 + i]].data[1] = buf[5 + buf[2] + i * 5];
            dimmers[buf[4 + i]].data[2] = buf[6 + buf[2] + i * 5];
            dimmers[buf[4 + i]].data[3] = buf[7 + buf[2] + i * 5];
            dimmers[buf[4 + i]].data[4] = buf[8 + buf[2] + i * 5];
          } else if (buf[2] > buf[3] && buf[3] != 0) {
            dimmers[buf[4 + i]].data[0] = buf[4 + buf[2] + (buf[3] - 1) * 5];
            dimmers[buf[4 + i]].data[1] = buf[5 + buf[2] + (buf[3] - 1) * 5];
            dimmers[buf[4 + i]].data[2] = buf[6 + buf[2] + (buf[3] - 1) * 5];
            dimmers[buf[4 + i]].data[3] = buf[7 + buf[2] + (buf[3] - 1) * 5];
            dimmers[buf[4 + i]].data[4] = buf[8 + buf[2] + (buf[3] - 1) * 5];
          } else {
            dimmers[buf[4 + i]].data[0] = 0;
            dimmers[buf[4 + i]].data[1] = 0;
            dimmers[buf[4 + i]].data[2] = 0;
            dimmers[buf[4 + i]].data[3] = 0;
            dimmers[buf[4 + i]].data[4] = 0;
          }
          break;
        case 2:
          //Function
          if (buf[2] <= buf[3] || i < buf[3]) {
            dimmers[buf[4 + i]].function = buf[4 + buf[2] + i];
          } else {
            dimmers[buf[4 + i]].function = buf[3 + buf[2] + buf[3]];
          }
          break;
        case 3:
          //Properties. buf[2] = property, buf[3] = count of dimmers, buf[4] = count of values, lists
          if (buf[3] <= buf[4] || i < buf[4]) {
            setDimmerProperty(&dimmers[buf[5 + i]], buf[2], buf[5 + buf[3] + i]);
          } else {
            setDimmerProperty(&dimmers[buf[5 + i]], buf[2], buf[4 + buf[3] + buf[4]]);
          }
          bindAll();
          break;
      }

    }

  } else if (buf[0] == 2) {

    //Put dimmer values in the reply buffer. buf[1] = value to get, buf[2] = count of dimmers, buf[3] = property FOR GET PROPERTIES ONLY, lists...

    for (byte i = 0; i < buf[2]; i++) {
      switch (buf[1]) {
        case 0:
          replyBuffer[i] = dimmers[buf[3 + i]].value;
          replySize++;
          break;
        case 1:
          replyBuffer[i * 5 + 0] = dimmers[buf[3 + i]].data[0];
          replyBuffer[i * 5 + 1] = dimmers[buf[3 + i]].data[1];
          replyBuffer[i * 5 + 2] = dimmers[buf[3 + i]].data[2];
          replyBuffer[i * 5 + 3] = dimmers[buf[3 + i]].data[3];
          replyBuffer[i * 5 + 4] = dimmers[buf[3 + i]].data[4];
          replySize += 5;
          break;
        case 2:
          replyBuffer[i] = dimmers[buf[3 + i]].function;
          replySize++;
          break;
        case 3:
          replyBuffer[i] = getDimmerProperty(&dimmers[buf[4 + i]], buf[3]);
          replySize++;
          break;
      }
    }

  } else if (buf[0] == 3) {
    i2cAddr = buf[1];
    EEPROM.update(m_I2C_Addr, i2cAddr);
    Wire.end();
    Wire.begin(i2cAddr);
  }

}

void receiveEvent(int amount) {
  
  while (Wire.available()) {
    wireBuffer[wireIndex] = Wire.read();
    wireIndex++;
  }

  wireIndex = 0;
  
  processData(wireBuffer, 'W');
  
  //Even if there is a reply, we can't send it back until it's requested.
  if (replySize > 0) {
    for (byte i = 0; i < replySize; i++) {
      wireReply[i] = replyBuffer[i]; //Set aside the reply for later and free up the buffer
    }
    wireReplySize = replySize;
    replySize = 0; //The replyBuffer has been copied and is now free
  }
  
}

void requestEvent() {
  
  if (wireReplySize > 0) {
    for (byte i = 0; i < wireReplySize; i++) {
      Wire.write(wireReply[i]);
    }
    wireReplySize = 0;
  } else {
    //Something has to be sent back, so if there is nothing waiting to be sent, send a NAK.
    Wire.write(21);
  }
}
