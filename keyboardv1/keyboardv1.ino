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
  //  * control characters: ctrl + letter
  //  * F13 is toggles "send next char as "alt""
  //  * F14 is toggles "numLock"
  //  * Other characters are translated.
  if (nextIsAlt) {
    if (useSerialLibrary) {
      Serial.print("alt+");
      if ((translated & CTRL_MOD) == CTRL_MOD) {
        Serial.print("ctrl+");
      }
      if ((translated & SHIFT_MOD) == SHIFT_MOD) {
        Serial.print("shift+");
      }
      Serial.println(translated & 0xff);
    } else {
      // TODO: think about alt+ctrl+char, and alt+ctrl+shift+char
      Keyboard.begin();
      Keyboard.press(KEY_LEFT_ALT);
      if ((translated & CTRL_MOD) == CTRL_MOD) {
        Keyboard.press(KEY_LEFT_CTRL);
      }
      if ((translated & SHIFT_MOD) == SHIFT_MOD) {
        Keyboard.press(KEY_LEFT_SHIFT);
      }
      Keyboard.write(translated & 0xff);
      delay(100);
      Keyboard.releaseAll();
      Keyboard.end();
    }
    nextIsAlt = false;
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
  else if (translated == KEY_F13) {
    // This will ONLY trigger for F13

    // TODO: Maybe use ctrl+f13 to represent "next char is ctrl-" and ctrl-shift_f13
    // to represent "next char is ctrl+shift+", etc.
    if (useSerialLibrary) {
      Serial.println("F13: Next-alt");
    }
    nextIsAlt = true;
  } else if (translated == KEY_F14) {
    // This will ONLY trigger for F14
    numLock = !numLock;
    if (useSerialLibrary) {
      Serial.print("F14: Toggling numlock "); Serial.println(numLock);
    }
  } else {
    // Function key or other unprintable.
    if (useSerialLibrary) {
      Serial.print("Unprintable: ");
      if ((translated & CTRL_MOD) == CTRL_MOD) {
        Serial.print("ctrl+");
      }
      if ((translated & SHIFT_MOD) == SHIFT_MOD) {
        Serial.print("shift+");
      }
      Serial.print((int) translated & 0xff); Serial.print(" decimal; was decimal: "); Serial.println(key);
    } else {
      Keyboard.begin();
      if ((translated & CTRL_MOD) == CTRL_MOD) {
        Keyboard.press(KEY_LEFT_CTRL);
      }
      if ((translated & SHIFT_MOD) == SHIFT_MOD) {
        Keyboard.press(KEY_LEFT_SHIFT);
      }
      Keyboard.press(translated & 0xff);
      delay(100);
      Keyboard.releaseAll();
      Keyboard.end();
    }
  }
}
