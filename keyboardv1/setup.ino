#include <Keyboard.h>
#include "vg_keys.h"

/** Set up the translation table and numLock table for all keys */
void setupTable() {
  for (int i = 0; i < 256; i++)  {
    if (i >= ' ' && i <= '~') {
      // Printable characters
      translationTable[i] = i;
    } else {
      // Initialize the whole table so it defaults anything not mapped to space
      translationTable[i] = ' ';
    }
    numLockTable[i] = translationTable[i];
  }

  translationTable[VG_TAB] = KEY_TAB;
  translationTable[VG_LINEFEED] = KEY_LEFT_GUI; // Windows key
  translationTable[VG_ENTER] = KEY_RETURN;
  translationTable[VG_ESC] = KEY_ESC;

  translationTable[VG_SHIFT_TAB] = SPECIAL;
  translationTable[VG_CTRL_BACKSLASH] = SPECIAL;
  translationTable[VG_CTRL_SHIFT_DASH] = SPECIAL;
  translationTable[VG_CTRL_SHIFT_6] = SPECIAL;

  // Delete and backspace are swapped.
  translationTable[VG_DEL] = KEY_BACKSPACE;
  translationTable[VG_BACKSPACE] = KEY_DELETE;

  setupKeyWModifiers(VG_F1, KEY_F1);
  setupKeyWModifiers(VG_F2, KEY_F2);
  setupKeyWModifiers(VG_F3, KEY_F3);
  setupKeyWModifiers(VG_F4, KEY_F4);
  setupKeyWModifiers(VG_F5, KEY_F5);
  setupKeyWModifiers(VG_F6, KEY_F6);
  setupKeyWModifiers(VG_F7, KEY_F7);
  setupKeyWModifiers(VG_F8, KEY_F8);
  setupKeyWModifiers(VG_F9, KEY_F9);
  setupKeyWModifiers(VG_F10, KEY_F10);
  setupKeyWModifiers(VG_F11, KEY_F11);
  setupKeyWModifiers(VG_F12, KEY_F12);
  setupKeyWModifiers(VG_F13, KEY_F13);
  setupKeyWModifiers(VG_F14, KEY_F14);

  setupKeyWModifiers(VG_UP, KEY_UP_ARROW);
  setupKeyWModifiers(VG_DOWN, KEY_DOWN_ARROW);
  setupKeyWModifiers(VG_LEFT, KEY_LEFT_ARROW);
  setupKeyWModifiers(VG_RIGHT, KEY_RIGHT_ARROW);

  translationTable[VG_NP_ENTER] = KEY_RETURN;
  translationTable[VG_NP_ENTER + VG_SHIFT] = KEY_RETURN;

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

  translationTable[VG_NP_0 + VG_SHIFT] = '0';
  translationTable[VG_NP_1 + VG_SHIFT] = '1';
  translationTable[VG_NP_2 + VG_SHIFT] = '2';
  translationTable[VG_NP_3 + VG_SHIFT] = '3';
  translationTable[VG_NP_4 + VG_SHIFT] = '4';
  translationTable[VG_NP_5 + VG_SHIFT] = '5';
  translationTable[VG_NP_6 + VG_SHIFT] = '6';
  translationTable[VG_NP_7 + VG_SHIFT] = '7';
  translationTable[VG_NP_8 + VG_SHIFT] = '8';
  translationTable[VG_NP_9 + VG_SHIFT] = '9';
  translationTable[VG_NP_COMMA + VG_SHIFT] = ',';
  translationTable[VG_NP_DASH + VG_SHIFT] = '-';
  translationTable[VG_NP_DOT + VG_SHIFT] = '.';

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

  numLockTable[VG_NP_0 + VG_SHIFT] = KEY_INSERT;
  numLockTable[VG_NP_1 + VG_SHIFT] = KEY_END;
  numLockTable[VG_NP_2 + VG_SHIFT] = KEY_DOWN_ARROW;
  numLockTable[VG_NP_3 + VG_SHIFT] = KEY_PAGE_DOWN;
  numLockTable[VG_NP_4 + VG_SHIFT] = KEY_LEFT_ARROW;
  numLockTable[VG_NP_5 + VG_SHIFT] = '5';
  numLockTable[VG_NP_6 + VG_SHIFT] = KEY_RIGHT_ARROW;
  numLockTable[VG_NP_7 + VG_SHIFT] = KEY_HOME;
  numLockTable[VG_NP_8 + VG_SHIFT] = KEY_UP_ARROW;
  numLockTable[VG_NP_9 + VG_SHIFT] = KEY_PAGE_UP;
  numLockTable[VG_NP_DOT + VG_SHIFT] = KEY_DELETE;
}

/** Set up versions of this key with modifiers, mapping to the given IBM key. */
void setupKeyWModifiers(int vgKey, int ibmKey) {
  translationTable[vgKey] = ibmKey;
  translationTable[vgKey + VG_CTRL] = ibmKey;
  translationTable[vgKey + VG_SHIFT] = ibmKey;
  translationTable[vgKey + VG_SHIFT + VG_CTRL] = ibmKey;
}
