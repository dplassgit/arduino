/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud and looks up the value to send back.
*/
#include <Keyboard.h>
#include "vg_keys.h"

bool useSerialLibrary = true;

const int dataPin = 7;      // pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 9;     // pin 2 on the KB connector (and pin 4 on the RJ12)

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);
  setupTable();

  Serial.begin(9600);
  Serial.println("Hello keyboard passthrough. Starting with serial");

  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin, LOW);
  delay(10);
  digitalWrite(resetPin, HIGH);
}

void setUseSerialLibrary(bool use) {
  if (use) {
    Serial.println("Stopping keyboard, starting serial");
  } else {
    Serial.println("Stopping serial, starting keyboard");
  }
  useSerialLibrary = use;
}

byte translationTable[256];
byte numLockTable[256];

#define SPECIAL 255


void setupTable() {
  for (int i = 0; i < 256; i++)  {
    if (i >= ' ' && i <= '~') {
      // Printable characters
      translationTable[i] = i;
    } else if (i < ' ') {
      // Control characters, except for exceptions, below.
      translationTable[i] = SPECIAL;
    } else {
      // Initialize the whole table so defaults anything not mapped to space
      translationTable[i] = ' ';
    }
    numLockTable[i] = translationTable[i];
  }

  translationTable[VG_TAB] = KEY_TAB;
  // maybe this should be alt, and F13 is Windows?
  translationTable[VG_LINEFEED] = KEY_LEFT_GUI; // Windows key, by itself. Will not work as-is, as a modifier
  translationTable[13] = KEY_RETURN;
  translationTable[VG_ESC] = KEY_ESC;
  // translationTable[29] = SPECIAL; // shift-tab
  // translationTable[28] = SPECIAL; // control-backslash
  // Yes, delete and backspace are swapped.
  translationTable[VG_DEL] = KEY_BACKSPACE;
  translationTable[VG_BACKSPACE] = KEY_DELETE;

  translationTable[VG_F1] = KEY_F1;
  translationTable[VG_F2] = KEY_F2;
  translationTable[VG_F3] = KEY_F3;
  translationTable[VG_F4] = KEY_F4;
  translationTable[VG_F5] = KEY_F5;
  translationTable[VG_F6] = KEY_F6;
  translationTable[VG_F7] = KEY_F7;
  translationTable[VG_F8] = KEY_F8;
  translationTable[VG_F9] = KEY_F9;
  translationTable[VG_F10] = KEY_F10;
  translationTable[VG_F11] = KEY_F11;
  translationTable[VG_F12] = SPECIAL; // KEY_F12;
  translationTable[VG_F13] = SPECIAL;
  translationTable[VG_F14] = SPECIAL;

  translationTable[VG_UP] = KEY_UP_ARROW;
  translationTable[VG_DOWN] = KEY_DOWN_ARROW;
  translationTable[VG_LEFT] = KEY_LEFT_ARROW;
  translationTable[VG_RIGHT] = KEY_RIGHT_ARROW;

  translationTable[VG_NP_ENTER] = KEY_RETURN;

  // Numlock off: cursor keys:
  translationTable[VG_NP_0] = KEY_INSERT;
  translationTable[VG_NP_1] = KEY_END;
  translationTable[VG_NP_2] = KEY_DOWN_ARROW;
  translationTable[VG_NP_3] = KEY_PAGE_DOWN;
  translationTable[VG_NP_4] = KEY_LEFT_ARROW;
  translationTable[VG_NP_5] = '5';
  translationTable[VG_NP_6] = KEY_RIGHT_ARROW;
  translationTable[VG_NP_7] = KEY_HOME;
  translationTable[VG_NP_8] = KEY_UP_ARROW;
  translationTable[VG_NP_9] = KEY_PAGE_UP;
  translationTable[VG_NP_COMMA] = ',';
  translationTable[VG_NP_DASH] = '-';
  translationTable[VG_NP_DOT] = KEY_DELETE;

  for (int i = 0; i < 256; i++)  {
    numLockTable[i] = translationTable[i];
  }

  // Numlock overrides:
  numLockTable[VG_NP_0] = '0';
  numLockTable[VG_NP_1] = '1';
  numLockTable[VG_NP_2] = '2';
  numLockTable[VG_NP_3] = '3';
  numLockTable[VG_NP_4] = '4';
  numLockTable[VG_NP_5] = '5';
  numLockTable[VG_NP_6] = '6';
  numLockTable[VG_NP_7] = '7';
  numLockTable[VG_NP_8] = '8';
  numLockTable[VG_NP_9] = '9';
  numLockTable[VG_NP_DOT] = '.';
}


