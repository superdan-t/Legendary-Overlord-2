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
