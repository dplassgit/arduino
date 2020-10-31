#include <Keyboard.h>
#include "vg_keys.h"

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
  // Windows key
  translationTable[VG_LINEFEED] = KEY_LEFT_GUI;
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
