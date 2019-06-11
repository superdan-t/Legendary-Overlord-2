void runScript(String scriptName) {
  //DEBUG//DEBUG/Serial.println("Running " + scriptName);

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

    } else {

      lineBuffer[lineIndex] = c;
      lineIndex++;

    }

  }


}

void compileScript(String scriptName) {

  File uncompiledScript = SD.open("LEGO/scripts/" + scriptName + ".lsf", FILE_READ);
  if (!uncompiledScript) {
    return;
  }

  if (SD.exists("LEGO/scripts/" + scriptName + ".lcs")) {
    SD.remove("LEGO/scripts/" + scriptName + ".lcs");
  }
  File compiledScript = SD.open("LEGO/scripts/" + scriptName + ".lcs", FILE_WRITE);

  boolean compileError = false;
  boolean lineEndReached = false;
  char c;
  String currentLine = "";

  while (uncompiledScript.available()) {

    while (uncompiledScript.available() && !lineEndReached) {
      c = uncompiledScript.read();
      if (c == '\n') {
        lineEndReached = true;
      } else if (c != '\r') {
        currentLine += c;
      }
    }

    //DEBUG/Serial.println("Reached the end of a line. The line found is:");
    //DEBUG/Serial.println(currentLine);

    byte temp = 0;

    currentLine.toLowerCase();

    //These keywords are used to make writing scripts easier but can be substituted with numbers in place
    currentLine.replace("true", "1");
    currentLine.replace("false", "0");
    currentLine.replace("none", "0");
    currentLine.replace("fade", "1");
    currentLine.replace("strobe", "2");
    currentLine.replace("sequence", "3");
    currentLine.replace("flicker", "4");

    //DEBUG/Serial.println("Finished replacing keywords.");

    //The "command" should be first
    String currentSegment = returnSegment(currentLine, 0, ' ');

    //DEBUG/Serial.println("The commmand segment is " + currentSegment);

    if (currentSegment == "dimmer" || currentSegment == "dimmers" || currentSegment == "remotedimmer" || currentSegment == "remotedimmers") {
      if (currentSegment == "dimmer" || currentSegment == "dimmers") {
        compiledScript.write((byte)1);
      } else {
        compiledScript.write((byte)4);
        temp = returnSegment(currentLine, 1, ' ').toInt();
        if (temp == '\n' || temp == '\\') compiledScript.write('\\');
        compiledScript.write(temp);
        currentLine.remove(0, currentLine.indexOf(' ') + 1);
      }

      byte offset = 0;

      //The "sub-command" is 3rd when human-readable, but 2nd in binary, so skip forward
      currentSegment = returnSegment(currentLine, 2, ' ');
      //DEBUG/Serial.println("The sub-command is " + currentSegment);
      if (currentSegment == "level") {
        compiledScript.write((byte)0);
      } else if (currentSegment == "data") {
        compiledScript.write((byte)1);
      } else if (currentSegment == "function") {
        compiledScript.write((byte)2);
      } else if (currentSegment == "property") {
        compiledScript.write((byte)3);
        offset = 1;
        currentSegment = returnSegment(currentLine, 3, ' ');
        if (currentSegment == "pin") {
          compiledScript.write((byte)0);
        } else if (currentSegment == "enabled") {
          compiledScript.write((byte)1);
        } else if (currentSegment == "bipolar") {
          compiledScript.write((byte)2);
        } else if (currentSegment == "inverse") {
          compiledScript.write((byte)3);
        } else if (currentSegment == "method") {
          compiledScript.write((byte)5);
        } else {
          compileError = true;
        }
      }
      
      //DEBUG/Serial.println("Writing lists portion.");

      //Next is how many dimmers are supplied
      currentSegment = returnSegment(currentLine, 1, ' ');
      temp = countSegments(currentSegment, ',');
      //DEBUG/Serial.println(String(temp) + " dimmers supplied.");
      if (temp == '\n' || temp == '\\') compiledScript.write('\\');
      compiledScript.write(temp);

      //How many values
      currentSegment = returnSegment(currentLine, 3 + offset, ' ');
      temp = countSegments(currentSegment, ',');
      //DEBUG/Serial.println(String(temp) + " values supplied.");
      if (temp == '\n' || temp == '\\') compiledScript.write('\\');
      compiledScript.write(temp);

      //Now actually write the dimmers
      currentSegment = returnSegment(currentLine, 1, ' ');
      for (byte i = 0; i < countSegments(currentSegment, ','); i++) {
        temp = returnSegment(currentSegment, i, ',').toInt();
        //DEBUG/Serial.println("This dimmer is " + String(temp));
        if (temp == '\n' || temp == '\\') compiledScript.write('\\');
        compiledScript.write(temp);
      }

      //Write the data
      currentSegment = returnSegment(currentLine, 3 + offset, ' ');
      for (byte i = 0; i < countSegments(currentSegment, ','); i++) {
        for (byte j = 0; j < countSegments(returnSegment(currentSegment, i, ','), '|'); j++) {
          temp = returnSegment(returnSegment(currentSegment, i, ','), j, '|').toInt();
          //DEBUG/Serial.println("This value is " + String(temp));
          if (temp == '\n' || temp == '\\') compiledScript.write('\\');
          compiledScript.write(temp);
        }
      }

      //DEBUG/Serial.println("Printed dimmers and values.");

    } else if (currentSegment == "pause") {
      compiledScript.write((byte)9);
      temp = returnSegment(currentLine, 1, ' ').toInt();
      if (temp == '\n' || temp == '\\') compiledScript.write('\\');
      compiledScript.write(temp);
    } else {
      compileError = true;
    }

    //Mark the line ending
    compiledScript.write('\n');

    //DEBUG/Serial.println("Finished this line. Moving on.");

    currentLine = "";
    lineEndReached = false;
    
  }

  compiledScript.close();
  uncompiledScript.close();
  
}

