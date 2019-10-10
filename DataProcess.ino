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
            setLevel(&dimmers2[buf[4 + i]], buf[3 + buf[2] + buf[3]]);
          }
          break;
        case 1:
          //Data values
          if (buf[2] <= buf[3] || i < buf[3]) {
            dimmers[buf[4 + i]].data[0] = buf[4 + buf[2] + i * 5];
            dimmers[buf[4 + i]].data[1] = buf[5 + buf[2] + i * 5];
            dimmers3[buf[4 + i]].data[2] = buf[6 + buf[2] + i * 5];
            dimmers4[buf[4 + i]].data[3] = buf[7 + buf[2] + i * 5];
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

    //RESERVED FOR I2C DEVICES

  } else if (buf[0] == 4) {

    //Remote dimmer commands, buf[1] = address, buf[2] = sub-command, buf[3] = count of dimmers, buf[4] = count of values, then the lists.

    switch (buf[2]) {
      case 0:
        setRemoteLevels(buf[1], buf[3], &buf[5], buf[4], &buf[5 + buf[3]]);
        break;
      case 1:
        setRemoteData(buf[1], buf[3], &buf[5], buf[4], &buf[5 + buf[3]]);
        break;
      case 2:
        setRemoteFunctions(buf[1], buf[3], &buf[5], buf[4], &buf[5 + buf[3]]);
        break;
      case 3:
        setRemoteDimmerProperties(buf[1], buf[4], &buf[6], buf[3], buf[5], &buf[6 + buf[3]]);
        break;
    }


  } else if (buf[0] == 5) {

  } else if (buf[0] == 6) {

    //Time was requested
    sendTimeUpdate();

  } else if (buf[0] == 7) {
    //Set system variables

    if (inType == 'I') {
      replyBuffer[0] = 21;
      replySize = 1;
      return;
    }

    if (buf[1] == 0) {

      //UDP Server Port: This is fuckin broken and I didn't know it for months
      socketPort = buf[2];
      byte b = 0;
      byte c = 0;
      for (byte i = 0; i < 8; i++) {
        bitWrite(b, i, bitRead(socketPort, i + 8));
      }
      for (byte i = 0; i < 8; i++) {
        bitWrite(c, i, bitRead(socketPort, i));
      }
      EEPROM.update(m_UdpPort, b);
      EEPROM.update(m_UdpPort + 1, c);
      socket.stop();
      socket.begin(socketPort);

    } else if (buf[1] == 1) {

      //IP Address
      ip[0] = buf[2];
      ip[1] = buf[3];
      ip[2] = buf[4];
      ip[3] = buf[5];
      EEPROM.update(m_IPAddr, ip[0]);
      EEPROM.update(m_IPAddr + 1, ip[1]);
      EEPROM.update(m_IPAddr + 2, ip[2]);
      EEPROM.update(m_IPAddr + 3, ip[3]);
      Ethernet.setLocalIP(IPAddress(ip[0], ip[1], ip[2], ip[3]));
    } else if (buf[1] == 2) {

      //LCD Timeout
      timeoutDuration = buf[2];
      EEPROM.update(m_DisplayTimeout, timeoutDuration);

    } else if (buf[1] == 3) {

      //System time
      now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), buf[2], buf[3], buf[4]));

    } else if (buf[1] == 4) {
      //System date
      now = rtc.now();
      rtc.adjust(DateTime(buf[2] + 2000, buf[3], buf[4], now.hour(), now.minute(), now.second()));

    } else if (buf[1] == 5) {
      //System time and date
      rtc.adjust(DateTime(buf[2] + 2000, buf[3], buf[4], buf[5], buf[6], buf[7]));
    }

  } else if (buf[0] == 8) {
    //Get system variables
    switch (buf[1]) {
      case 0:
        replyBuffer[0] = socketPort;
        replySize = 1;
        break;
      case 1:
        replyBuffer[0] = ip[0];
        replyBuffer[1] = ip[1];
        replyBuffer[2] = ip[2];
        replyBuffer[3] = ip[3];
        replySize = 4;
        break;
      case 2:
        replyBuffer[0] = timeoutDuration;
        replySize = 1;
        break;
      case 3:
        replyBuffer[0] = now.year();
        replyBuffer[1] = now.month();
        replyBuffer[2] = now.day();
        replyBuffer[3] = now.hour();
        replyBuffer[4] = now.minute();
        replyBuffer[5] = now.second();
        replySize = 6;
        break;
    }
  } else if (buf[0] == 9) {
    //Pause for scripts only
    if (inType == 'L') {
      shortPause(buf[1] * 100);
    }

  } else if (buf[0] == 10) {
    //Get remote dimmer values
    //Put dimmer values in the reply buffer. buf[1] = address, buf[2] = value to get, buf[3] = count of dimmers, buf[4] = property FOR GET PROPERTIES ONLY, lists...
    for (byte i = 0; i < buf[3]; i++) {
      switch (buf[2]) {
        case 0:
          replyBuffer[i] = getRemoteLevel(buf[1], buf[4 + i]);
          replySize++;
          break;
        case 1:
          //Unimplemented
          break;
        case 2:
          replyBuffer[i] = getRemoteFunction(buf[1], buf[4 + i]);
          replySize++;
          break;
        case 3:
          replyBuffer[i] = getRemoteDimmerProperty(buf[1], buf[5 + i], buf[4]);
          replySize++;
          break;
      }
    }
  } else if (buf[0] == 11) {
    //Set alarm
    alarmEnabled = true;
    alarmHours = buf[1];
    alarmMinutes = buf[2];
  } else if (buf[0] == 12) {
    //Get alarm
    replyBuffer[0] = alarmEnabled;
    replyBuffer[1] = alarmHours;
    replyBuffer[2] = alarmMinutes;
    replySize = 3;
  } else if (buf[0] == 13) {
    //Disable alarm
    alarmEnabled = false;
  }

}
