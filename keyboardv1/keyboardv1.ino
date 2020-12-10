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
  Serial.println("Hello keyboardv1 with setup. Compiled: " __DATE__);
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
    delay(100);
    reset();
  }
  useSerialLibrary = use;
}

// Microseconds to wait between bits. Corresponds to 110 baud, empirically determined.
#define BAUD_DELAY_US 3387

// Have to have at least one high reading before triggering low.
boolean oneHigh = false;

void loop() {
  int data = digitalRead(dataPin);
  if (oneHigh && data == LOW) {
    byte key = getKeyFromVG();
    if (useSerialLibrary) {
      Serial.print("Raw char: "); Serial.print(key); Serial.print(" decimal 0b"); Serial.println(key, BIN);
    }
    if (key != 0 && key != 255) {
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
  // Wait half a cycle so that we're sampling in the middle of the bit.
  delayMicroseconds(BAUD_DELAY_US / 2);

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
bool nextIsCtrl = false;
bool nextIsShift = false;
bool numLock = false;

int translationTable[256];
int numLockTable[256];

/* Send a key chord over the USB. */
void sendChar(byte key) {
  // TEMPORARY
  if (key == VG_F6 + VG_CTRL + VG_SHIFT) { // 246
    // Toggles the keyboard-iness
    setUseSerialLibrary(!useSerialLibrary);
  }

  int translated = translationTable[key];
  if (numLock) {
    translated = numLockTable[key];
  }

  // Figure out what to do with the key
  //  * If "next is alt", send this char as alt
  //  * printable characters just get sent
  //  * F13 is toggles "send next char as "alt""
  //  * F14 is toggles "numLock"
  //  * Other characters are translated.
  if (nextIsAlt || nextIsCtrl || nextIsShift) {
    if (nextIsAlt) {
      translated |= ALT_MOD;
    }
    if (nextIsCtrl) {
      translated |= CTRL_MOD;
    }
    if (nextIsShift) {
      translated |= SHIFT_MOD;
    }
    pressKey(translated, key);
    nextIsAlt = false;
    nextIsCtrl = false;
    nextIsShift = false;
  }
  // Printable character
  else if (translated >= ' ' && translated <= '~') {
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
  }
  // Deal with unprintables
  else if ((translated & 0xff) == KEY_F13) {
    nextIsAlt = translated == KEY_F13;
    nextIsCtrl = (translated & CTRL_MOD) == CTRL_MOD;
    nextIsShift = (translated & SHIFT_MOD) == SHIFT_MOD;
    // if next is shift, but neither of the others, it's alt+shift.
    if (nextIsShift && !nextIsAlt && !nextIsCtrl) {
      nextIsAlt = true;
    }
    if (useSerialLibrary) {
      Serial.print("F13: nextIsAlt. nextIsCtrl: "); Serial.print(nextIsCtrl);
      Serial.print(". nextIsShift: "); Serial.println(nextIsShift);
    }
  } else if (translated == KEY_F14) {
    numLock = !numLock;
    if (useSerialLibrary) {
      Serial.print("F14: Toggling numlock "); Serial.println(numLock);
    }
  } else {
    // Function key or other unprintable.
    pressKey(translated, key);
  }
}

void pressKey(int translated, byte key) {
  byte rawCode = (translated & 0xff);
  if (useSerialLibrary) {
    if ((translated & CTRL_MOD) == CTRL_MOD) {
      Serial.print("ctrl+");
    }
    if ((translated & ALT_MOD) == ALT_MOD) {
      Serial.print("alt+");
    }
    if ((translated & SHIFT_MOD) == SHIFT_MOD) {
      Serial.print("shift+");
    }
    if (key >= ' ' && key <= '~') {
      Serial.print((char) rawCode); Serial.print(" ; was: "); Serial.println(key);
    } else {
      Serial.print((int) rawCode); Serial.print(" decimal; was decimal: "); Serial.println(key);
    }
  } else {
    Keyboard.begin();
    if ((translated & CTRL_MOD) == CTRL_MOD) {
      Keyboard.press(KEY_LEFT_CTRL);
    }
    if ((translated & ALT_MOD) == ALT_MOD) {
      Keyboard.press(KEY_LEFT_ALT);
    }
    if ((translated & SHIFT_MOD) == SHIFT_MOD) {
      Keyboard.press(KEY_LEFT_SHIFT);
    }
    Keyboard.write(rawCode);
    delay(100);
    Keyboard.releaseAll();
    Keyboard.end();
  }
}