//void compileScriptOld(String scriptName) {
//  if (!sdPresent)
//    return;
//
//  if (!SD.exists("LEGO/scripts/" + scriptName + ".lsf")) {
//    //DEBUG//DEBUG/Serial.println("Script no exist");
//    return;
//  }
//
//  if (SD.exists("LEGO/scripts/" + scriptName + ".lcs")) {
//    //DEBUG//DEBUG/Serial.println("Compiled script already exists. Removing old version.");
//    SD.remove("LEGO/scripts/" + scriptName + ".lcs");
//  }
//
//  File uncompiledScript = SD.open("LEGO/scripts/" + scriptName + ".lsf", FILE_READ);
//  File compiledScript = SD.open("LEGO/scripts/" + scriptName + ".lcs", FILE_WRITE);
//
//  boolean compileError = false;
//  char inChar;
//
//  while (uncompiledScript.available() && !compileError) {
//
//    //DEBUG//DEBUG/Serial.println("More lines are left to compile. No errors have occured.");
//
//    boolean lineEndReached = false;
//    String strReadBuf = "";
//    byte byteReadBuf = 0;
//
//    while (uncompiledScript.available() && !lineEndReached) {
//
//      //DEBUG//DEBUG/Serial.println("Beginning new segment.");
//
//      inChar = uncompiledScript.read();
//
//      //If the line is/becomes a comment, ignore it.
//      if (inChar == '#') {
//        //DEBUG//DEBUG/Serial.println("Line is a comment. Ignoring.");
//        while (uncompiledScript.read() != '\n' && uncompiledScript.available())
//          ;
//        lineEndReached = true;
//      }
//
//      //If this segment begins with a number, hopefully it is really a number because we're treating it as such anyways.
//      else if (isDigit(inChar)) {
//        //DEBUG//DEBUG/Serial.println("Segment is number");
//        //As long as a space (end of segment) or \r (end of line) are not reached, keep reading this number
//        while (inChar != ' ' && inChar != '\r') {
//          strReadBuf += inChar;
//          inChar = uncompiledScript.read();
//          //If we happen to be at the end of the file, grab the last byte and stop the loop
//          if (!uncompiledScript.available()) {
//            strReadBuf += inChar;
//            break;
//          }
//        }
//        byteReadBuf = strReadBuf.toInt();
//        if (byteReadBuf == 13 || byteReadBuf == 10) compiledScript.write('\\');
//        compiledScript.write(byteReadBuf);
//        byteReadBuf = 0;
//        strReadBuf = "";
//        //If it was the end of line, let the upper while loop know
//        if (inChar == '\r') {
//          uncompiledScript.read(); //We've detected the \r already, but still have to pass that \n
//          lineEndReached = true;
//        }
//      }
//
//      // The C U R L Y B O Y indicates that we have an array. Fun ensues
//      else if (inChar == '{') {
//        //DEBUG//DEBUG/Serial.println("Segment is array");
//        inChar = uncompiledScript.read();
//        //Puts the array into the string buffer. Then moves on
//        while (inChar != '}') {
//          strReadBuf += inChar;
//          inChar = uncompiledScript.read();
//          // If the file is over or end of line is reached before the array is closed, Houston has a problem.
//          if (inChar != '}' && !uncompiledScript.available() || inChar == '\r' || inChar == '\n' || inChar == ' ') {
//            compileError = true;
//            break;
//          }
//        }
//        //DEBUG//DEBUG/Serial.println("Curly boy's friend found.");
//        if (compileError) {
//          break;
//        } else {
//          byte segCount = countSegments(strReadBuf, ',');
//          //DEBUG//DEBUG/Serial.println(segCount);
//          //DEBUG//DEBUG/Serial.println(strReadBuf);
//          if (segCount == 13 || segCount == 10) compiledScript.write('\\'); //13 is carriage return and needs to be escaped
//          compiledScript.write(segCount); //Compiled array starts with how many values are in it
//          //Turn the text values into unsigned chars and write them.
//          for (byte i = 0; i < segCount; i++) {
//            if (returnSegment(strReadBuf, i, ',').toInt() == 13 || returnSegment(strReadBuf, i, ',').toInt() == 10) compiledScript.write('\\');
//            compiledScript.write(returnSegment(strReadBuf, i, ',').toInt());
//            //DEBUG//DEBUG/Serial.println(returnSegment(strReadBuf, i, ','));
//          }
//          strReadBuf = "";
//          if (!uncompiledScript.available() || inChar == '\r') {
//            uncompiledScript.read();
//            lineEndReached = true;
//          }
//        }
//      }
//
//      //Someone included a blank line? That can't stop us!
//      else if (inChar == '\r') {
//        //DEBUG//DEBUG/Serial.println("Line is blank.");
//        uncompiledScript.read();
//        lineEndReached = true;
//      }
//
//      //Stray space handler. Will occasionally come up even in properly formatted scripts.
//      else if (inChar == ' ') {
//        //DEBUG//DEBUG/Serial.println("Stray space found. Finding home...");
//      }
//
//      //The only possiblity left is that it is a string to replace. Or there is a big problem. Either or. Hopefully the former.
//      else {
//        //DEBUG//DEBUG/Serial.println("Found string to replace.");
//        while (inChar != '\r' && inChar != ' ') {
//          strReadBuf += inChar;
//          //Check if the string matches any if these. While this is an inefficient way of doing it, this procedure has no time constraints.
//          if (strReadBuf.equals("dimmer") || strReadBuf.equals("func") || strReadBuf.equals("enabled")) {
//            compiledScript.write((byte)1);
//            //DEBUG//DEBUG/Serial.println("Writing 1");
//            break;
//          }
//          else if (strReadBuf.equals("set") || strReadBuf.equals("pin")) {
//            compiledScript.write((byte)0);
//            //DEBUG//DEBUG/Serial.println("Writing 0");
//            break;
//          }
//          else if (strReadBuf.equals("clr") || strReadBuf.equals("bipolar")) {
//            compiledScript.write((byte)2);
//            //DEBUG//DEBUG/Serial.println("Writing 2");
//            break;
//          }
//          else if (strReadBuf.equals("prop") || strReadBuf.equals("inverse")) {
//            compiledScript.write((byte)3);
//            //DEBUG//DEBUG/Serial.println("Writing 3");
//            break;
//          }
//          else if (strReadBuf.equals("remoteDimmer")) {
//            compiledScript.write((byte)4);
//            break;
//          }
//          else if (strReadBuf.equals("pause")) {
//            compiledScript.write((byte)9);
//            break;
//          }
//          else if (strReadBuf.equals("method")) {
//            compiledScript.write((byte)5);
//            break;
//          }
//          else if (strReadBuf.length() > 12 || !uncompiledScript.available()) {
//            //DEBUG//DEBUG/Serial.println("Unknown operator.");
//            compileError = true;
//            break;
//          }
//          else {
//            //DEBUG//DEBUG/Serial.println("No matches. Continuing to read...");
//            inChar = uncompiledScript.read();
//          }
//        }
//        if (compileError)
//          break;
//        strReadBuf = "";
//        if (inChar == '\r') {
//          uncompiledScript.read();
//          lineEndReached = true;
//        }
//      }
//    }
//    compiledScript.write('\n');
//  }
//
//  uncompiledScript.close();
//  compiledScript.close();
//
//}
