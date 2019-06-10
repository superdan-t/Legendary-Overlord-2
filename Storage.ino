void runScript(String scriptName) {
  //DEBUGSerial.println("Running " + scriptName);

  File script = SD.open("LEGO/scripts/" + scriptName + ".lcs", FILE_READ);

  if (!script) {
    //If the compiled script doesn't exist, check to see if there is an uncompiled version
    if (SD.exists("LEGO/scripts/" + scriptName + ".lsf")) {
      compileScript(scriptName);
      script = SD.open("LEGO/scripts/" + scriptName + ".lcs", FILE_READ);
      if (!script) return; //If it still doesn't exist, there is something wrong and we have to go back
    } else {
      return;
    }
  }

  byte lineBuffer[50];
  byte lineIndex = 0;

  while (script.available()) {

    char c = script.read();

    if (c == '\\') {
      
      //The backslash will be used as an escape character
      lineBuffer[lineIndex] = script.read();
      lineIndex++;
      
    } else if (c == '\n' || !script.available()) {
      
      //If the end-of-line or end-of-file is reached. If the value of \n is used as data, it must be escaped
      processData(lineBuffer, 'L');
      replySize = 0; //The reply is always ignored here

      for (byte i = 0; i < 50; i++) lineBuffer[i] = 0;
      lineIndex = 0;
      
    } else if (c != '\r') {
      //Everything that's not a carriage return. Ignore all carriage returns. If the value of \r (0D) is used as data, it must be escaped.

      lineBuffer[lineIndex] = c;
      lineIndex++;
      
    }
    
  }

    
}

