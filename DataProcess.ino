void processData(byte *buf, char inType) {

  if (buf[0] == 0) {
    replyBuffer[0] = 6;
    replySize = 1;
  } else if (buf[0] == 1) {

    //Do I want to look at this ever again? No. Absolutely not. And you don't either.
    if (buf[2 + buf[1]] == 0) {

      //Set the level of local dimmers

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
          (1 - dimmer set) (count) [Dimmers] (1 - data bytes) (count) [data 0, data 1, data 2, data 3] [data 0, data 1, ..] ...
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
         Set dimmer functions. Uses standard format.
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
         Set dimmer properties. Uses standard format
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
       Put dimmer values in the reply buffer. This does not match the "set" format used above. It goes [value to get] (sub) [dimmers]
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
        replySize += 1;
      }
    }
  } else if (buf[0] == 3) {

    //RESERVED FOR I2C DEVICES
    
  } else if (buf[0] == 4) {

    //The format used above is confusing and I would like to rewrite it differently, but really cannot right now.
    //The location of always-present data in the buffer (ex. sub-command) shouldn't change, while changing-length data (ex. dimmer list) should be at the end

    //Remote dimmer commands, buf[1] = address, buf[2] = sub-command, buf[3] = count of dimmers, buf[4] = count of values, then the lists. Once again, always-present required data shouldn't move.

    if (buf[2] == 0) {

      byte targetDimmers[buf[3]];
      byte targetValues[buf[4]];

      for (byte i = 0; i < buf[3]; i++) {
        targetDimmers[i] = buf[5 + i];
      }

      for (byte i = 0; i < buf[4]; i++) {
        targetValues[i] = buf[5 + buf[3] + i];
      }

      setRemoteLevels(buf[1], buf[3], targetDimmers, buf[4], targetValues);
      
    }

    
    
   
  } else if (buf[0] == 5) {

  } else if (buf[0] == 6) {

    //RESERVED FOR OTHER DEVICES. HAS NO EFFECT ON MASTER DEVICE

  } else if (buf[0] == 7) {
    //Set system variables

    if (inType == 'I') {
      replyBuffer[0] = 21;
      replySize = 1;
      return;
    }

    if (buf[1] == 0) {

      //UDP Server Port
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
  }

}
