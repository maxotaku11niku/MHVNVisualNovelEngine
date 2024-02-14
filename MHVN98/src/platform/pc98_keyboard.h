//PC-98 keyboard interface
#pragma once

//#include "x86ports.h"
#include <dos.h>

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

#define KEY_BUFFER ((unsigned __far short*)0x0502) //Keyboard BIOS keycode buffer
#define KEY_CONVERSION_TABLE  (*((unsigned __far short**)0x0522)) //Keyboard BIOS conversion table offset pointer
#define KEY_BUFFER_HEAD  (*((unsigned __far short**)0x0524)) //Keyboard BIOS keycode buffer head offset
#define KEY_BUFFER_TAIL  (*((unsigned __far short**)0x0526)) //Keyboard BIOS keycode buffer tail offset
#define KEY_BUFFER_COUNT  (*((unsigned __far char*)0x0528)) //Keyboard BIOS keycode buffer count
#define KEY_ERRORRETRY  (*((unsigned __far char*)0x0529)) //Keyboard BIOS error retry flag
#define KEY_STATUS  ((unsigned __far char*)0x052A) //Keyboard BIOS key status array
#define KEY_MOD  (*((unsigned __far char*)0x053A)) //Keyboard BIOS modifier key flags

#define KEY_MOD_SHIFT 0x01
#define KEY_MOD_CAPS  0x02
#define KEY_MOD_KANA  0x04
#define KEY_MOD_GRPH  0x08
#define KEY_MOD_CTRL  0x10

//INPORT 41 - Read Key Data (put into 'data')
inline unsigned char PC98ReadKey()
{
    return inportb(0x41);
}
//#define pc98k_readkey(data) inportb(0x41, data)

extern unsigned char prevKeyStatus[16];
extern unsigned char keyChangeStatus[16];

