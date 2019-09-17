byte registerEffect(EffectController *ec) {

  //Look for an empty slot
  for (unsigned int i = 0; i < 16; i++) {
    if (!(((~effectThreadStates) | 1 << i) & effectThreadStates)) {
      //Slot is empty. Mark as active
      effectThreadStates = effectThreadStates | 1 << i;

      //Copy the data from the supplied EffectController to the once associated with this effect thread
      effectThreads[i].generator = ec->generator;
      effectThreads[i].effect = ec->effect;
      effectThreads[i].threadID = ec->threadID;
      for (byte j = 0; j < 10; j++) {
        effectThreads[i].data[j] = ec->data[j];
      }

      return i; //return the slot that was occupied

    }
  }

}

void killEffectWithID(byte threadID) {
  for (byte i = 0; i < 16; i++) {
    if (effectThreads[i].threadID == threadID) {
      killEffectThread(i);
    }
  }
}

void killEffectThread(byte index) {
  //Mark as inactive. It will be overwritten when needed
  effectThreadStates = effectThreadStates ^ 1 << index;
}

void runEffectThreads() {
  for (byte i = 0; i < 16; i++) {
    //The effect will return "true" if it has finished, signaling to kill the thread
    if (((~effectThreadStates) | 1 << i) & effectThreadStates) {
    if (effectThreads[i].effect(&effectThreads[i])) {
        killEffectThread(i);
      }
    }
  }
}
