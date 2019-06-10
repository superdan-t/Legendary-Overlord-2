void beginNetwork() {
  Ethernet.begin(mac, IPAddress(ip[0], ip[1], ip[2], ip[3]));
  webserver.begin();
  socket.begin(socketPort);
}

void checkSocket() {

  if (socket.parsePacket() == 0) {
    return;
  }

  socket.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

  processData(packetBuffer, 'I');


  socket.beginPacket(socket.remoteIP(), socket.remotePort());

  if (replySize != 0) {
    socket.write(replyBuffer, replySize);
    replySize = 0;
  } else {
    socket.write(6);
  }

  socket.endPacket();

}

void runServer() {

  String inString = "";
  String queryString = "";
  String get = "";

  EthernetClient client = webserver.available();

  if (!client) return;

  while (client.connected()) {

    if (client.available()) {

      char c = client.read();
      inString += c;

      if (c == '\n') {

        boolean getFound = false;
        for (byte i = 0; i < inString.length(); i++) {

          //Look for the start of the GET request
          if (inString[i] == 'G' && inString[i + 1] == 'E' && inString[i + 2] == 'T' && inString[i + 3] == ' ' && inString[i + 4] == '/') {
            getFound = true;
            i += 5; //Jump forward to the actual request
          }

          if (getFound && inString[i] != ' ') {
            get += inString[i];
          } else if (getFound && inString[i] == ' ') {
            getFound = false;
            break;
          }
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println();
        client.println("<!DOCTYPE HTML>");

        //Check for a query string
        int8_t queryStart = get.indexOf('?');
        if (queryStart != -1) {
          for (byte i = queryStart + 1; i < get.length(); i++) {
            queryString += get.charAt(i);
          }
          get.remove(queryStart); //Clear everything else from the GET request now that the query has been isolated.

        }

        if (get == "favicon.ico") {

          //Ignore this one
          client.println("no");

        } else if (get == "c") {

          //Process data and send response
          String response = processQuery(queryString);
          if (response != "\0") {
            client.print(response);
          }
          queryString = "";

        } else if (get == "f") {
          
          //Do nothing. Useful for running scripts because they will still run, but the connection will close first
          
        } else if (get == "") {


          //Home page
          File page = SD.open("LEGO/html/home.htm", FILE_READ);

          if (!page) {
            page = SD.open("LEGO/html/404.htm", FILE_READ);
            if (!page) {
              client.println("<html>");
              client.println("404 Error: The 404 error page was not found either so something is very wrong");
              client.println("</html>");
            }
          }

          if (page) {
            while (page.available()) {
              client.write(page.read());
            }
            page.close();
          }

        } else {
          //If it was nothing else, search for a page

          File page = SD.open("LEGO/html/" + get, FILE_READ);

          if (!page) {
            page = SD.open("LEGO/html/404.htm", FILE_READ);
            if (!page) {
              client.println("<html>");
              client.println("404 Error: The 404 error page was not found either so something is very wrong");
              client.println("</html>");
            }
          }

          if (page) {
            while (page.available()) {
              client.write(page.read());
            }
            page.close();
          }

        }

        inString = "";
        get = "";
        get = "";
        delay(1);
        client.stop();
        if (queryString.length() != 0) {
          processQuery(queryString);
        }
      }

    }
  }

}

String processQuery(String query) {
  for (byte i = 0; i < countSegments(query, '&'); i++) {

    String var = returnSegment(returnSegment(query, i, '&'), 0, '=');
    String val = returnSegment(returnSegment(query, i, '&'), 1, '=');

    if (var == "script") {
      runScript(val);
    } else if (var == "recomp") {

      compileScript(val);

    } else if (var == "data") {

      byte webBuffer[val.length() / 2];
      for (byte i = 0; i < val.length(); i += 2) {
        webBuffer[i / 2] = 16 * ((val[i] < '9') ? val[i] - '0' : val[i] - '7') + ((val[i + 1] < '9') ? val[i + 1] - '0' : val[i + 1] - '7');
      }
      processData(webBuffer, 'I');
      if (replySize == 0) {
        return "\0";
      } else {
        String reply;
        for (byte i = 0; i < replySize; i++) {
          if (replyBuffer[i] < 16) reply += '0';
          reply += String(replyBuffer[i], HEX);
        }
        replySize = 0;  //Done with replyBuffer
        return reply;
      }
    }

  }

  return "\0";
}
