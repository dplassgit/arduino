/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud and looks up the value to send back.
*/
#include <Keyboard.h>
#include "vg_keys.h"

// Whether to write logs through the serial library (true, default),
// or to act as a true USB keyboard (false).
bool useSerialLibrary = true;

const int dataPin = 7;      // pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 9;     // pin 2 on the KB connector (and pin 4 on the RJ12)

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);
  setupTable();

  Serial.begin(9600);
  Serial.println("Hello keyboardv1 with setup");
  setUseSerialLibrary(true);
}

void reset() {
  // Set the reset pin low for 5 ms.
  digitalWrite(resetPin, LOW);
  delay(5);
  digitalWrite(resetPin, HIGH);
}

void setUseSerialLibrary(bool use) {
  if (use) {
    Serial.println("Stopping keyboard, starting serial");
    reset();
  } else {
    Serial.println("Stopping serial, starting keyboard");
    reset();
    reset();
  }
  useSerialLibrary = use;
}

byte translationTable[256];
byte numLockTable[256];


// Microseconds to wait between bits. Corresponds to 110 baud, empirically determined.
#define BAUD_DELAY_US 3387

// Have to have at least one high reading before triggering low.
boolean oneHigh = false;

void loop() {
  int data = digitalRead(dataPin);
  if (oneHigh && data == LOW) {
    // Wait half a cycle so that we're sampling in the middle of the bit.
    delayMicroseconds(BAUD_DELAY_US / 2);

    byte key = getChar();
    if (useSerialLibrary) {
      Serial.print("Raw char: "); Serial.print(key); Serial.print(" decimal 0b"); Serial.println(key, BIN);
    }
    if (key != 0) {
      // If the keyboard has no power it will return all lows, sending back an avalanche of
      // zeros. Ignore (for now?)
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
    delayMicroseconds(BAUD_DELAY_US);
  }

  return (byte) theKey;
}

bool nextIsAlt = false;
bool numLock = false;

void sendChar(byte key) {
  if (key == 246) { // ctrl+shift+F6 (Help)
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
    if (key == VG_F13) {
      if (useSerialLibrary) {
        Serial.println("F13: Next-alt");
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
        // Maybe use ctrl+f13 to represent "next char is ctrl-" and ctrl-shift_f13 to represent "next char is ctrl+shift+"?
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
