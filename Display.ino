void updateHomeScreen() {

  if (!lcdEnabled || !screenHome) {
    return;
  }


  if (homeMode == 0) {

    now = rtc.now();

    String tdp = String(now.month()) + '/' + String(now.day()) + '/' + String(now.year());

    lcd.clear();
    lcd.print(daysOfTheWeekShort[now.dayOfTheWeek()] + ". ");
    lcd.setCursor(16 - tdp.length(), 0);
    lcd.print(tdp);
    lcd.setCursor(0, 1);


    //Format 0 is for 12-hour format. Not 0 (typically 1) is for 24 hour
    if (timeFormat == 0) {
      lcd.print(now.hour() == 0 ? 12 : now.hour() > 12 ? now.hour() - 12 : now.hour());
    } else {

      //If the hour is less than 10, then a 0 should be added before the time is printed
      if (now.hour() < 10) {
        lcd.print('0');
        lcd.print(now.hour());
      } else {
        lcd.print(now.hour());
      }

    }

    lcd.print(':');
    lcd.print(now.minute() > 9 ? now.minute() : "0" + String(now.minute()));

    if (timeFormat == 0) lcd.print(now.hour() > 11 ? " PM" : " AM");

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
    displayOn();
  }

  const byte itemCount = 4;
  const String menuItems[itemCount] = {"Dimmers", "Date/Time", "Network", "Display"};

  byte selection = lcdSelector(menuItems, itemCount);

  if (selection == 0) {

    return;

  } else if (selection == 1) {

    //Dimmers

    //First, find the dimmer to edit
    const byte dtCount = 2;
    const String dtItems[dtCount] = {"Local", "Remote"};

    byte targetDimmer = 0; //The index of the targeted dimmer, local or remote

    byte targetAddress = 0; //The I2C addr of the remote device if remote is selected

    Dimmer remoteDimmer; //Will be used as a temporary dimmer in the event that the selection is remote

    byte editType = lcdSelector(dtItems, dtCount); //Select between local or remote

    if (editType == 0) return;

    unsigned int temp = 0; //Will be used all throughout as a carrier to send non-integer data through the lcdEditValue function

    //The target dimmer has to be selected either way
    if (lcdEditValue("Dimmer", &temp, 0, 50, 0, true) == l_quit) {
      return;
    } else {
      targetDimmer = temp;
    }

    //If local, nothing needs to be done here

    if (editType == 2) {

      temp = 0;

      if (lcdEditValue("Remote Addr", &temp, 0, 127, 0, true) == l_quit) {
        return;
      } else {
        targetAddress = temp;
      }

      setDimmerProperty(&remoteDimmer, d_Pin, getRemoteDimmerProperty(targetAddress, targetDimmer, d_Pin));
      setDimmerProperty(&remoteDimmer, d_Enabled, getRemoteDimmerProperty(targetAddress, targetDimmer, d_Enabled));
      setDimmerProperty(&remoteDimmer, d_Bipolar, getRemoteDimmerProperty(targetAddress, targetDimmer, d_Bipolar));
      setDimmerProperty(&remoteDimmer, d_Inverse, getRemoteDimmerProperty(targetAddress, targetDimmer, d_Inverse));
      setDimmerProperty(&remoteDimmer, d_Method, getRemoteDimmerProperty(targetAddress, targetDimmer, d_Method));

    }

    const byte itemCount = 5;
    const String dmItems[itemCount] = {"Method", "Pin", "Enabled", "Bipolar", "Inverse"};

    boolean sustain = true;

    while (sustain) {


      byte selection = lcdSelector(dmItems, itemCount);

      if (selection == 0) {

        sustain = false;

      } else if (selection == 1 || selection == 2) {

        temp = getDimmerProperty(editType == 1 ? &dimmers[targetDimmer] : &remoteDimmer, selection == 1 ? d_Method : d_Pin); //The initial value that needs editing

        while (sustain == true) {

          //Launch a value editor for method or pin. Second ternary determines max value (15 for Method or 255 for Pin), while the third determines arrow state (down only for Method, both for Pin)
          byte exitState = lcdEditValue(selection == 1 ? "Method" : "Pin", &temp, 0, selection == 1 ? 15 : 255, selection == 1 ? 1 : 3);

          switch (exitState) {

            case l_save:
              setDimmerProperty(editType == 1 ? &dimmers[targetDimmer] : &remoteDimmer, selection == 1 ? d_Method : d_Pin, temp);
              if (editType == 1) {
                bindAll();

              } else {

                setRemoteDimmerProperty(targetAddress, targetDimmer, selection == 1 ? d_Method : d_Pin, temp);

              }
              break;

            case l_quit:
              sustain = false;
              break;

          }

        }

        sustain = true; //Set it back to true so the previous menu will show again

      } else {

        boolean prop = getDimmerProperty(editType == 1 ? &dimmers[targetDimmer] : &remoteDimmer, selection - 2);

        lcdEditBoolean(selection == 3 ? "Enabled" : (selection == 4 ? "Bipolar" : "Inverse"), &prop);

        setDimmerProperty(editType == 1 ? &dimmers[targetDimmer] : &remoteDimmer, selection - 2, prop);

        if (editType == 1) {
          bindAll();
        } else {
          setRemoteDimmerProperty(targetAddress, targetDimmer, selection - 2, prop);
        }

      }



    }

  } else if (selection == 2) {

    //Date/Time
    const char *dtmItems[] = {"Year", "Month", "Day", "Hour", "Minute", "Second"};
    const unsigned int dtmMaxVal[] = {2100, 12, 31, 23, 59, 59};
    const byte dtmItemsCount = 6;

    unsigned int val = 0;

    byte index = 0;

    boolean sustain = true;

    while (sustain) {

      now = rtc.now();

      switch (index) {
        case 0:
          val = now.year();
          break;
        case 1:
          val = now.month();
          break;
        case 2:
          val = now.day();
          break;
        case 3:
          val = now.hour();
          break;
        case 4:
          val = now.minute();
          break;
        case 5:
          val = now.second();
          break;
      }

      byte exitState = lcdEditValue(dtmItems[index], &val, 0, dtmMaxVal[index], index == 0 ? 1 : (index == 5 ? 2 : 3));

      switch (exitState) {
        case l_save:
          //This whole block here is stupid redundant but I couldn't find any function to adjust individual values of a DateTime so this is the best I could manage
          now = rtc.now();
          switch (index) {
            case 0:
              rtc.adjust(DateTime(val, now.month(), now.day(), now.hour(), now.minute(), now.second()));
              break;
            case 1:
              rtc.adjust(DateTime(now.year(), val, now.day(), now.hour(), now.minute(), now.second()));
              break;
            case 2:
              rtc.adjust(DateTime(now.year(), now.month(), val, now.hour(), now.minute(), now.second()));
              break;
            case 3:
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), val, now.minute(), now.second()));
              break;
            case 4:
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), val, now.second()));
              break;
            case 5:
              rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), val));
              break;
          }
          break;

        case l_scrollUp:
          index--;
          break;
        case l_scrollDown:
          index++;
          break;
        case l_quit:
          sustain = false;

      }

    }


  } else if (selection == 3) {

    //Network. This is a disaster but it would be hard to update it to use new procedures because of the IP address being 4 segments.

    boolean reprint = true;
    byte index = 0;
    byte segSel = 0;
    byte segPos = 0;
    byte toSave = 0;
    boolean edit = false;
    boolean saveNext = false;
    boolean saved = true;
    boolean sustain = true;
    String ipStr[4] = {String(ip[0]), String(ip[1]), String(ip[2]), String(ip[3])};
    String portStr = String(socketPort);
    const byte nItemCount = 2;
    const char *nMenuItems[] = {"IP Address:", "UDP Port:"};

    while (sustain && timeout > 0) {

      char key = keyPressed(RELEASED);

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
      if (key == b_CMD || key == b_GO && !edit) {
        edit = !edit;
        reprint = true;
        toSave = index;
        saveNext = true;
      } else if (key == b_Up) {
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
      } else if (key == b_Down) {
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
      } else if (key == b_GO) {
        if (edit) {
          reprint = true;
          if (index == 0) {

            if (ipStr[segSel].toInt() > 255) ipStr[segSel] = "255";
            else if (ipStr[segSel].length() == 0) ipStr[segSel] = "0";

            segSel++;
            if (segSel >= 4) segSel = 0;

          }
        }
      } else if (key == b_DEL) {
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

  } else if (selection == 4) {

    //Display
    byte index = 0;
    unsigned int val;
    boolean sustain = true;

    while (sustain) {

      if (index == 0) {

        val = timeoutDuration;

        byte exitState = lcdEditValue("Timeout", &val, 10, 255, 1);
        switch (exitState) {
          case l_scrollDown:
            index++;
            break;
          case l_save:
            timeoutDuration = val;
            EEPROM.update(m_DisplayTimeout, timeoutDuration);
            break;
          case l_quit:
            sustain = false;
            break;
        }
      } else if (index == 1) {

        val = i2cDisplay;

        byte exitState = lcdEditValue("I2C Disp Addr", &val, 10, 255, 3);

        switch (exitState) {
          case l_scrollUp:
            index--;
            break;
          case l_scrollDown:
            index++;
            break;
          case l_save:
            i2cDisplay = val;
            EEPROM.update(m_I2CDisplay, i2cDisplay);
            break;
          case l_quit:
            sustain = false;
            break;
        }

      } else if (index == 2) {

        boolean reprint = true;

        while (sustain) {

          if (reprint) {
            lcd.clear();
            lcd.print("Time Format:");
            lcd.setCursor(15, 0);
            lcd.print(char(0));
            lcd.setCursor(0, 1);
            lcd.print(timeFormat == 0 ? "12-hour" : "24-hour");
            reprint = false;
          }


          char key = keyPressed(RELEASED);

          if (key == b_Up) {
            index--;
            sustain = false;
          } else if (key  == b_DEL) {
            sustain = false;
          } else if (key == b_GO) {

            const String opt[] = {"12-hour", "24-hour"};

            byte selection = lcdSelector(opt, 2);

            if (selection != 0) {
              timeFormat = selection - 1;
              EEPROM.update(m_TimeFormat, timeFormat);
              reprint = true;
            } else {
              sustain = false;
            }
          }
        }

        sustain = true;

      }

    }

  }



}

byte lcdEditValue(char *valName, unsigned int *value, unsigned int minValue, unsigned int maxValue, byte arrows) {
  return lcdEditValue(valName, value, minValue, maxValue, arrows, false);
}

/**
   An LCD prompt for changing a variable.
   valName is displayed at the top
   value should be supplied as &value so that it is actually edited
   arrows: 0 - none, 1 - down, 2 - up, 3 - up and down
   Returns an end state indicator:
*/
byte lcdEditValue(char *valName, unsigned int *value, unsigned int minValue, unsigned int maxValue, byte arrows, boolean startActive) {

  boolean reprint = true;
  boolean editing = startActive;
  byte pos = 0;
  byte maxPos = String(maxValue).length() - 1;
  static byte enterState;
  String valStr = String(*value); //It is needed as string, but this way, we can keep the initial value too

  timeout = timeoutDuration;

  if (enterState != 0) {
    byte b = enterState;
    enterState = 0;
    lcd.noCursor();
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

    if (key == b_CMD || key == b_GO && !editing) {

      editing = !editing;
      reprint = true;
      if (!editing) {
        if (valStr.toInt() > maxValue) *value = maxValue;
        else if (valStr.toInt() < minValue) *value = minValue;
        else *value = valStr.toInt();
        lcd.noBlink();
        lcd.noCursor();
        return l_save;
      }

    } else if (key == b_Up && (arrows == 2 || arrows == 3)) {

      if (editing) {
        if (lcdConfirm("Save?")) {
          enterState = l_scrollUp;
          if (valStr.toInt() > maxValue) *value = maxValue;
          else if (valStr.toInt() < minValue) *value = minValue;
          else *value = valStr.toInt();
          lcd.noBlink();
          lcd.noCursor();
          return l_save;
        }
      }

      return l_scrollUp;

    } else if (key == b_Down && (arrows == 1 || arrows == 3)) {

      if (editing) {
        if (lcdConfirm("Save?")) {
          enterState = l_scrollDown;
          if (valStr.toInt() > maxValue) *value = maxValue;
          else if (valStr.toInt() < minValue) *value = minValue;
          else *value = valStr.toInt();
          lcd.noBlink();
          lcd.noCursor();
          return l_save;
        }
      }

      return l_scrollDown;

    } else if (key == b_GO) {

      if (editing) {

        reprint = true;

        pos++;

        if (pos > maxPos) pos = 0;

      }

    } else if (key == b_DEL) {

      if (editing) {
        reprint = true;
        if (valStr.length() != 0) valStr.remove(pos, 1);
        if (pos == valStr.length()) pos--;
      } else {
        lcd.noBlink();
        lcd.noCursor();
        return l_quit;
      }

    } else if (isDigit(key)) {

      if (editing) {

        reprint = true;

        if (valStr.length() == 0) {
          valStr += key;
          pos++;
        }
        else if (valStr.length() > pos) valStr[pos] = key;
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

  lcd.noBlink();
  lcd.noCursor();
  return l_quit;

}
/**
   Edit a boolean value. Since this doesn't support scrolling, there is no return value.
*/
void lcdEditBoolean(char *valName, boolean *value) {

  const String opt[] = {"False", "True"};

  if (*value != 0 && *value != 1) *value = true; //Make sure that it is a 0 or 1 for sure.

  byte selection = lcdSelector(opt, 2, *value);

  if (selection != 0) {
    *value = selection - 1;
  }

}

/**
   See docstring below. This lacks 1 param
*/
byte lcdSelector(String *items, byte itemCount) {
  return lcdSelector(items, itemCount, 0);
}

/**
   Makes a menu out of items.
   Returns selection + 1!!! This is so that 0 can be returned if the user exits the menu.
*/
byte lcdSelector(String *items, byte itemCount, byte startIndex) {

  byte index = startIndex;
  boolean reprint = true;

  timeout = timeoutDuration;

  while (timeout > 0) {

    if (reprint) {

      lcd.clear();
      lcd.print('>');
      lcd.print(items[index]);

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
        lcd.print(items[index + 1]);
      }

      reprint = false;


    }

    char key = keyPressed(RELEASED);

    switch (key) {
      case b_DEL:
        return 0;
      case b_Up:
        if (index != 0) {
          index--;
          reprint = true;
        }
        break;
      case b_Down:
        if (index != itemCount - 1) {
          index++;
          reprint = true;
        }
        break;
      case b_GO:
        return index + 1;
    }

    if (millis() >= nextSecond) {
      nextSecond = millis() + 1000;
      timeout--;
    }

  }


  return 0;


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
