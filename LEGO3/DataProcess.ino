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
      FastLED.show();
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
        newEffect.data[i] = buf[6 + i];
      }
      registerEffect(&newEffect);
      break;
    case 10:
      //Set a system variable
      switch (buf[1]) {
        case 0:
          //IP Address
          EEPROM.update(ADDR_ETH, buf[2]);
          EEPROM.update(ADDR_ETH + 1, buf[3]);
          EEPROM.update(ADDR_ETH + 2, buf[4]);
          EEPROM.update(ADDR_ETH + 3, buf[5]);
          Ethernet.setLocalIP(IPAddress(buf[2], buf[3], buf[4], buf[5]));
          break;
        case 1:
          //UDP port
          EEPROM.update(ADDR_UDP, buf[2]);
          EEPROM.update(ADDR_UDP + 1, buf[3]);
          socket.stop();
          socket.begin(EEPROM.read(ADDR_UDP) * 256 + EEPROM.read(ADDR_UDP + 1));
          break;
      }
      break;
    case 11:
      //Get up time
      replySize = 1;
      replyBuf[0] = millis();
      break;
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
      replySize = 0;
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

void checkSocket() {
  if (socket.parsePacket() == 0) {
    return;
  }
  socket.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

  replySize = 0;
  if (interpret(packetBuffer, replyBuf, &replySize, SERIAL_MAX_SIZE) == 0) {
    if (replySize > 0) {
      socket.beginPacket(socket.remoteIP(), socket.remotePort());
      for (byte i = 0; i < replySize; i++) {
        if (replyBuf[i] == EOT || replyBuf[i] == DLE) {
          socket.write(DLE);
        }
        socket.write(replyBuf[i]);
      }
      socket.write(EOT);
      socket.endPacket();
    }
  }

}

