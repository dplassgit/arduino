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

    byte key = getKeyFromVG();
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

/** Get a keystroke from the VG keyboard */
byte getKeyFromVG() {
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

/* Send a key chord over the USB. */
void sendChar(byte key) {
  // TEMPORARY
  if (key == VG_F6 + VG_CTRL + VG_SHIFT) { // 246
    // Toggles the keyboard-iness
    setUseSerialLibrary(!useSerialLibrary);
  }

  byte translated = translationTable[key];
  if (numLock) {
    translated = numLockTable[key];
  }

  // Figure out what to do with the key
  //  * If "next is alt", send this char as alt
  //  * printable characters just get sent
  //  * control characters: ctrl + letter
  //  * F13 is toggles "send next char as "alt""
  //  * F14 is toggles "numLock"
  //  * Other characters are translated.
  if (nextIsAlt) {
    if (useSerialLibrary) {
      Serial.print("Alt+");
      if (key == VG_SHIFT_TAB) {
        Serial.println("shift+tab");
      } else {
        Serial.println(translated);
      }
    } else {
      // TODO: think about alt+ctrl+char, and alt+ctrl+shift+char
      Keyboard.begin();
      Keyboard.press(KEY_LEFT_ALT);
      if (key == VG_SHIFT_TAB) {
        // Unfortunately, shift+tab is 29, which is NOT 8+16 (shift),
        // so we have to hack this.
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_TAB);
      } else {
        Keyboard.write(translated);
      }
      delay(100);
      Keyboard.releaseAll();
      Keyboard.end();
    }
    nextIsAlt = false;
    return;
  }

  // Printable character
  if (translated >= ' ' && translated <= '~') {
    if (useSerialLibrary) {
      Serial.print("Printable: "); Serial.print((char) translated);
      if (key != translated) {
        Serial.print("; was: "); Serial.println(key);
      } else {
        Serial.println();
      }
    } else {
      Keyboard.begin();
      Keyboard.write(translated);
      Keyboard.end();
    }
    return;
  }

  // Deal with unprintables
  if (translated == SPECIAL) {
    if (key == VG_CTRL_SHIFT_DASH) {
      if (useSerialLibrary) {
        Serial.print("Unprintable: ctrl+shift+dash: "); Serial.println((char) key);
      } else {
        Keyboard.begin();
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press('-');
        delay(100);
        Keyboard.releaseAll();
        Keyboard.end();
      }
    } else if (key == VG_CTRL_SHIFT_6) {
      if (useSerialLibrary) {
        Serial.print("Unprintable: ctrl+shift+6: "); Serial.println((char) key);
      } else {
        Keyboard.begin();
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press('6');
        delay(100);
        Keyboard.releaseAll();
        Keyboard.end();
      }
    } else if (key == VG_CTRL_BACKSLASH) {
      if (useSerialLibrary) {
        Serial.print("Unprintable: ctrl+backslash: "); Serial.println((char) key);
      } else {
        Keyboard.begin();
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('\\');
        delay(100);
        Keyboard.releaseAll();
        Keyboard.end();
      }
    } else if (key == VG_SHIFT_TAB) {
      if (useSerialLibrary) {
        Serial.print("Unprintable: Shift tab: "); Serial.println((char) key);
      } else {
        Keyboard.begin();
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_TAB);
        delay(100);
        Keyboard.releaseAll();
        Keyboard.end();
      }
    }
  } else if (translated == KEY_F13) {
    // This will trigger for F13, ctrl+F13, shift+F13, etc.

    // TODO: Maybe use ctrl+f13 to represent "next char is ctrl-" and ctrl-shift_f13
    // to represent "next char is ctrl+shift+", etc.
    if (useSerialLibrary) {
      Serial.println("F13: Next-alt");
    }
    nextIsAlt = true;
  } else if (translated == KEY_F14) {
    // This will trigger for F14, ctrl+F14, shift+F14, etc.
    numLock = !numLock;
    if (useSerialLibrary) {
      Serial.print("F14: Toggling numlock "); Serial.println(numLock);
    }
  } else if (key < ' ') {
    // Send control character.
    if (useSerialLibrary) {
      Serial.print("ctrl-"); Serial.println((char)(key + 96));
    } else {
      // This currently can only send lower case control characters. TODO: look into this more.
      // ctrl+number (or ctrl+shift+number) is not possible because they're not represented in ASCII.
      // Similarly, ctrl+shift+letter is also not possible.
      Keyboard.begin();
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press((char) (key + 96));
      delay(100);
      Keyboard.releaseAll();
      Keyboard.end();
    }
  } else {
    // Function key or other unprintable.
    if (useSerialLibrary) {
      Serial.print("Unprintable: ");
      if (key > 127 && (key & VG_CTRL) == VG_CTRL) {
        Serial.print("ctrl+");
      }
      if (key > 127 && (key & VG_SHIFT) == VG_SHIFT) {
        Serial.print("shift+");
      }
      Serial.print((int) translated); Serial.print(" decimal; was decimal: "); Serial.println(key);
    } else {
      Keyboard.begin();
      if (key > 127 && (key & VG_SHIFT) == VG_SHIFT) {
        Keyboard.press(KEY_LEFT_SHIFT);
      }
      if (key > 127 && (key & VG_CTRL) == VG_CTRL) {
        Keyboard.press(KEY_LEFT_CTRL);
      }
      Keyboard.press(translated);
      delay(100);
      Keyboard.releaseAll();
      Keyboard.end();
    }
  }
}
