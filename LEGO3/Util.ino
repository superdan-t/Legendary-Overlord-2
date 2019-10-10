char indexInString(char string[], char len, char c) {
  //Searches from the end rather than beginning to save a variable
  while (len != -1) {
    len--;
    if (string[len] == c) {
      return len;
    }
  }
  return -1;
}

bool stringsEqual(char string1[], char len1, char string2[], char len2) {
  //strings can't be equal if lengths aren't equal
  if (len1 != len2) {
    return false;
  }
  //lens are for sure the same now, so I just chose len1
  while (len1 != -1) {
    len1--;
    if (string1[len1] != string2[len2]) {
      return false;
    }
  }
  return true;
}
