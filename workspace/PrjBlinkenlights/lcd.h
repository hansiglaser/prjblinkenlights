/*
 * lcd.h
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include "utils.h"
#include "iodef.h"

/*
 *
 * Display Address Map (in Hex) (20 Chars x 4 Rows)
 *
 *  Row |  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
 *  ----+------------------------------------------------------------
 *    1 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13
 *    2 | 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53
 *    3 | 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27
 *    4 | 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60 61 62 63 64 65 66 67
 *
 * So, the 3rd row is the continuation of the 1st row, and the 4th row is the
 * continuation of the 2nd row.
 *
 * The above table is valid for the TC2004A as well as the TC1604A. The latter
 * device just doesn't display columns 17..20.
 *
 */

// We have an SPLC780D-01 chip, for the character table see p. 32 of SPLC780D.pdf.
#define auml  "\341"    // 0xE1 = 11100001 = 0341
#define ouml  "\358"    // 0xEF = 11101111 = 0357
#define uuml  "\365"    // 0xF5 = 11110101 = 0365
#define szlig "\342"    // 0xD2 = 11100010 = 0342
#define larr  '\177'    // 0x7F = 01111111 = 0277
#define rarr  '\176'    // 0x7E = 01111110 = 0276
#define deg   '\337'    // 0xDF = 11011111 = 0337
// damn, C doesn't support this for strings and chars equally :-( -> dirty solution

#define LCD_CMD_CLEAR_DISPLAY    0x01
#define LCD_CMD_RETURN_HOME      0x02
#define LCD_CMD_SET_ENTRY_MODE   0x04  // logical or LCD_ENTRY_MODE_* constants
#define LCD_CMD_DISPLAY_CONTOL   0x08  // logical or LCD_DISPLAY_CONTROL_* constants
#define LCD_CMD_SHIFT            0x10  // logical or LCD_SHIFT_* constants
#define LCD_CMD_SET_FUNCTION     0x20  // logical or LCD_FUNCTION_* constants
#define LCD_CMD_SET_CGRAM_ADDR   0x40  // logical or address 0x00-0x3F
#define LCD_CMD_SET_DDRAM_ADDR   0x80  // logical or address 0x00-0x7F

#define LCD_ENTRY_MODE_SHIFT     0x01   // shift the LCD on new characters
#define LCD_ENTRY_MODE_FIXED     0x00   // don't shift the LCD
#define LCD_ENTRY_MODE_INC       0x02   // increment address counter on new characters
#define LCD_ENTRY_MODE_DEC       0x00   // decrement address counter

#define LCD_DISPLAY_CONTROL_DISPLAY_ON   0x04
#define LCD_DISPLAY_CONTROL_DISPLAY_OFF  0x00
#define LCD_DISPLAY_CONTROL_CURSOR_ON    0x02
#define LCD_DISPLAY_CONTROL_CURSOR_OFF   0x00
#define LCD_DISPLAY_CONTROL_BLINK_ON     0x01
#define LCD_DISPLAY_CONTROL_BLINK_OFF    0x00

#define LCD_SHIFT_CURSOR_LEFT    0x00
#define LCD_SHIFT_CURSOR_RIGHT   0x04
#define LCD_SHIFT_DISPLAY_LEFT   0x08
#define LCD_SHIFT_DISPLAY_RIGHT  0x0C

#define LCD_FUNCTION_8BIT        0x10
#define LCD_FUNCTION_4BIT        0x00
#define LCD_FUNCTION_ONE_LINE    0x00
#define LCD_FUNCTION_TWO_LINES   0x08
#define LCD_FUNCTION_5X8         0x00
#define LCD_FUNCTION_5X10        0x04

#define LCD_DDRAM_ADDR(x,y)    (x + ((y & 0x01)?0x40:0x00) + ((y & 0x02)?0x14:0x00))

void LCDWrite(uint8_t Ctrl, uint8_t Data);
void LCDWriteString(const char* St);
void LCDWriteBCD(uint16_t BCD);

static inline void LCDGoto(uint8_t AddrCnt) {
  LCDWrite(0,LCD_CMD_SET_DDRAM_ADDR | AddrCnt);
}

void LCDGotoXY(uint8_t X, uint8_t Y);

static inline void LCDClearScreen() {
  LCDWrite(0,LCD_CMD_CLEAR_DISPLAY);
  delay_ms(2);
}

uint8_t LCDRead(uint8_t Ctrl);

void LCDInit();

#endif /* LCD_H_ */
