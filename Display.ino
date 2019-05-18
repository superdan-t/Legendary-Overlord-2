void updateHomeScreen() {
  
  if (timeout == 0 || !screenHome) {
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
    lcd.print(now.hour() > 11 ? "PM" : "AM");
    
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
