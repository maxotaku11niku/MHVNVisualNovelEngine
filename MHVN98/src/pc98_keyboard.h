//PC-98 keyboard interface
#pragma once

#include "x86ports.h"

//Key code defines
#define key_down(k) k
#define key_up(k) (k | 0x80)

#define KEY_UP_MASK 0x80

#define KC_ESC            0x00
#define KC_1              0x01
#define KC_2              0x02
#define KC_3              0x03
#define KC_4              0x04
#define KC_5              0x05
#define KC_6              0x06
#define KC_7              0x07
#define KC_8              0x08
#define KC_9              0x09
#define KC_0              0x0A
#define KC_EQUALS         0x0B
#define KC_CARET          0x0C
#define KC_YEN            0x0D //equivalent to backslash
#define KC_BACKSPACE      0x0E
#define KC_TAB            0x0F
#define KC_Q              0x10
#define KC_W              0x11
#define KC_E              0x12
#define KC_R              0x13
#define KC_T              0x14
#define KC_Y              0x15
#define KC_U              0x16
#define KC_I              0x17
#define KC_O              0x18
#define KC_P              0x19
#define KC_AT             0x1A
#define KC_LSQUAREBRACKET 0x1B
#define KC_ENTER          0x1C
#define KC_A              0x1D
#define KC_S              0x1E
#define KC_D              0x1F
#define KC_F              0x20
#define KC_G              0x21
#define KC_H              0x22
#define KC_J              0x23
#define KC_K              0x24
#define KC_L              0x25
#define KC_SEMICOLON      0x26
#define KC_COLON          0x27
#define KC_RSQUAREBRACKET 0x28
#define KC_Z              0x29
#define KC_X              0x2A
#define KC_C              0x2B
#define KC_V              0x2C
#define KC_B              0x2D
#define KC_N              0x2E
#define KC_M              0x2F
#define KC_COMMA          0x30
#define KC_FULLSTOP       0x31
#define KC_SLASH          0x32
#define KC_MINUS          0x33
#define KC_SPACE          0x34
#define KC_XFER           0x35 //not found on other keyboards
#define KC_ROLLUP         0x36 //equivalent to PAGE DOWN
#define KC_ROLLDOWN       0x37 //equivalent to PAGE UP
#define KC_INS            0x38
#define KC_DEL            0x39
#define KC_UP             0x3A
#define KC_LEFT           0x3B
#define KC_RIGHT          0x3C
#define KC_DOWN           0x3D
#define KC_HOME           0x3E
#define KC_HELP           0x3F
#define KC_NUM_MINUS      0x40
#define KC_NUM_DIVIDE     0x41
#define KC_NUM_7          0x42
#define KC_NUM_8          0x43
#define KC_NUM_9          0x44
#define KC_NUM_MULTIPLY   0x45
#define KC_NUM_4          0x46
#define KC_NUM_5          0x47
#define KC_NUM_6          0x48
#define KC_NUM_PLUS       0x49
#define KC_NUM_1          0x4A
#define KC_NUM_2          0x4B
#define KC_NUM_3          0x4C
#define KC_NUM_EQUALS     0x4D
#define KC_NUM_0          0x4E
#define KC_NUM_COMMA      0x4F
#define KC_NUM_POINT      0x50
#define KC_NFER           0x51 //not found on other keyboards
#define KC_STOP           0x60 //not found on other keyboards
#define KC_COPY           0x61 //not found on other keyboards
#define KC_F1             0x62
#define KC_F2             0x63
#define KC_F3             0x64
#define KC_F4             0x65
#define KC_F5             0x66
#define KC_F6             0x67
#define KC_F7             0x68
#define KC_F8             0x69
#define KC_F9             0x6A
#define KC_F10            0x6B
#define KC_SHIFT          0x70
#define KC_CAPS           0x71
#define KC_KANA           0x72 //found on other Japanese keyboards
#define KC_GRAPH          0x73 //equivalent to ALT
#define KC_CTRL           0x74

#define KEY_BUFFER ((unsigned short*)0x0502) //Keyboard BIOS keycode buffer
#define KEY_CONVERSION_TABLE  (*((unsigned short**)0x0522)) //Keyboard BIOS conversion table offset pointer
#define KEY_BUFFER_HEAD  (*((unsigned short**)0x0524)) //Keyboard BIOS keycode buffer head offset
#define KEY_BUFFER_TAIL  (*((unsigned short**)0x0526)) //Keyboard BIOS keycode buffer tail offset
#define KEY_BUFFER_COUNT  (*((unsigned char*)0x0528)) //Keyboard BIOS keycode buffer count
#define KEY_ERRORRETRY  (*((unsigned char*)0x0529)) //Keyboard BIOS error retry flag
#define KEY_STATUS  ((unsigned char*)0x052A) //Keyboard BIOS key status array
#define KEY_MOD  (*((unsigned char*)0x053A)) //Keyboard BIOS modifier key flags

#define KEY_MOD_SHIFT 0x01
#define KEY_MOD_CAPS  0x02
#define KEY_MOD_KANA  0x04
#define KEY_MOD_GRPH  0x08
#define KEY_MOD_CTRL  0x10

//INPORT 41 - Read Key Data (put into 'data')
#define pc98k_readkey(data) inportb(0x41, data)