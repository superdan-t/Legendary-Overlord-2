void EncoderPinA() {
  noInterrupts(); //stop interrupts happening before we read pin values
  if (digitalRead(encoderPinA) && digitalRead(encoderPinB) && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (digitalRead(encoderPinA)) {
    bFlag = 1; //signal that we're expecting encoderPinB to signal the transition to detent from free rotation
  }
  interrupts(); //restart interrupts
}

void EncoderPinB() {
  noInterrupts(); //stop interrupts happening before we read pin values
  if (digitalRead(encoderPinA) && digitalRead(encoderPinB) && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (digitalRead(encoderPinB)) {
    aFlag = 1; //signal that we're expecting encoderPinA to signal the transition to detent from free rotation
  }
  interrupts(); //restart interrupts
}

/**
   Should run when the device is on the home screen. If it is running elsewhere, the keypresses will cause weird results
*/
void keypadKeys() {
  if (kpd.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      if (kpd.key[i].stateChanged) {
        timeout = timeoutDuration;
        updateHomeScreen();
        switch (kpd.key[i].kstate) {
          case PRESSED:
            if (!lcdEnabled) {
              displayOn();
            }
            break;
          case RELEASED:
            onKeyPress(kpd.key[i].kchar);
            break;
          case HOLD:
            onKeyHold(kpd.key[i].kchar);
            break;
        }
      }
    }
  }
}

void onKeyPress(char key) {
  switch (key) {
    case '*':
      cmdInterface();
      updateHomeScreen();
      break;
  }
}

void onKeyHold(char key) {
  switch (key) {
    case '.':
      displayOff();
      break;
  }
}

char keyPressed(byte restriction) {
  if (kpd.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      if (kpd.key[i].stateChanged) {
        byte keyState = kpd.key[i].kstate;
        if (restriction == noRestriction)
          return kpd.key[i].kchar;
        else if (restriction == HOLD && keyState == HOLD)
          return kpd.key[i].kchar;
        else if (restriction == IDLE && keyState == IDLE)
          return kpd.key[i].kchar;
        else if (restriction == PRESSED && keyState == PRESSED)
          return kpd.key[i].kchar;
        else if (restriction == RELEASED && keyState == RELEASED)
          return kpd.key[i].kchar;
      }
    }
  }
  return ' ';
}
