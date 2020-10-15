/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud and looks up the value to send back.
*/

#include <Keyboard.h>

#ifndef KEY_F12

// Arduino UNO can't be used as a HID device, but in order
// to test the logic, we define some of the keys here.

#warning "Using fake keyboard definitions"

#define KEY_LEFT_CTRL   0x80
#define KEY_LEFT_ALT    0x82

#define KEY_UP_ARROW    0xDA
#define KEY_DOWN_ARROW  0xD9
#define KEY_LEFT_ARROW  0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEY_BACKSPACE   0xB2
#define KEY_TAB         0xB3
#define KEY_RETURN      0xB0
#define KEY_ESC         0xB1
#define KEY_INSERT      0xD1
#define KEY_DELETE      0xD4
#define KEY_PAGE_UP     0xD3
#define KEY_PAGE_DOWN   0xD6
#define KEY_HOME        0xD2
#define KEY_END         0xD5
#define KEY_F1          0xC2
#define KEY_F2          0xC3
#define KEY_F3          0xC4
#define KEY_F4          0xC5
#define KEY_F5          0xC6
#define KEY_F6          0xC7
#define KEY_F7          0xC8
#define KEY_F8          0xC9
#define KEY_F9          0xCA
#define KEY_F10         0xCB
#define KEY_F11         0xCC
#define KEY_F12         0xCD
#endif

const int dataPin = 7;      // pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 9;     // pin 2 on the KB connector (and pin 4 on the RJ12)

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);
  setupTable();

  Serial.begin(9600);
  Serial.println("Hello keyboard passthrough");

  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin, LOW);
  delay(10);
  digitalWrite(resetPin, HIGH);
}

byte translationTable[256];
byte numLockTable[256];

#define SPECIAL 255

#define VG_BACKSPACE 8
#define VG_DEL 127
// Maps to "Windows key"
#define VG_LINEFEED 10

#define VG_HELP 192
#define VG_F1 193
#define VG_F2 194
#define VG_F3 195
#define VG_F4 196
#define VG_F5 197
#define VG_F6 198
#define VG_F7 199
#define VG_F8 200
#define VG_F9 201
#define VG_F10 202
#define VG_F11 203
#define VG_F12 204
#define VG_F13 205
#define VG_F14 206

// Numeric keypad:
#define VG_NP_DOT 207
#define VG_NP_0 128
#define VG_NP_1 129
#define VG_NP_2 130
#define VG_NP_3 131
#define VG_NP_4 132
#define VG_NP_5 133
#define VG_NP_6 134
#define VG_NP_7 135
#define VG_NP_8 136
#define VG_NP_9 167
#define VG_NP_DASH 142
#define VG_NP_COMMA 143
#define VG_NP_UP 138
#define VG_NP_DOWN 139
#define VG_NP_LEFT 140
#define VG_NP_RIGHT 141
#define VG_NP_ENTER 160

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

  translationTable[9] = KEY_TAB;
  // maybe this should be alt, and F13 is Windows?
  translationTable[VG_LINEFEED] = KEY_LEFT_GUI; // Windows key, by itself. Will not work as-is, as a modifier
  translationTable[13] = KEY_RETURN;
  translationTable[27] = KEY_ESC;
  translationTable[VG_DEL] = KEY_DELETE;
  translationTable[VG_BACKSPACE] = KEY_BACKSPACE;

  translationTable[VG_HELP] = KEY_F1; // map vg help key to f1
  translationTable[VG_F1] = KEY_F2; // map vg f1 to f2
  translationTable[VG_F2] = KEY_F3; // map vg f2 to f3
  translationTable[VG_F3] = KEY_F4; // map vg f3 to f4
  translationTable[VG_F4] = KEY_F5; // map vg f4 to f6
  translationTable[VG_F5] = KEY_F6; // map vg f5 to f6
  // NOTE: NO 198 (F6), because you can't type F6 on this keyboard...
  translationTable[VG_F7] = KEY_F7; // FROM HERE, the mappings are correct
  translationTable[VG_F8] = KEY_F8;
  translationTable[VG_F9] = KEY_F9;
  translationTable[VG_F10] = KEY_F10;
  translationTable[VG_F11] = KEY_F11;
  translationTable[VG_F12] = KEY_F12;
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


void loop() {
  int data = digitalRead(dataPin);
  if (data == LOW) {
    // Wait half a cycle so that we're sampling in the middle of the bit.
    delayMicroseconds(baudDelay / 2);

    byte key = getChar();
    Serial.print("Raw char: "); Serial.print(key); Serial.print(" decimal 0b"); Serial.println(key, BIN);
    sendChar(key);
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
  // Figure out what to do with the key
  // 	* printable characters just get returned.
  // 	* control characters: ctrl + letter
  //    * F13 is "send next char as "alt""
  //    * F14 is "numLock toggle"
  //    * Other characters are translated.
  if (key >= ' ' && key <= '~') {
    if (nextIsAlt) {
      Serial.print("Alt+"); Serial.println((char) key);
      // Keyboard.begin();
      // Keyboard.press(KEY_ALT)
      // Keyboard.press(key);
      // delay(100);// I've seen this elsewhere. sometimes 20 ms
      // Keyboard.releaseAll();
      // Keyboard.end();
      nextIsAlt = false;
      return;
    }

    Serial.print("Printable: "); Serial.println((char) key);
    // Keyboard.begin();
    // Keyboard.write(key);
    // Keyboard.end();
    return;
  }

  // TODO: think about alt+ctrl+char, and alt+ctrl+shift+char
  byte translated;
  if (numLock) {
    translated = numLockTable[key];
  } else {
    translated = translationTable[key];
  }

  if (translated == SPECIAL) {
    // deal with F13 & F14
    if (key == VG_F13) {
      Serial.println("F13: Next-alt");
      nextIsAlt = true;
    } else if (key == VG_F14) {
      numLock = !numLock;
      Serial.print("F14: Toggling numlock "); Serial.println(numLock);
    } else if (key < ' ') {
      Serial.print("ctrl-"); Serial.println((char)(key + 64));
      // Keyboard.begin();
      // Keyboard.press(KEY_LEFT_CTRL);
      // Keyboard.press(key + 64)
      // delay(100);// I've seen this elsewhere. sometimes 20 ms
      // Keyboard.releaseAll();
      // Keyboard.end();
    }
  } else if (translated > ' ' && translated <= '~') {
    Serial.print("Printable: "); Serial.print((char) translated); Serial.print("; was decimal: "); Serial.println(key);
    // Keyboard.begin();
    // Keyboard.write(translated);
    // Keyboard.end();
  } else {
    Serial.print("Unprintable: "); Serial.print((int) translated); Serial.print(" decimal; was decimal: "); Serial.println(key);
    // Keyboard.begin();
    // Keyboard.press(key);
    // delay(100);// I've seen this elsewhere. sometimes 20 ms
    // Keyboard.releaseAll();
    // Keyboard.end();
  }
}
