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
    lcd.print(now.hour() > 12 ? now.hour() - 12 : now.hour());
    lcd.print(':');
    lcd.print(now.minute() > 9 ? now.minute() : '0' + char(now.minute()));
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

  timeout = 2 * timeoutDuration; //Interface will exit after the first countdown and return home for the next

  boolean sustain = true;
  boolean reprint = true;
  byte index = 0;

  const byte itemCount = 4;
  const char *menuItems[itemCount] = {"Dimmers", "Date/Time", "Network", "Display"};


  while (sustain && timeout > timeoutDuration) {

    if (reprint) {

      lcd.clear();
      lcd.print(menuItems[index]);

      if (index != 0) {
        lcd.setCursor(15, 0);
        lcd.print(char(0));
      }

      if (index != itemCount - 1) {
        lcd.setCursor(15, 1);
        lcd.print(char(1));
      }

      reprint = false;

    }

    char key = keyPressed(RELEASED);

    if (key != ' ') {
      timeout = 2 * timeoutDuration;
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
        const byte nItemCount = 2;
        const char *nMenuItems[] = {"IP Address:", "UDP Port:"};

        while (sustain && timeout > timeoutDuration) {

          key = keyPressed(RELEASED);
          
          if (reprint) {

            lcd.clear();
            lcd.print(nMenuItems[index]);

            lcd.setCursor(0, 1);

            lcd.print(String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]));

            if (index != 0) {
              lcd.setCursor(15, 0);
              lcd.print(char(0));
            }

            if (index != nItemCount - 1) {
              lcd.setCursor(15, 1);
              lcd.print(char(1));
            }

            reprint = false;

          }

          if (index == 0) {
            
          }

        }
      } else if (index == 3) {
        //Display

      }
    } else if (key == '.') {
      timeout = timeoutDuration;
      sustain = false;
    }


  }

}