// Microseconds to wait between bits. Corresponds to 110 baud, empirically determined.
const int baudDelay = 3387;

// Have to have at least one high reading before triggering low.
boolean oneHigh = false;

void loop() {
  int data = digitalRead(dataPin);
  if (oneHigh && data == LOW) {
    // Wait half a cycle so that we're sampling in the middle of the bit.
    delayMicroseconds(baudDelay / 2);

    byte key = getChar();
    if (useSerialLibrary) {
      Serial.print("Raw char: "); Serial.print(key); Serial.print(" decimal 0b"); Serial.println(key, BIN);
    }
    if (key != 0) {
      sendChar(key);
    }
    oneHigh = false;
  } else {
    oneHigh = true;
  }
}

byte getChar() {
  int theKey = 0;
  for (int i = 0; i <= 8; ++i) {
    int data = digitalRead(dataPin);

    // Shift right and possibly shift in a 1.
    theKey >>= 1;
    if (data == HIGH) {
      theKey = theKey | 0x80;
    }

    // Wait for the next bit
    delayMicroseconds(baudDelay);
  }

  return (byte) theKey;
}

bool nextIsAlt = false;
bool numLock = false;

void sendChar(byte key) {
  if (key == 246) { // ctrl+shift+F6 (help)
    // Toggles the keyboard-iness
    setUseSerialLibrary(!useSerialLibrary);
  }

  byte translated;
  if (numLock) {
    translated = numLockTable[key];
  } else {
    translated = translationTable[key];
  }

  // Figure out what to do with the key
  //  * printable characters just get returned.
  //  * control characters: ctrl + letter
  //  * F13 is "send next char as "alt""
  //  * F14 is "numLock toggle"
  //  * Other characters are translated.
  if (nextIsAlt) {
    if (useSerialLibrary) {
      Serial.print("Alt+"); Serial.println(translated);
    } else {
      // TODO: think about alt+ctrl+char, and alt+ctrl+shift+char
      Keyboard.begin();
      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.write(translated);
      delay(100); // I've seen this elsewhere. sometimes 100 ms
      Keyboard.releaseAll();
      Keyboard.end();
    }
    nextIsAlt = false;
    return;
  }

  if (key >= ' ' && key <= '~') {
    if (useSerialLibrary) {
      Serial.print("Printable: "); Serial.println((char) key);
    } else {
      Keyboard.begin();
      Keyboard.write(key);
      Keyboard.end();
    }
    return;
  }

  if (translated == SPECIAL) {
    // deal with F13 & F14
    if (key == VG_F12) {
      if (useSerialLibrary) {
        Serial.println("F12: Next-alt");
      }
      nextIsAlt = true;
    } else if (key == VG_F14) {
      numLock = !numLock;
      if (useSerialLibrary) {
        Serial.print("F14: Toggling numlock "); Serial.println(numLock);
      }
    } else if (key < ' ') {
      if (useSerialLibrary) {
        Serial.print("ctrl-"); Serial.println((char)(key + 96));
      } else {
        Keyboard.begin();
        Keyboard.press(KEY_LEFT_CTRL);
        // this currently can only send lower case control characters. Need to look into this more.
        // control+number is not possible because it's not represented in ASCII.
        // control+shift+letter is also not possible.
        Keyboard.press((char) (key + 96));
        delay(20);
        Keyboard.releaseAll();
        Keyboard.end();
      }
    }
  } else if (translated > ' ' && translated <= '~') {
    if (useSerialLibrary) {
      Serial.print("Printable: "); Serial.print((char) translated); Serial.print("; was decimal: "); Serial.println(key);
    } else {
      Keyboard.begin();
      Keyboard.write(translated);
      Keyboard.end();
    }
  } else {
    if (useSerialLibrary) {
      Serial.print("Unprintable: "); Serial.print((int) translated); Serial.print(" decimal; was decimal: "); Serial.println(key);
    } else {
      Keyboard.begin();
      Keyboard.write(translated);
      Keyboard.end();
    }
  }
}