void compileScript(String scriptName) {
  if (!sdPresent)
    return;

  if (!SD.exists("LEGO/scripts/" + scriptName + ".lsf")) {
    //DEBUGSerial.println("Script no exist");
    return;
  }

  if (SD.exists("LEGO/scripts/" + scriptName + ".lcs")) {
    //DEBUGSerial.println("Compiled script already exists. Removing old version.");
    SD.remove("LEGO/scripts/" + scriptName + ".lcs");
  }

  File uncompiledScript = SD.open("LEGO/scripts/" + scriptName + ".lsf", FILE_READ);
  File compiledScript = SD.open("LEGO/scripts/" + scriptName + ".lcs", FILE_WRITE);

  boolean compileError = false;
  char inChar;

  while (uncompiledScript.available() && !compileError) {

    //DEBUGSerial.println("More lines are left to compile. No errors have occured.");

    boolean lineEndReached = false;
    String strReadBuf = "";
    byte byteReadBuf = 0;

    while (uncompiledScript.available() && !lineEndReached) {

      //DEBUGSerial.println("Beginning new segment.");

      inChar = uncompiledScript.read();

      //If the line is/becomes a comment, ignore it.
      if (inChar == '#') {
        //DEBUGSerial.println("Line is a comment. Ignoring.");
        while (uncompiledScript.read() != '\n' && uncompiledScript.available())
          ;
        lineEndReached = true;
      }

      //If this segment begins with a number, hopefully it is really a number because we're treating it as such anyways.
      else if (isDigit(inChar)) {
        //DEBUGSerial.println("Segment is number");
        //As long as a space (end of segment) or \r (end of line) are not reached, keep reading this number
        while (inChar != ' ' && inChar != '\r') {
          strReadBuf += inChar;
          inChar = uncompiledScript.read();
          //If we happen to be at the end of the file, grab the last byte and stop the loop
          if (!uncompiledScript.available()) {
            strReadBuf += inChar;
            break;
          }
        }
        byteReadBuf = strReadBuf.toInt();
        if (byteReadBuf == 13 || byteReadBuf == 10) compiledScript.write('\\');
        compiledScript.write(byteReadBuf);
        byteReadBuf = 0;
        strReadBuf = "";
        //If it was the end of line, let the upper while loop know
        if (inChar == '\r') {
          uncompiledScript.read(); //We've detected the \r already, but still have to pass that \n
          lineEndReached = true;
        }
      }

      // The C U R L Y B O Y indicates that we have an array. Fun ensues
      else if (inChar == '{') {
        //DEBUGSerial.println("Segment is array");
        inChar = uncompiledScript.read();
        //Puts the array into the string buffer. Then moves on
        while (inChar != '}') {
          strReadBuf += inChar;
          inChar = uncompiledScript.read();
          // If the file is over or end of line is reached before the array is closed, Houston has a problem.
          if (inChar != '}' && !uncompiledScript.available() || inChar == '\r' || inChar == '\n' || inChar == ' ') {
            compileError = true;
            break;
          }
        }
        //DEBUGSerial.println("Curly boy's friend found.");
        if (compileError)
          break;
        else {
          byte segCount = countSegments(strReadBuf, ',');
          //DEBUGSerial.println(segCount);
          //DEBUGSerial.println(strReadBuf);
          if (segCount == 13 || segCount == 10) compiledScript.write('\\'); //13 is carriage return and needs to be escaped
          compiledScript.write(segCount); //Compiled array starts with how many values are in it
          //Turn the text values into unsigned chars and write them.
          for (byte i = 0; i < segCount; i++) {
            if (returnSegment(strReadBuf, i, ',').toInt() == 13 || returnSegment(strReadBuf, i, ',').toInt() == 10) compiledScript.write('\\');
            compiledScript.write(returnSegment(strReadBuf, i, ',').toInt());
            //DEBUGSerial.println(returnSegment(strReadBuf, i, ','));
          }
          strReadBuf = "";
          if (!uncompiledScript.available() || inChar == '\r') {
            uncompiledScript.read();
            lineEndReached = true;
          }
        }
      }

      //Someone included a blank line? That can't stop us!
      else if (inChar == '\r') {
        //DEBUGSerial.println("Line is blank.");
        uncompiledScript.read();
        lineEndReached = true;
      }

      //Stray space handler. Will occasionally come up even in properly formatted scripts.
      else if (inChar == ' ') {
        //DEBUGSerial.println("Stray space found. Finding home...");
      }

      //The only possiblity left is that it is a string to replace. Or there is a big problem. Either or. Hopefully the former.
      else {
        //DEBUGSerial.println("Found string to replace.");
        while (inChar != '\r' && inChar != ' ') {
          strReadBuf += inChar;
          //Check if the string matches any if these. While this is an inefficient way of doing it, this procedure has no time constraints.
          if (strReadBuf.equals("dimmer") || strReadBuf.equals("func")) {
            compiledScript.write((byte)1);
            //DEBUGSerial.println("Writing 1");
            break;
          }
          else if (strReadBuf.equals("set")) {
            compiledScript.write((byte)0);
            //DEBUGSerial.println("Writing 0");
            break;
          }
          else if (strReadBuf.equals("clr")) {
            compiledScript.write((byte)2);
            //DEBUGSerial.println("Writing 2");
            break;
          }
          else if (strReadBuf.equals("prop")) {
            compiledScript.write((byte)3);
            //DEBUGSerial.println("Writing 3");
            break;
          }
          else if (strReadBuf.equals("pause")) {
            compiledScript.write((byte)9);
          }
          else if (strReadBuf.length() > 6 || !uncompiledScript.available()) {
            //DEBUGSerial.println("Unknown operator.");
            compileError = true;
            break;
          }
          else {
            //DEBUGSerial.println("No matches. Continuing to read...");
            inChar = uncompiledScript.read();
          }
        }
        if (compileError)
          break;
        strReadBuf = "";
        if (inChar == '\r') {
          uncompiledScript.read();
          lineEndReached = true;
        }
      }
    }
    compiledScript.write('\n');
  }

  uncompiledScript.close();
  compiledScript.close();

}
