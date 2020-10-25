#include <Keyboard.h>

/* Vector Graphic keyboard keycodes. */
#define VG_TAB 9
#define VG_ESC 27
#define VG_BACKSPACE 8
#define VG_DEL 127
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
#define VG_NP_9 137
#define VG_NP_DASH 142
#define VG_NP_COMMA 143
#define VG_UP 138
#define VG_DOWN 139
#define VG_LEFT 140
#define VG_RIGHT 141
#define VG_NP_ENTER 160

#ifndef KEY_F12
// The Arduino UNO can't be used as a HID device, but in order
// to test the logic, we define some of the Windows keycodes here.

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
