/*
 * lcd.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#include <msp430g2553.h>
#include "lcd.h"

/**
 * Write out one nibble to the LCD display
 *
 * @param Ctrl    either LCD_RS or 0
 * @param Nibble  properly formatted data to be written
 */
inline void LCDWriteNibble(uint8_t Ctrl, uint8_t Nibble) {
  // First assert data signals
  LCD_DATA_OUT = (LCD_DATA_OUT & ~LCD_DATA) | Nibble;
  // assert control signals
  LCD_CTRL_OUT = (LCD_CTRL_OUT & ~LCD_CTRL) | Ctrl;
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
}

/**
 * Write out one byte to the LCD display
 *
 * @param Ctrl  either LCD_RS or 0
 * @param Data  data byte
 */
void LCDWrite(uint8_t Ctrl, uint8_t Data) {
  int i;
  // assert control signals
  LCD_CTRL_OUT = (LCD_CTRL_OUT & ~LCD_CTRL) | Ctrl;
  // MSB: assert data signals
  LCD_DATA_OUT = (LCD_DATA_OUT & ~LCD_DATA) | LCD_DATA_MSB(Data);
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  for (i=0;i < 100;i++);
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
  for (i=0;i < 100;i++);
  // MSB: assert data signals
  LCD_DATA_OUT = (LCD_DATA_OUT & ~LCD_DATA) | LCD_DATA_LSB(Data);
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  for (i=0;i < 100;i++);
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
  for (i=0;i < 100;i++);
}

void LCDWriteString(const char* St) {
  while (*St) {
    LCDWrite(LCD_RS,*St++);
  }
}

void LCDGotoXY(uint8_t X, uint8_t Y) {
  LCDGoto(LCD_DDRAM_ADDR(X,Y));
}

uint8_t LCDRead(uint8_t Ctrl) {
  // set direction of D7-D4 pins to input
  LCD_DATA_DIR &= ~(LCD_DATA);
  // assert control signals to indicate read
  LCD_CTRL_OUT = (LCD_CTRL_OUT & ~LCD_CTRL) | LCD_RW | Ctrl;
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  // read MSB
  Ctrl = LCD_DATA_TO_MSB(LCD_DATA_IN);  // use Ctrl as dummy variable
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  // read LSB
  Ctrl |= LCD_DATA_TO_LSB(LCD_DATA_IN);
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
  // assert control signals to indicate write
  LCD_CTRL_OUT = (LCD_CTRL_OUT & ~LCD_CTRL);
  // set direction of D7-D4 pins to output
  LCD_DATA_DIR |= LCD_DATA;

  return Ctrl;
}

void LCDInit() {
  // wait for more than 15 ms
  delay_ms(15);
  // Set Function: 8 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_8BIT));
  // wait for more than 4.1 ms
  delay_ms(4);
  // Set Function: 8 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_8BIT));
  // wait for more than 100us
  delay_ms(1);
  // Set Function: 8 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_8BIT));
  delay_ms(1);
  // Set Function: 4 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_4BIT));
  delay_ms(1);
  // Set Function: 4 bit interface, two lines, 5x8 characters
  LCDWrite(0,LCD_CMD_SET_FUNCTION | LCD_FUNCTION_4BIT | LCD_FUNCTION_TWO_LINES | LCD_FUNCTION_5X8);
  // wait for more than 37us
  delay_ms(1);
  // Display Control: display on, cursor off, blink off
  LCDWrite(0,LCD_CMD_DISPLAY_CONTOL | LCD_DISPLAY_CONTROL_DISPLAY_ON | LCD_DISPLAY_CONTROL_CURSOR_OFF | LCD_DISPLAY_CONTROL_BLINK_OFF);
  //  wait for more than 37us
  delay_ms(1);
  // Clear display
  LCDWrite(0,LCD_CMD_CLEAR_DISPLAY);
  // wait for more than 1.52ms
  delay_ms(2);
  // Set Entry Mode: increment address counter, don't shift
  LCDWrite(0,LCD_CMD_SET_ENTRY_MODE | LCD_ENTRY_MODE_FIXED | LCD_ENTRY_MODE_INC);
  // Initialization done
}