//Key bit defines (use them in if statements)
#define K_ESC   	((*KEY_STATUS) & 0x01)
#define K_1     	((*KEY_STATUS) & 0x02)
#define K_2     	((*KEY_STATUS) & 0x04)
#define K_3     	((*KEY_STATUS) & 0x08)
#define K_4     	((*KEY_STATUS) & 0x10)
#define K_5     	((*KEY_STATUS) & 0x20)
#define K_6     	((*KEY_STATUS) & 0x40)
#define K_7     	((*KEY_STATUS) & 0x80)
#define K_8     	((*(KEY_STATUS + 0x1)) & 0x01)
#define K_9     	((*(KEY_STATUS + 0x1)) & 0x02)
#define K_0     	((*(KEY_STATUS + 0x1)) & 0x04)
#define K_EQUALS	((*(KEY_STATUS + 0x1)) & 0x08)
#define K_CARET 	((*(KEY_STATUS + 0x1)) & 0x10)
#define K_YEN   	((*(KEY_STATUS + 0x1)) & 0x20)//equivalent to backslash
#define K_BACKSPACE	((*(KEY_STATUS + 0x1)) & 0x40)
#define K_TAB   	((*(KEY_STATUS + 0x1)) & 0x80)
#define K_Q     	((*(KEY_STATUS + 0x2)) & 0x01)
#define K_W     	((*(KEY_STATUS + 0x2)) & 0x02)
#define K_E     	((*(KEY_STATUS + 0x2)) & 0x04)
#define K_R     	((*(KEY_STATUS + 0x2)) & 0x08)
#define K_T     	((*(KEY_STATUS + 0x2)) & 0x10)
#define K_Y     	((*(KEY_STATUS + 0x2)) & 0x20)
#define K_U     	((*(KEY_STATUS + 0x2)) & 0x40)
#define K_I     	((*(KEY_STATUS + 0x2)) & 0x10)
#define K_O     	((*(KEY_STATUS + 0x3)) & 0x01)
#define K_P     	((*(KEY_STATUS + 0x3)) & 0x02)
#define K_AT    	((*(KEY_STATUS + 0x3)) & 0x04)
#define K_LSQUAREBRACKET	((*(KEY_STATUS + 0x3)) & 0x08)
#define K_ENTER 	((*(KEY_STATUS + 0x3)) & 0x10)
#define K_A     	((*(KEY_STATUS + 0x3)) & 0x20)
#define K_S     	((*(KEY_STATUS + 0x3)) & 0x40)
#define K_D     	((*(KEY_STATUS + 0x3)) & 0x80)
#define K_F     	((*(KEY_STATUS + 0x4)) & 0x01)
#define K_G     	((*(KEY_STATUS + 0x4)) & 0x02)
#define K_H     	((*(KEY_STATUS + 0x4)) & 0x04)
#define K_J     	((*(KEY_STATUS + 0x4)) & 0x08)
#define K_K     	((*(KEY_STATUS + 0x4)) & 0x10)
#define K_L     	((*(KEY_STATUS + 0x4)) & 0x20)
#define K_SEMICOLON	((*(KEY_STATUS + 0x4)) & 0x40)
#define K_COLON 	((*(KEY_STATUS + 0x4)) & 0x80)
#define K_RSQUAREBRACKET	((*(KEY_STATUS + 0x5)) & 0x01)
#define K_Z     	((*(KEY_STATUS + 0x5)) & 0x02)
#define K_X     	((*(KEY_STATUS + 0x5)) & 0x04)
#define K_C     	((*(KEY_STATUS + 0x5)) & 0x08)
#define K_V     	((*(KEY_STATUS + 0x5)) & 0x10)
#define K_B     	((*(KEY_STATUS + 0x5)) & 0x20)
#define K_N     	((*(KEY_STATUS + 0x5)) & 0x40)
#define K_M     	((*(KEY_STATUS + 0x5)) & 0x80)
#define K_COMMA 	((*(KEY_STATUS + 0x6)) & 0x01)
#define K_FULLSTOP	((*(KEY_STATUS + 0x6)) & 0x02)
#define K_SLASH 	((*(KEY_STATUS + 0x6)) & 0x04)
#define K_MINUS 	((*(KEY_STATUS + 0x6)) & 0x08)
#define K_SPACE 	((*(KEY_STATUS + 0x6)) & 0x10)
#define K_XFER  	((*(KEY_STATUS + 0x6)) & 0x20) //not found on other keyboards
#define K_ROLLUP	((*(KEY_STATUS + 0x6)) & 0x40) //equivalent to PAGE DOWN
#define K_ROLLDOWN	((*(KEY_STATUS + 0x6)) & 0x80) //equivalent to PAGE UP
#define K_INS   	((*(KEY_STATUS + 0x7)) & 0x01)
#define K_DEL   	((*(KEY_STATUS + 0x7)) & 0x02)
#define K_UP    	((*(KEY_STATUS + 0x7)) & 0x04)
#define K_LEFT  	((*(KEY_STATUS + 0x7)) & 0x08)
#define K_RIGHT 	((*(KEY_STATUS + 0x7)) & 0x10)
#define K_DOWN  	((*(KEY_STATUS + 0x7)) & 0x20)
#define K_HOME  	((*(KEY_STATUS + 0x7)) & 0x40)
#define K_HELP  	((*(KEY_STATUS + 0x7)) & 0x80)
#define K_NUM_MINUS 	((*(KEY_STATUS + 0x8)) & 0x01)
#define K_NUM_DIVIDE	((*(KEY_STATUS + 0x8)) & 0x02)
#define K_NUM_7     	((*(KEY_STATUS + 0x8)) & 0x04)
#define K_NUM_8     	((*(KEY_STATUS + 0x8)) & 0x08)
#define K_NUM_9     	((*(KEY_STATUS + 0x8)) & 0x10)
#define K_NUM_MULTIPLY	((*(KEY_STATUS + 0x8)) & 0x20)
#define K_NUM_4     	((*(KEY_STATUS + 0x8)) & 0x40)
#define K_NUM_5     	((*(KEY_STATUS + 0x8)) & 0x80)
#define K_NUM_6     	((*(KEY_STATUS + 0x9)) & 0x01)
#define K_NUM_PLUS  	((*(KEY_STATUS + 0x9)) & 0x02)
#define K_NUM_1     	((*(KEY_STATUS + 0x9)) & 0x04)
#define K_NUM_2     	((*(KEY_STATUS + 0x9)) & 0x08)
#define K_NUM_3     	((*(KEY_STATUS + 0x9)) & 0x10)
#define K_NUM_EQUALS	((*(KEY_STATUS + 0x9)) & 0x20)
#define K_NUM_0     	((*(KEY_STATUS + 0x9)) & 0x40)
#define K_NUM_COMMA 	((*(KEY_STATUS + 0x9)) & 0x80)
#define K_NUM_POINT 	((*(KEY_STATUS + 0xA)) & 0x01)
#define K_NFER      	((*(KEY_STATUS + 0xA)) & 0x02) //not found on other keyboards
#define K_STOP	((*(KEY_STATUS + 0xC)) & 0x01) //not found on other keyboards
#define K_COPY	((*(KEY_STATUS + 0xC)) & 0x02) //not found on other keyboards
#define K_F1	((*(KEY_STATUS + 0xC)) & 0x04)
#define K_F2	((*(KEY_STATUS + 0xC)) & 0x08)
#define K_F3	((*(KEY_STATUS + 0xC)) & 0x10)
#define K_F4	((*(KEY_STATUS + 0xC)) & 0x20)
#define K_F5	((*(KEY_STATUS + 0xC)) & 0x40)
#define K_F6	((*(KEY_STATUS + 0xC)) & 0x80)
#define K_F7	((*(KEY_STATUS + 0xD)) & 0x01)
#define K_F8	((*(KEY_STATUS + 0xD)) & 0x02)
#define K_F9	((*(KEY_STATUS + 0xD)) & 0x04)
#define K_F10	((*(KEY_STATUS + 0xD)) & 0x08)
#define K_SHIFT	((*(KEY_STATUS + 0xE)) & 0x01)
#define K_CAPS	((*(KEY_STATUS + 0xE)) & 0x02)
#define K_KANA	((*(KEY_STATUS + 0xE)) & 0x04) //found on other Japanese keyboards
#define K_GRAPH	((*(KEY_STATUS + 0xE)) & 0x08) //equivalent to ALT
#define K_CTRL	((*(KEY_STATUS + 0xE)) & 0x10)

