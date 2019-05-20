void updateHomeScreen() {

  if (!lcdEnabled || !screenHome) {
    return;
  }


  if (homeMode == 0) {

    String tdp = String(now.month()) + '/' + String(now.day()) + '/' + String(now.year());

    lcd.clear();
    lcd.print(daysOfTheWeekShort[now.dayOfTheWeek()] + ". ");
    lcd.setCursor(16 - tdp.length(), 0);
    lcd.print(tdp);
    lcd.setCursor(0, 1);
    lcd.print(now.hour() == 0 ? 12 : now.hour() > 12 ? now.hour() - 12 : now.hour());
    lcd.print(':');
    lcd.print(now.minute() > 9 ? now.minute() : "0" + String(now.minute()));
    lcd.print(now.hour() > 11 ? " PM" : " AM");

  }

}

void displayOff() {
  lcdEnabled = false;
  lcd.setBacklight(0);
  lcd.noDisplay();
}

void displayOn() {
  lcdEnabled = true;
  lcd.setBacklight(255);
  lcd.display();
}

void cmdInterface() {

  if (!lcdEnabled) {
    //This should never happen but if it does
    displayOn();
  }

  timeout = timeoutDuration;

  boolean sustain = true;
  boolean reprint = true;
  byte index = 0;

  const byte itemCount = 4;
  const char *menuItems[itemCount] = {"Dimmers", "Date/Time", "Network", "Display"};


  while (sustain && timeout > 0) {

    if (reprint) {
      //Indicates that some display element has changed and needs to be printed

      lcd.clear();
      lcd.print('>');
      lcd.print(menuItems[index]);

      if (index != 0) {
        //Show up arrow if it isn't the first item
        lcd.setCursor(15, 0);
        lcd.print(char(0));
      }

      if (index != itemCount - 1) {
        //Show down arrow and next item if it isn't the last item
        lcd.setCursor(15, 1);
        lcd.print(char(1));

        lcd.setCursor(0, 1);
        lcd.print(menuItems[index + 1]);
      }

      reprint = false;

    }

    char key = keyPressed(RELEASED);

    if (key != ' ') {
      timeout = timeoutDuration;
    }

    if (key == 'A') {
      if (index != 0) {
        index--;
        reprint = true;
      }
    } else if (key == 'B') {
      if (index != itemCount - 1) {
        index++;
        reprint = true;
      }
    } else if (key == '#') {
      if (index == 0) {
        //Dimmers

      } else if (index == 1) {
        //Date/Time

      } else if (index == 2) {

        //Network


        reprint = true;
        index = 0;
        byte segSel = 0;
        byte segPos = 0;
        byte toSave = 0;
        boolean edit = false;
        boolean saveNext = false;
        boolean saved = true;
        String ipStr[4] = {String(ip[0]), String(ip[1]), String(ip[2]), String(ip[3])};
        String portStr = String(socketPort);
        const byte nItemCount = 2;
        const char *nMenuItems[] = {"IP Address:", "UDP Port:"};

        while (sustain && timeout > 0) {

          key = keyPressed(RELEASED);

          if (reprint) {

            lcd.clear();
            lcd.noCursor();
            lcd.noBlink();
            lcd.print(nMenuItems[index]);

            if (index != 0) {
              lcd.setCursor(15, 0);
              lcd.print(char(0));
            }

            if (index != nItemCount - 1) {
              lcd.setCursor(15, 1);
              lcd.print(char(1));
            }

            if (index == 0) {
              lcd.setCursor(0, 1);
              lcd.print(ipStr[0] + '.' + ipStr[1] + '.' + ipStr[2] + '.' + ipStr[3]);


              if (edit) {
                byte offset = 0;
                if (segSel == 1) offset = ipStr[0].length();
                else if (segSel == 2) offset = ipStr[0].length() + ipStr[1].length();
                else if (segSel == 3) offset = ipStr[0].length() + ipStr[1].length() + ipStr[2].length();

                lcd.setCursor(segSel + segPos + offset, 1);
                lcd.cursor();
                lcd.blink();
              }
            } else if (index == 1) {
              lcd.setCursor(0, 1);
              lcd.print(portStr);

              if (edit) {
                lcd.setCursor(segPos, 1);
                lcd.cursor();
                lcd.blink();
              }

            }

            reprint = false;

          }

          //Network keypress handler
          if (key == '*' || key == '#' && !edit) {
            edit = !edit;
            reprint = true;
            toSave = index;
            saveNext = true;
          } else if (key == 'A') {
            edit = false;
            segPos = 0;
            if (!saved) {
              if (lcdConfirm("Save changes?")) {
                toSave = index;
                saveNext = true;
              } else {
                saved = true;
              }
            }
            if (index != 0) {
              index--;
              reprint = true;
            }
          } else if (key == 'B') {
            edit = false;
            segPos = 0;
            if (!saved) {
              if (lcdConfirm("Save changes?")) {
                toSave = index;
                saveNext = true;
              } else {
                saved = true;
              }
            }
            if (index != itemCount - 1) {
              index++;
              reprint = true;
            }
          } else if (key == '#') {
            if (edit) {
              reprint = true;
              if (index == 0) {

                if (ipStr[segSel].toInt() > 255) ipStr[segSel] = "255";
                else if (ipStr[segSel].length() == 0) ipStr[segSel] = "0";

                segSel++;
                if (segSel >= 4) segSel = 0;

              }
            }
          } else if (key == '.') {
            if (edit) {
              saved = false;
              reprint = true;
              if (index == 0) {
                ipStr[segSel].remove(segPos, 1);
                if (segPos == ipStr[segSel].length()) segPos--;
              } else if (index == 1) {
                portStr.remove(segPos, 1);
                if (segPos == portStr.length()) segPos--;
              }
            } else {
              sustain = false;
            }
          } else if (isDigit(key)) {
            if (edit) {
              saved = false;
              reprint = true;

              if (index == 0) {

                if (ipStr[segSel].length() > segPos) ipStr[segSel][segPos] = key;
                else ipStr[segSel] += key;

                segPos++;
                if (segPos == 3) segPos = 0;

              } else if (index == 1) {

                if (portStr.length() > segPos) portStr[segPos] = key;
                else portStr += key;

                segPos++;
                if (segPos == 5) segPos = 0;

                if (portStr.toInt() > 65565) portStr = "65565";

              }
            }
          }

          if (saveNext) {
            if (toSave == 0) {

              if (ipStr[segSel].toInt() > 255) ipStr[segSel] = "255";
              else if (ipStr[segSel].length() == 0) ipStr[segSel] = "0";

              ip[0] = ipStr[0].toInt();
              ip[1] = ipStr[1].toInt();
              ip[2] = ipStr[2].toInt();
              ip[3] = ipStr[3].toInt();
              EEPROM.update(m_IPAddr, ip[0]);
              EEPROM.update(m_IPAddr + 1, ip[1]);
              EEPROM.update(m_IPAddr + 2, ip[2]);
              EEPROM.update(m_IPAddr + 3, ip[3]);
              Ethernet.setLocalIP(IPAddress(ip[0], ip[1], ip[2], ip[3]));
              saved = true;
            } else if (toSave == 1) {
              socketPort = portStr.toInt();
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
              saved = true;
            }
            saveNext = false;
          }


          if (millis() >= nextSecond) {
            nextSecond = millis() + 1000;
            timeout--;
          }


        }



      } else if (index == 3) {
        //Display

      }
    } else if (key == '.') {
      sustain = false;
    }

    if (millis() >= nextSecond) {
      nextSecond = millis() + 1000;
      timeout--;
    }


  }

  timeout = timeoutDuration;

}

