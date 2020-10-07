/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud and looks up the value to send back.
*/

#include <keyboard.h>

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

char translationTable[256];
char numLockTable[256];

#define SPECIAL 255
#define VG_F13 205
#define VG_F14 206
#define VG_NP0 128
#define VG_NP_DOT 207

void setupTable() {
  for (int i = 0; i < 256; i++)  {
    if (i >= ' ' && i <= '~') {
      // Printable characters
      translationTable[i] = i;
    } else if (i < ' '){
      translationTable[i] = SPECIAL;  // Control characters, except for exceptions, below.
    } else {
      translationTable[i] = ' ';  // initialize the whole table so defaults anything not mapped to space
    }
    numLockTable[i] = ' ';
	}
  
  //translationTable[VG_BACKSPACE] = ;
  translationTable[8] = KEY_TAB;
  translationTable[13] = KEY_RETURN;
  translationTable[27] = KEY_ESC;

  translationTable[192] = KEY_F1; // map vg help key to f1
  translationTable[193] = KEY_F2; // map vg f1 to f2
  translationTable[194] = KEY_F3; // map vg f2 to f3
  translationTable[195] = KEY_F4; // map vg f3 to f4
  translationTable[196] = KEY_F5; // map vg f4 to f6
  translationTable[197] = KEY_F6; // map vg f5 to f6
  // NOTE: NO 198 (F6), because you can't type F6 on this keyboard...
  translationTable[199] = KEY_F7; // FROM HERE, the mappings are correct
  translationTable[200] = KEY_F8;
  translationTable[201] = KEY_F9;
  translationTable[202] = KEY_F10;
  translationTable[203] = KEY_F11;
  translationTable[204] = KEY_F12;
  translationTable[VG_F13] = SPECIAL;
  translationTable[VG_F14] = SPECIAL;

  // translationTable[VG_UP] = KEY_UP_ARROW;
  // translationTable[VG_DOWN] = KEY_DOWN_ARROW; 
  // translationTable[VG_LEFT] = KEY_LEFT_ARROW; 
  // translationTable[VG_RIGHT] = KEY_RIGHT_ARROW;
  // translationTable[VG_NP_ENTER] = KEY_RETURN;

  // Numlock:
  numLockTable[VG_NP0] = '0';
  // TODO: NP1-9, enter, comma, dash, up, down, left, right
  // numLockTable[VG_NP_1] = '1';
  // numLockTable[VG_NP_2] = '2';
  // numLockTable[VG_NP_3] = '3';
  // numLockTable[VG_NP_4] = '4';
  // numLockTable[VG_NP_5] = '5';
  // numLockTable[VG_NP_6] = '6';
  // numLockTable[VG_NP_7] = '7'; 
  // numLockTable[VG_NP_8] = '8'; 
  // numLockTable[VG_NP_9] = '9';
  // numLockTable[VG_NP_COMMA] = ','; 
  numLockTable[VG_NP_DOT] = '.'; // NP DOT
  // numlockTable[VG_NP_MINUS] = '-';

  // Numlock off: cursor keys:
  translationTable[VG_NP0] = KEY_INS;
  // translationTable{VG_NP_1] = KEY_END;
  // translationTable{VG_NP_2] = KEY_DOWN_ARROW;
  // translationTable{VG_NP_3] = KEY_PAGE_DOWN;
  // translationTable{VG_NP_4] = KEY_LEFT_ARROW;
  // translationTable{VG_NP_3] = '5';
  // translationTable{VG_NP_6] = KEY_RIGHT_ARROW;
  // translationTable{VG_NP_7] = KEY_HOME;
  // translationTable{VG_NP_8] = KEY_UP_ARROW;
  // translationTable{VG_NP_9] = KEY_PAGE_UP;
  // translationTable[VG_NP_COMMA] = ','; 
  translationTable[VG_NP_DOT] = KEY_DELETE;
  // translationTable[VG_NP_MINUS] = '-';
}


// Microseconds to wait between bits. Corresponds to 110 baud, empirically determined.
const int baudDelay = 3387;


void loop() {
  int data = digitalRead(dataPin);
  if (data == LOW) {

    // Wait half a cycle so that we're sampling in the middle of the bit.
    delayMicroseconds(baudDelay / 2);

    char key = getChar();
    int theKey = (int)key;
    Serial.print("Raw char: ");
    Serial.print(key);
    Serial.print(" (decimal "); Serial.print(theKey); Serial.print(" =0b");
    for (int i = 7; i >= 0; i--) {
      Serial.write(bitRead(theKey, i));
    }
    Serial.println(")");
    sendChar(key);
  }
}

char getChar() {
  int theKey = 0;
  for (int i = 0; i < 8; ++i) {
    data = digitalRead(dataPin);

    // Shift right and possibly shift in a 1.
    theKey >>= 1;
    if (data == HIGH) {
      theKey = theKey | 0x80;
    }

    // Wait for the next bit
    delayMicroseconds(baudDelay);
  }

  return (char) theKey;
}

bool nextIsAlt = false;
bool numLock = false;

void sendChar(char key) {
  // Figure out what to do with the key
  // 	* printable characters just get returned.
  // 	* control characters: ctrl + letter
  //    * F13 is "send next char as "alt""
  //    * F14 is "numLock toggle"
  //    * Other characters are translated.
  if (key >= ' ' && i <= '~') {
    if (nextIsAlt) {
      Serial.print("Alt+"); Serial.println(key);
      // Keyboard.begin();
      // Keyboard.press(KEY_ALT)
      // Keyboard.press(key);
      // delay(100);// I've seen this elsewhere. sometimes 20 ms
      // Keyboard.releaseAll();
      // Keyboard.end();
      nextIsAlt = false;
      return;
    }

    Serial.print("Printable: "); Serial.println(key);
    // Keyboard.begin();
    // Keyboard.write(key);
    // Keyboard.end();
    return;
  }

  // TODO: think about alt+ctrl+char, and alt+ctrl+shift+char
  char translated;
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
      Serial.print("ctrl-"); Serial.println(key+64);
      // Keyboard.begin();
      // Keyboard.press(KEY_LEFT_CTRL);
      // Keyboard.press(key + 64)
      // delay(100);// I've seen this elsewhere. sometimes 20 ms
      // Keyboard.releaseAll();
      // Keyboard.end();
    }
  } else if (translated >= ' ' && translated <= '~') {
    Serial.print("Printable: "); Serial.print(translated); Serial.print("; was decimal: "); Serial.print((int) key);
    // Keyboard.begin();
    // Keyboard.write(translated);
    // Keyboard.end();
  } else {
    Serial.print("Unprintable: "); Serial.print((int) translated); Serial.print(" decimal; was decimal: "); Serial.print((int) key);
    // Keyboard.begin();
    // Keyboard.press(key);
    // delay(100);// I've seen this elsewhere. sometimes 20 ms
    // Keyboard.releaseAll();
    // Keyboard.end();
  }
}