//Previous key bit defines (use them in if statements)
#define PK_ESC   	((*prevKeyStatus) & 0x01)
#define PK_1     	((*prevKeyStatus) & 0x02)
#define PK_2     	((*prevKeyStatus) & 0x04)
#define PK_3     	((*prevKeyStatus) & 0x08)
#define PK_4     	((*prevKeyStatus) & 0x10)
#define PK_5     	((*prevKeyStatus) & 0x20)
#define PK_6     	((*prevKeyStatus) & 0x40)
#define PK_7     	((*prevKeyStatus) & 0x80)
#define PK_8     	((*(prevKeyStatus + 0x1)) & 0x01)
#define PK_9     	((*(prevKeyStatus + 0x1)) & 0x02)
#define PK_0     	((*(prevKeyStatus + 0x1)) & 0x04)
#define PK_EQUALS	((*(prevKeyStatus + 0x1)) & 0x08)
#define PK_CARET 	((*(prevKeyStatus + 0x1)) & 0x10)
#define PK_YEN   	((*(prevKeyStatus + 0x1)) & 0x20)//equivalent to backslash
#define PK_BACKSPACE	((*(prevKeyStatus + 0x1)) & 0x40)
#define PK_TAB   	((*(prevKeyStatus + 0x1)) & 0x80)
#define PK_Q     	((*(prevKeyStatus + 0x2)) & 0x01)
#define PK_W     	((*(prevKeyStatus + 0x2)) & 0x02)
#define PK_E     	((*(prevKeyStatus + 0x2)) & 0x04)
#define PK_R     	((*(prevKeyStatus + 0x2)) & 0x08)
#define PK_T     	((*(prevKeyStatus + 0x2)) & 0x10)
#define PK_Y     	((*(prevKeyStatus + 0x2)) & 0x20)
#define PK_U     	((*(prevKeyStatus + 0x2)) & 0x40)
#define PK_I     	((*(prevKeyStatus + 0x2)) & 0x10)
#define PK_O     	((*(prevKeyStatus + 0x3)) & 0x01)
#define PK_P     	((*(prevKeyStatus + 0x3)) & 0x02)
#define PK_AT    	((*(prevKeyStatus + 0x3)) & 0x04)
#define PK_LSQUAREBRACKET	((*(prevKeyStatus + 0x3)) & 0x08)
#define PK_ENTER 	((*(prevKeyStatus + 0x3)) & 0x10)
#define PK_A     	((*(prevKeyStatus + 0x3)) & 0x20)
#define PK_S     	((*(prevKeyStatus + 0x3)) & 0x40)
#define PK_D     	((*(prevKeyStatus + 0x3)) & 0x80)
#define PK_F     	((*(prevKeyStatus + 0x4)) & 0x01)
#define PK_G     	((*(prevKeyStatus + 0x4)) & 0x02)
#define PK_H     	((*(prevKeyStatus + 0x4)) & 0x04)
#define PK_J     	((*(prevKeyStatus + 0x4)) & 0x08)
#define PK_K     	((*(prevKeyStatus + 0x4)) & 0x10)
#define PK_L     	((*(prevKeyStatus + 0x4)) & 0x20)
#define PK_SEMICOLON	((*(prevKeyStatus + 0x4)) & 0x40)
#define PK_COLON 	((*(prevKeyStatus + 0x4)) & 0x80)
#define PK_RSQUAREBRACKET	((*(prevKeyStatus + 0x5)) & 0x01)
#define PK_Z     	((*(prevKeyStatus + 0x5)) & 0x02)
#define PK_X     	((*(prevKeyStatus + 0x5)) & 0x04)
#define PK_C     	((*(prevKeyStatus + 0x5)) & 0x08)
#define PK_V     	((*(prevKeyStatus + 0x5)) & 0x10)
#define PK_B     	((*(prevKeyStatus + 0x5)) & 0x20)
#define PK_N     	((*(prevKeyStatus + 0x5)) & 0x40)
#define PK_M     	((*(prevKeyStatus + 0x5)) & 0x80)
#define PK_COMMA 	((*(prevKeyStatus + 0x6)) & 0x01)
#define PK_FULLSTOP	((*(prevKeyStatus + 0x6)) & 0x02)
#define PK_SLASH 	((*(prevKeyStatus + 0x6)) & 0x04)
#define PK_MINUS 	((*(prevKeyStatus + 0x6)) & 0x08)
#define PK_SPACE 	((*(prevKeyStatus + 0x6)) & 0x10)
#define PK_XFER  	((*(prevKeyStatus + 0x6)) & 0x20) //not found on other keyboards
#define PK_ROLLUP	((*(prevKeyStatus + 0x6)) & 0x40) //equivalent to PAGE DOWN
#define PK_ROLLDOWN	((*(prevKeyStatus + 0x6)) & 0x80) //equivalent to PAGE UP
#define PK_INS   	((*(prevKeyStatus + 0x7)) & 0x01)
#define PK_DEL   	((*(prevKeyStatus + 0x7)) & 0x02)
#define PK_UP    	((*(prevKeyStatus + 0x7)) & 0x04)
#define PK_LEFT  	((*(prevKeyStatus + 0x7)) & 0x08)
#define PK_RIGHT 	((*(prevKeyStatus + 0x7)) & 0x10)
#define PK_DOWN  	((*(prevKeyStatus + 0x7)) & 0x20)
#define PK_HOME  	((*(prevKeyStatus + 0x7)) & 0x40)
#define PK_HELP  	((*(prevKeyStatus + 0x7)) & 0x80)
#define PK_NUM_MINUS 	((*(prevKeyStatus + 0x8)) & 0x01)
#define PK_NUM_DIVIDE	((*(prevKeyStatus + 0x8)) & 0x02)
#define PK_NUM_7     	((*(prevKeyStatus + 0x8)) & 0x04)
#define PK_NUM_8     	((*(prevKeyStatus + 0x8)) & 0x08)
#define PK_NUM_9     	((*(prevKeyStatus + 0x8)) & 0x10)
#define PK_NUM_MULTIPLY	((*(prevKeyStatus + 0x8)) & 0x20)
#define PK_NUM_4     	((*(prevKeyStatus + 0x8)) & 0x40)
#define PK_NUM_5     	((*(prevKeyStatus + 0x8)) & 0x80)
#define PK_NUM_6     	((*(prevKeyStatus + 0x9)) & 0x01)
#define PK_NUM_PLUS  	((*(prevKeyStatus + 0x9)) & 0x02)
#define PK_NUM_1     	((*(prevKeyStatus + 0x9)) & 0x04)
#define PK_NUM_2     	((*(prevKeyStatus + 0x9)) & 0x08)
#define PK_NUM_3     	((*(prevKeyStatus + 0x9)) & 0x10)
#define PK_NUM_EQUALS	((*(prevKeyStatus + 0x9)) & 0x20)
#define PK_NUM_0     	((*(prevKeyStatus + 0x9)) & 0x40)
#define PK_NUM_COMMA 	((*(prevKeyStatus + 0x9)) & 0x80)
#define PK_NUM_POINT 	((*(prevKeyStatus + 0xA)) & 0x01)
#define PK_NFER      	((*(prevKeyStatus + 0xA)) & 0x02) //not found on other keyboards
#define PK_STOP	((*(prevKeyStatus + 0xC)) & 0x01) //not found on other keyboards
#define PK_COPY	((*(prevKeyStatus + 0xC)) & 0x02) //not found on other keyboards
#define PK_F1	((*(prevKeyStatus + 0xC)) & 0x04)
#define PK_F2	((*(prevKeyStatus + 0xC)) & 0x08)
#define PK_F3	((*(prevKeyStatus + 0xC)) & 0x10)
#define PK_F4	((*(prevKeyStatus + 0xC)) & 0x20)
#define PK_F5	((*(prevKeyStatus + 0xC)) & 0x40)
#define PK_F6	((*(prevKeyStatus + 0xC)) & 0x80)
#define PK_F7	((*(prevKeyStatus + 0xD)) & 0x01)
#define PK_F8	((*(prevKeyStatus + 0xD)) & 0x02)
#define PK_F9	((*(prevKeyStatus + 0xD)) & 0x04)
#define PK_F10	((*(prevKeyStatus + 0xD)) & 0x08)
#define PK_SHIFT	((*(prevKeyStatus + 0xE)) & 0x01)
#define PK_CAPS	((*(prevKeyStatus + 0xE)) & 0x02)
#define PK_KANA	((*(prevKeyStatus + 0xE)) & 0x04) //found on other Japanese keyboards
#define PK_GRAPH	((*(prevKeyStatus + 0xE)) & 0x08) //equivalent to ALT
#define PK_CTRL	((*(prevKeyStatus + 0xE)) & 0x10)