/**
   An LCD prompt for changing a variable.
   valName is displayed at the top
   value should be supplied as &value so that it is actually edited
   arrows: 0 - none, 1 - down, 2 - up, 3 - up and down
   Returns an end state indicator:
*/
byte lcdEditValue(char *valName, uint16_t &value, uint16_t minValue, uint16_t maxValue, byte arrows) {

  boolean reprint = true;
  boolean editing = false;
  byte pos = 0;
  byte maxPos = String(maxValue).length() - 1;
  static byte enterState;
  String valStr = String(value); //It is needed as string, but this way, we can keep the initial value too

  timeout = timeoutDuration;

  if (enterState != 0) {
    byte b = enterState;
    enterState = 0;
    return b;
  }

  while (timeout > 0) {

    if (reprint) {

      lcd.clear();
      lcd.noBlink();
      lcd.noCursor();
      lcd.print(valName);
      lcd.print(':');

      switch (arrows) {
        case 1:
          lcd.setCursor(15, 1);
          lcd.print(char(1));
          break;
        case 2:
          lcd.setCursor(15, 0);
          lcd.print(char(0));
          break;
        case 3:
          lcd.setCursor(15, 0);
          lcd.print(char(0));
          lcd.setCursor(15, 1);
          lcd.print(char(1));
          break;
      }

      lcd.setCursor(0, 1);
      lcd.print(valStr);

      if (editing) {
        lcd.setCursor(pos, 1);
        lcd.cursor();
        lcd.blink();
      }

      reprint = false;

    }

    char key = keyPressed(RELEASED);

    if (key == '*' || key == '#' && !editing) {

      editing = !editing;
      reprint = true;
      if (!editing) {
        if (valStr.toInt() > maxValue) value = maxValue;
        else value = valStr.toInt();
        return l_save;
      }

    } else if (key == 'A' && (arrows == 2 || arrows == 3)) {

      if (editing) {
        if (lcdConfirm("Save?")) {
          enterState = l_scrollUp;
          if (valStr.toInt() > maxValue) value = maxValue;
          else value = valStr.toInt();
          return l_save;
        }
      }

      return l_scrollUp;

    } else if (key == 'B' && (arrows == 1 || arrows == 3)) {

      if (editing) {
        if (lcdConfirm("Save?")) {
          enterState = l_scrollDown;
          if (valStr.toInt() > maxValue) value = maxValue;
          else value = valStr.toInt();
          return l_save;
        }
      }

      return l_scrollDown;

    } else if (key == '#') {

      if (editing) {

        pos++;

        if (pos > maxPos) pos = 0;

      }

    } else if (key == '.') {

      if (editing) {
        if (valStr.length() != 0) valStr.remove(pos, 1);
        if (pos == valStr.length()) pos--;
      } else {
        return l_quit;
      }

    } else if (isDigit(key)) {

      if (editing) {

        if (valStr.length() > pos) valStr[pos] = key;
        else valStr += key;

        pos++;
        if (pos > maxPos) pos = 0;
      }

    }


    if (millis() >= nextSecond) {
      nextSecond = millis() + 1000;
      timeout--;
    }


  }

}

boolean lcdConfirm(char *prompt) {
  lcd.clear();
  lcd.print(prompt);
  lcd.setCursor(0, 1);
  lcd.print("(1) Yes   (2) No");
  while (true) {
    char key = keyPressed(RELEASED);
    if (key == '1') {
      return true;
    } else if (key == '2') {
      return false;
    }
  }

}
