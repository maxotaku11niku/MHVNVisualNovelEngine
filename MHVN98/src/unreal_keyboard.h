#include "unrealhwaddr.h"
#pragma once

extern unsigned char prevKeyStatus[16];
extern unsigned char keyChangeStatus[16];

//Key bit defines (use them in if statements)
#define K_ESC   	((*key_status_relptr) & 0x01)
#define K_1     	((*key_status_relptr) & 0x02)
#define K_2     	((*key_status_relptr) & 0x04)
#define K_3     	((*key_status_relptr) & 0x08)
#define K_4     	((*key_status_relptr) & 0x10)
#define K_5     	((*key_status_relptr) & 0x20)
#define K_6     	((*key_status_relptr) & 0x40)
#define K_7     	((*key_status_relptr) & 0x80)
#define K_8     	((*(key_status_relptr + 0x1)) & 0x01)
#define K_9     	((*(key_status_relptr + 0x1)) & 0x02)
#define K_0     	((*(key_status_relptr + 0x1)) & 0x04)
#define K_EQUALS	((*(key_status_relptr + 0x1)) & 0x08)
#define K_CARET 	((*(key_status_relptr + 0x1)) & 0x10)
#define K_YEN   	((*(key_status_relptr + 0x1)) & 0x20)//equivalent to backslash
#define K_BACKSPACE	((*(key_status_relptr + 0x1)) & 0x40)
#define K_TAB   	((*(key_status_relptr + 0x1)) & 0x80)
#define K_Q     	((*(key_status_relptr + 0x2)) & 0x01)
#define K_W     	((*(key_status_relptr + 0x2)) & 0x02)
#define K_E     	((*(key_status_relptr + 0x2)) & 0x04)
#define K_R     	((*(key_status_relptr + 0x2)) & 0x08)
#define K_T     	((*(key_status_relptr + 0x2)) & 0x10)
#define K_Y     	((*(key_status_relptr + 0x2)) & 0x20)
#define K_U     	((*(key_status_relptr + 0x2)) & 0x40)
#define K_I     	((*(key_status_relptr + 0x2)) & 0x10)
#define K_O     	((*(key_status_relptr + 0x3)) & 0x01)
#define K_P     	((*(key_status_relptr + 0x3)) & 0x02)
#define K_AT    	((*(key_status_relptr + 0x3)) & 0x04)
#define K_LSQUAREBRACKET	((*(key_status_relptr + 0x3)) & 0x08)
#define K_ENTER 	((*(key_status_relptr + 0x3)) & 0x10)
#define K_A     	((*(key_status_relptr + 0x3)) & 0x20)
#define K_S     	((*(key_status_relptr + 0x3)) & 0x40)
#define K_D     	((*(key_status_relptr + 0x3)) & 0x80)
#define K_F     	((*(key_status_relptr + 0x4)) & 0x01)
#define K_G     	((*(key_status_relptr + 0x4)) & 0x02)
#define K_H     	((*(key_status_relptr + 0x4)) & 0x04)
#define K_J     	((*(key_status_relptr + 0x4)) & 0x08)
#define K_K     	((*(key_status_relptr + 0x4)) & 0x10)
#define K_L     	((*(key_status_relptr + 0x4)) & 0x20)
#define K_SEMICOLON	((*(key_status_relptr + 0x4)) & 0x40)
#define K_COLON 	((*(key_status_relptr + 0x4)) & 0x80)
#define K_RSQUAREBRACKET	((*(key_status_relptr + 0x5)) & 0x01)
#define K_Z     	((*(key_status_relptr + 0x5)) & 0x02)
#define K_X     	((*(key_status_relptr + 0x5)) & 0x04)
#define K_C     	((*(key_status_relptr + 0x5)) & 0x08)
#define K_V     	((*(key_status_relptr + 0x5)) & 0x10)
#define K_B     	((*(key_status_relptr + 0x5)) & 0x20)
#define K_N     	((*(key_status_relptr + 0x5)) & 0x40)
#define K_M     	((*(key_status_relptr + 0x5)) & 0x80)
#define K_COMMA 	((*(key_status_relptr + 0x6)) & 0x01)
#define K_FULLSTOP	((*(key_status_relptr + 0x6)) & 0x02)
#define K_SLASH 	((*(key_status_relptr + 0x6)) & 0x04)
#define K_MINUS 	((*(key_status_relptr + 0x6)) & 0x08)
#define K_SPACE 	((*(key_status_relptr + 0x6)) & 0x10)
#define K_XFER  	((*(key_status_relptr + 0x6)) & 0x20) //not found on other keyboards
#define K_ROLLUP	((*(key_status_relptr + 0x6)) & 0x40) //equivalent to PAGE DOWN
#define K_ROLLDOWN	((*(key_status_relptr + 0x6)) & 0x80) //equivalent to PAGE UP
#define K_INS   	((*(key_status_relptr + 0x7)) & 0x01)
#define K_DEL   	((*(key_status_relptr + 0x7)) & 0x02)
#define K_UP    	((*(key_status_relptr + 0x7)) & 0x04)
#define K_LEFT  	((*(key_status_relptr + 0x7)) & 0x08)
#define K_RIGHT 	((*(key_status_relptr + 0x7)) & 0x10)
#define K_DOWN  	((*(key_status_relptr + 0x7)) & 0x20)
#define K_HOME  	((*(key_status_relptr + 0x7)) & 0x40)
#define K_HELP  	((*(key_status_relptr + 0x7)) & 0x80)
#define K_NUM_MINUS 	((*(key_status_relptr + 0x8)) & 0x01)
#define K_NUM_DIVIDE	((*(key_status_relptr + 0x8)) & 0x02)
#define K_NUM_7     	((*(key_status_relptr + 0x8)) & 0x04)
#define K_NUM_8     	((*(key_status_relptr + 0x8)) & 0x08)
#define K_NUM_9     	((*(key_status_relptr + 0x8)) & 0x10)
#define K_NUM_MULTIPLY	((*(key_status_relptr + 0x8)) & 0x20)
#define K_NUM_4     	((*(key_status_relptr + 0x8)) & 0x40)
#define K_NUM_5     	((*(key_status_relptr + 0x8)) & 0x80)
#define K_NUM_6     	((*(key_status_relptr + 0x9)) & 0x01)
#define K_NUM_PLUS  	((*(key_status_relptr + 0x9)) & 0x02)
#define K_NUM_1     	((*(key_status_relptr + 0x9)) & 0x04)
#define K_NUM_2     	((*(key_status_relptr + 0x9)) & 0x08)
#define K_NUM_3     	((*(key_status_relptr + 0x9)) & 0x10)
#define K_NUM_EQUALS	((*(key_status_relptr + 0x9)) & 0x20)
#define K_NUM_0     	((*(key_status_relptr + 0x9)) & 0x40)
#define K_NUM_COMMA 	((*(key_status_relptr + 0x9)) & 0x80)
#define K_NUM_POINT 	((*(key_status_relptr + 0xA)) & 0x01)
#define K_NFER      	((*(key_status_relptr + 0xA)) & 0x02) //not found on other keyboards
#define K_STOP	((*(key_status_relptr + 0xC)) & 0x01) //not found on other keyboards
#define K_COPY	((*(key_status_relptr + 0xC)) & 0x02) //not found on other keyboards
#define K_F1	((*(key_status_relptr + 0xC)) & 0x04)
#define K_F2	((*(key_status_relptr + 0xC)) & 0x08)
#define K_F3	((*(key_status_relptr + 0xC)) & 0x10)
#define K_F4	((*(key_status_relptr + 0xC)) & 0x20)
#define K_F5	((*(key_status_relptr + 0xC)) & 0x40)
#define K_F6	((*(key_status_relptr + 0xC)) & 0x80)
#define K_F7	((*(key_status_relptr + 0xD)) & 0x01)
#define K_F8	((*(key_status_relptr + 0xD)) & 0x02)
#define K_F9	((*(key_status_relptr + 0xD)) & 0x04)
#define K_F10	((*(key_status_relptr + 0xD)) & 0x08)
#define K_SHIFT	((*(key_status_relptr + 0xE)) & 0x01)
#define K_CAPS	((*(key_status_relptr + 0xE)) & 0x02)
#define K_KANA	((*(key_status_relptr + 0xE)) & 0x04) //found on other Japanese keyboards
#define K_GRAPH	((*(key_status_relptr + 0xE)) & 0x08) //equivalent to ALT
#define K_CTRL	((*(key_status_relptr + 0xE)) & 0x10)

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

void updatePrevKeyStatus();