void checkServer() {

  char inString[100];
  byte currentIndex = 0;
  //String queryString = "";
  char getIndex = -1;
  byte getLength;

  EthernetClient client = webserver.available();

  if (!client) return;

  //Log this later to improve the response time
  byte rIP[] = {0, 0, 0, 0};
  client.getRemoteIP(rIP);

  while (client.connected()) {

    if (client.available()) {

      char c = client.read();
      inString[++currentIndex] = c;

      if (c == '\n') {

        for (byte i = 0; i < currentIndex; i++) {

          //Look for the start of the GET request
          if (inString[i] == 'G' && inString[i + 1] == 'E' && inString[i + 2] == 'T' && inString[i + 3] == ' ' && inString[i + 4] == '/') {
            i += 5; //Jump forward to the actual request
            getIndex = i;
          }

          if (getIndex != -1 && inString[i] != ' ') {
            getLength++;
          } else if (getIndex != -1 && inString[i] == ' ') {
            break;
          }
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println();
        client.println("<!DOCTYPE HTML>");

        //Check for a query string
        char queryStart = indexInString(&inString[getIndex], getLength, '?'); //get.indexOf('?'); Old line when using Strings
        char queryLength = -1;
        if (queryStart != -1) {
          queryLength = getLength - queryStart;
          getLength = queryStart; //We don't want the query string in the get request
        }
//        if (queryStart != -1) {
//          for (byte i = queryStart + 1; i < get.length(); i++) {
//            queryString += get.charAt(i);
//          }
//          get.remove(queryStart); //Clear everything else from the GET request now that the query has been isolated.
//
//        }

         if (getLength == 0) {


          //Home page
          File page = SD.open("LEGO3/html/index.htm", FILE_READ);

          if (page) {

            //Print the header
            File header = SD.open("LEGO3/html/header.htm", FILE_READ);
            while (header.available()) {
              client.write(header.read());
            }
            header.close();

            //Print the actual page
            while (page.available()) {
              client.write(page.read());
            }
            page.close();

            //Print the footer
            File footer = SD.open("LEGO3/html/footer.htm", FILE_READ);
            while (footer.available()) {
              client.write(footer.read());
            }
            footer.close();

          }

        } else if (stringsEqual(inString, getLength, "favicon.ico", 11) {

          //Ignore it

        } else if (stringsEqual(inString, getLength, "c", 1) {

          //Process data and send response, down for now
//          String response = processQuery(queryString);
//          if (response != "") {
//            client.print(response);
//          }
//          queryString = "";

        } else if (stringsEqual(inString, getLength, "f", 1) {

          //Do nothing. Useful for running scripts because they will still run, but the connection will close first

        } else {
          //If it was nothing else, search for a page

          File page = SD.open("LEGO3/html/" + get, FILE_READ);

          if (!page) {
            page = SD.open("LEGO3/html/404.htm", FILE_READ);
            if (!page) {
              client.println("<html>");
              client.print("500 Error: The server has ran into a problem.");
              client.println("</html>");
            }
          }

          if (page) {

            //Print the header
            File header = SD.open("LEGO3/html/header.htm", FILE_READ);
            while (header.available()) {
              client.write(header.read());
            }
            header.close();

            //Print the actual page
            while (page.available()) {
              client.write(page.read());
            }
            page.close();

            //Print the footer
            File footer = SD.open("LEGO3/html/footer.htm", FILE_READ);
            while (footer.available()) {
              client.write(footer.read());
            }
            footer.close();

          }

        }


        delay(1);
        client.stop();

        if (queryLength != 0) {
          interpret(&inString[queryStart], 
        }
        
        if (queryString.length() != 0) {
          processQuery(queryString);
        }

//        File log = SD.open("LEGO3/html/traffic.htm", FILE_WRITE);
//
//        log.print(now.month());
//        log.print('/');
//        log.print(now.day());
//        log.print('/');
//        log.print(now.year());
//        log.print(' ');
//        log.print(now.hour());
//        log.print(':');
//        if (now.minute() < 10) log.print('0');
//        log.print(now.minute());
//        log.print(" -> ");
//
//        log.print(rIP[0]);
//        log.print('.');
//        log.print(rIP[1]);
//        log.print('.');
//        log.print(rIP[2]);
//        log.print('.');
//        log.print(rIP[3]);
//
//        log.print(" requested ");
//        log.print(get + (queryString.length() != 0 ? ('?' + queryString) : ""));
//        log.println("<br>");
//
//        log.close();

        inString = "";
        queryString = "";
        get = "";

      }

    }
  }

}

String processQuery(String query) {
  String reply = "";
  for (byte i = 0; i < countSegments(query, '&'); i++) {

    String var = returnSegment(returnSegment(query, i, '&'), 0, '=');
    String val = returnSegment(returnSegment(query, i, '&'), 1, '=');

    if (var == "script") {
      runScript(val);
    } else if (var == "msg") {
      val.replace("+", " ");
      val.replace("%20", " ");
      val.replace("%2C", ",");
      val.replace("%21", "!");
      val.replace("%40", "@");
      val.replace("%23", "#");
      val.replace("%24", "$");
      val.replace("%25", "%");
      val.replace("%5E", "^");
      val.replace("%26", "&");
      val.replace("%28", "(");
      val.replace("%29", ")");
      val.replace("%3D", "=");
      val.replace("%3A", ":");
      val.replace("%3B", ";");
      val.replace("%2F", "/");
      val.replace("%5C", "\\");
      val.replace("%3F", "?");
      val.replace("%2B", "+");
      val.replace("%27", "'");
      makeNotification(val);
    } else if (var == "recomp") {

      compileScript(val);

    } else if (var == "data") {

      byte webBuffer[val.length() / 2];
      for (byte i = 0; i < val.length(); i += 2) {
        webBuffer[i / 2] = 16 * ((val[i] < '9') ? val[i] - '0' : val[i] - '7') + ((val[i + 1] < '9') ? val[i + 1] - '0' : val[i + 1] - '7');
      }
      processData(webBuffer, 'I');
      if (replySize != 0) {
        for (byte i = 0; i < replySize; i++) {
          if (replyBuffer[i] < 16) reply += '0';
          reply += String(replyBuffer[i], HEX);
        }
        replySize = 0;  //Done with replyBuffer for now
        reply += '\n'; //Add a new line in case there is more data
      }
    }

  }

  return reply;
}