//Key change bit defines (use them in if statements)
#define DK_ESC   	((*keyChangeStatus) & 0x01)
#define DK_1     	((*keyChangeStatus) & 0x02)
#define DK_2     	((*keyChangeStatus) & 0x04)
#define DK_3     	((*keyChangeStatus) & 0x08)
#define DK_4     	((*keyChangeStatus) & 0x10)
#define DK_5     	((*keyChangeStatus) & 0x20)
#define DK_6     	((*keyChangeStatus) & 0x40)
#define DK_7     	((*keyChangeStatus) & 0x80)
#define DK_8     	((*(keyChangeStatus + 0x1)) & 0x01)
#define DK_9     	((*(keyChangeStatus + 0x1)) & 0x02)
#define DK_0     	((*(keyChangeStatus + 0x1)) & 0x04)
#define DK_EQUALS	((*(keyChangeStatus + 0x1)) & 0x08)
#define DK_CARET 	((*(keyChangeStatus + 0x1)) & 0x10)
#define DK_YEN   	((*(keyChangeStatus + 0x1)) & 0x20)//equivalent to backslash
#define DK_BACKSPACE	((*(keyChangeStatus + 0x1)) & 0x40)
#define DK_TAB   	((*(keyChangeStatus + 0x1)) & 0x80)
#define DK_Q     	((*(keyChangeStatus + 0x2)) & 0x01)
#define DK_W     	((*(keyChangeStatus + 0x2)) & 0x02)
#define DK_E     	((*(keyChangeStatus + 0x2)) & 0x04)
#define DK_R     	((*(keyChangeStatus + 0x2)) & 0x08)
#define DK_T     	((*(keyChangeStatus + 0x2)) & 0x10)
#define DK_Y     	((*(keyChangeStatus + 0x2)) & 0x20)
#define DK_U     	((*(keyChangeStatus + 0x2)) & 0x40)
#define DK_I     	((*(keyChangeStatus + 0x2)) & 0x10)
#define DK_O     	((*(keyChangeStatus + 0x3)) & 0x01)
#define DK_P     	((*(keyChangeStatus + 0x3)) & 0x02)
#define DK_AT    	((*(keyChangeStatus + 0x3)) & 0x04)
#define DK_LSQUAREBRACKET	((*(keyChangeStatus + 0x3)) & 0x08)
#define DK_ENTER 	((*(keyChangeStatus + 0x3)) & 0x10)
#define DK_A     	((*(keyChangeStatus + 0x3)) & 0x20)
#define DK_S     	((*(keyChangeStatus + 0x3)) & 0x40)
#define DK_D     	((*(keyChangeStatus + 0x3)) & 0x80)
#define DK_F     	((*(keyChangeStatus + 0x4)) & 0x01)
#define DK_G     	((*(keyChangeStatus + 0x4)) & 0x02)
#define DK_H     	((*(keyChangeStatus + 0x4)) & 0x04)
#define DK_J     	((*(keyChangeStatus + 0x4)) & 0x08)
#define DK_K     	((*(keyChangeStatus + 0x4)) & 0x10)
#define DK_L     	((*(keyChangeStatus + 0x4)) & 0x20)
#define DK_SEMICOLON	((*(keyChangeStatus + 0x4)) & 0x40)
#define DK_COLON 	((*(keyChangeStatus + 0x4)) & 0x80)
#define DK_RSQUAREBRACKET	((*(keyChangeStatus + 0x5)) & 0x01)
#define DK_Z     	((*(keyChangeStatus + 0x5)) & 0x02)
#define DK_X     	((*(keyChangeStatus + 0x5)) & 0x04)
#define DK_C     	((*(keyChangeStatus + 0x5)) & 0x08)
#define DK_V     	((*(keyChangeStatus + 0x5)) & 0x10)
#define DK_B     	((*(keyChangeStatus + 0x5)) & 0x20)
#define DK_N     	((*(keyChangeStatus + 0x5)) & 0x40)
#define DK_M     	((*(keyChangeStatus + 0x5)) & 0x80)
#define DK_COMMA 	((*(keyChangeStatus + 0x6)) & 0x01)
#define DK_FULLSTOP	((*(keyChangeStatus + 0x6)) & 0x02)
#define DK_SLASH 	((*(keyChangeStatus + 0x6)) & 0x04)
#define DK_MINUS 	((*(keyChangeStatus + 0x6)) & 0x08)
#define DK_SPACE 	((*(keyChangeStatus + 0x6)) & 0x10)
#define DK_XFER  	((*(keyChangeStatus + 0x6)) & 0x20) //not found on other keyboards
#define DK_ROLLUP	((*(keyChangeStatus + 0x6)) & 0x40) //equivalent to PAGE DOWN
#define DK_ROLLDOWN	((*(keyChangeStatus + 0x6)) & 0x80) //equivalent to PAGE UP
#define DK_INS   	((*(keyChangeStatus + 0x7)) & 0x01)
#define DK_DEL   	((*(keyChangeStatus + 0x7)) & 0x02)
#define DK_UP    	((*(keyChangeStatus + 0x7)) & 0x04)
#define DK_LEFT  	((*(keyChangeStatus + 0x7)) & 0x08)
#define DK_RIGHT 	((*(keyChangeStatus + 0x7)) & 0x10)
#define DK_DOWN  	((*(keyChangeStatus + 0x7)) & 0x20)
#define DK_HOME  	((*(keyChangeStatus + 0x7)) & 0x40)
#define DK_HELP  	((*(keyChangeStatus + 0x7)) & 0x80)
#define DK_NUM_MINUS 	((*(keyChangeStatus + 0x8)) & 0x01)
#define DK_NUM_DIVIDE	((*(keyChangeStatus + 0x8)) & 0x02)
#define DK_NUM_7     	((*(keyChangeStatus + 0x8)) & 0x04)
#define DK_NUM_8     	((*(keyChangeStatus + 0x8)) & 0x08)
#define DK_NUM_9     	((*(keyChangeStatus + 0x8)) & 0x10)
#define DK_NUM_MULTIPLY	((*(keyChangeStatus + 0x8)) & 0x20)
#define DK_NUM_4     	((*(keyChangeStatus + 0x8)) & 0x40)
#define DK_NUM_5     	((*(keyChangeStatus + 0x8)) & 0x80)
#define DK_NUM_6     	((*(keyChangeStatus + 0x9)) & 0x01)
#define DK_NUM_PLUS  	((*(keyChangeStatus + 0x9)) & 0x02)
#define DK_NUM_1     	((*(keyChangeStatus + 0x9)) & 0x04)
#define DK_NUM_2     	((*(keyChangeStatus + 0x9)) & 0x08)
#define DK_NUM_3     	((*(keyChangeStatus + 0x9)) & 0x10)
#define DK_NUM_EQUALS	((*(keyChangeStatus + 0x9)) & 0x20)
#define DK_NUM_0     	((*(keyChangeStatus + 0x9)) & 0x40)
#define DK_NUM_COMMA 	((*(keyChangeStatus + 0x9)) & 0x80)
#define DK_NUM_POINT 	((*(keyChangeStatus + 0xA)) & 0x01)
#define DK_NFER      	((*(keyChangeStatus + 0xA)) & 0x02) //not found on other keyboards
#define DK_STOP	((*(keyChangeStatus + 0xC)) & 0x01) //not found on other keyboards
#define DK_COPY	((*(keyChangeStatus + 0xC)) & 0x02) //not found on other keyboards
#define DK_F1	((*(keyChangeStatus + 0xC)) & 0x04)
#define DK_F2	((*(keyChangeStatus + 0xC)) & 0x08)
#define DK_F3	((*(keyChangeStatus + 0xC)) & 0x10)
#define DK_F4	((*(keyChangeStatus + 0xC)) & 0x20)
#define DK_F5	((*(keyChangeStatus + 0xC)) & 0x40)
#define DK_F6	((*(keyChangeStatus + 0xC)) & 0x80)
#define DK_F7	((*(keyChangeStatus + 0xD)) & 0x01)
#define DK_F8	((*(keyChangeStatus + 0xD)) & 0x02)
#define DK_F9	((*(keyChangeStatus + 0xD)) & 0x04)
#define DK_F10	((*(keyChangeStatus + 0xD)) & 0x08)
#define DK_SHIFT	((*(keyChangeStatus + 0xE)) & 0x01)
#define DK_CAPS	((*(keyChangeStatus + 0xE)) & 0x02)
#define DK_KANA	((*(keyChangeStatus + 0xE)) & 0x04) //found on other Japanese keyboards
#define DK_GRAPH	((*(keyChangeStatus + 0xE)) & 0x08) //equivalent to ALT
#define DK_CTRL	((*(keyChangeStatus + 0xE)) & 0x10)

#define key_is_down(k) (k)
#define key_pressed(k) ((k) && (D##k))
#define key_released(k) ((!k) && (D##k))

void UpdatePrevKeyStatus();
