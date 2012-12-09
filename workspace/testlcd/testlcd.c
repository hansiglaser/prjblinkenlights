/*
 * blinki.c
 *
 *  Created on: 30.11.2012
 *      Author: hansi
 *
 * Debugging: Before each debug session you have to start the JTAG driver
 *   $ mspdebug rf2500 "prog Debug/blinki" gdb
 * from the project base directory
 * ~/devel/ProjectBlinkenlights/workspace/blinki
 *
 * http://wiki.eclipse.org/CDT/User/FAQ#How_do_I_debug_a_remote_application.3F
 *
 * To debug with the MSP430 LaunchPad:
 *  - ... -> Debug Configurations
 *     - C/C++ Remote Application, double-click to add a new one
 *     - at the bottom click on "Select other..."
 *        - [X] Use configuration specific settins
 *        - choose "GDB (DSF) Manual Remote Debugging Launcher"
 *        - [OK]
 *     - "Debugger" tab
 *        - GDB debugger: "msp430-gdb"
 *        - GDB command file: empty
 *        - Sub-Tab "Connection"
 *           - Port number: "2000"
 *     - [Debug]
 *
 */

#include <msp430g2231.h>
#include <stdint.h>

#define LED_R 0x01
#define LED_G 0x40
#define LEDS  (LED_R | LED_G)


#define LCD_RS        0x01   // P1.0
#define LCD_RW        0x02   // P1.1
#define LCD_E         0x04   // P1.2
#define LCD_CTRL      (LCD_RS | LCD_RW | LCD_E)
#define LCD_CTRL_OUT  P1OUT
#define LCD_CTRL_DIR  P1DIR
#define LCD_D4        0x04   // P2.2
#define LCD_D5        0x08   // P2.3
#define LCD_D6        0x20   // P2.5
#define LCD_D7        0x40   // P2.6
#define LCD_DATA      (LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7)
#define LCD_DATA_MSB(x) ((((x)&0x30) >> 2) | (((x)&0xC0) >> 1))   // MSB of uint8_t -> pins
#define LCD_DATA_LSB(x) ((((x)&0x03) << 2) | (((x)&0x0C) << 3))   // LSB of uint8_t -> pins
#define LCD_DATA_TO_MSB(x)  ((((x)&(LCD_D4|LCD_D5)) >> 2) | (((x)&(LCD_D6|LCD_D7)) >> 3))  // pins -> MSB of uint8_t
#define LCD_DATA_TO_LSB(x)  ((((x)&(LCD_D4|LCD_D5)) << 2) | (((x)&(LCD_D6|LCD_D7)) << 1))  // pins -> LSB of uint8_t
#define LCD_DATA_OUT  P2OUT
#define LCD_DATA_IN   P2IN
#define LCD_DATA_DIR  P2DIR


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
#define LCD_ENTRY_MODE_INC       0x01   // increment address counter on new characters
#define LCD_ENTRY_MODE_DEC       0x00   // decrement address counter

#define LCD_DISPLAY_CONTROL_DISPLAY_ON   0x01
#define LCD_DISPLAY_CONTROL_DISPLAY_OFF  0x00
#define LCD_DISPLAY_CONTROL_CURSOR_ON    0x02
#define LCD_DISPLAY_CONTROL_CURSOR_OFF   0x00
#define LCD_DISPLAY_CONTROL_BLINK_ON     0x04
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
  // assert control signals
  LCD_CTRL_OUT = (LCD_CTRL_OUT & ~LCD_CTRL) | Ctrl;
  // MSB: assert data signals
  LCD_DATA_OUT = (LCD_DATA_OUT & ~LCD_DATA) | LCD_DATA_MSB(Data);
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
  // MSB: assert data signals
  LCD_DATA_OUT = (LCD_DATA_OUT & ~LCD_DATA) | LCD_DATA_LSB(Data);
  // rising edge of E
  LCD_CTRL_OUT |= LCD_E;
  // falling edge of E
  LCD_CTRL_OUT &= ~LCD_E;
}

void LCDWriteString(char* St) {
  while (*St) {
    LCDWrite(LCD_RS,*St++);
  }
}

inline void LCDGoto(uint8_t AddrCnt) {
  LCDWrite(0,LCD_CMD_SET_DDRAM_ADDR | AddrCnt);
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
  // TODO: wait for more than 15 ms
  // Set Function: 8 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_8BIT));
  // TODO: wait for more than 4.1 ms
  // Set Function: 8 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_8BIT));
  // TODO: wait for more than 100us
  // Set Function: 4 bit interface
  LCDWriteNibble(0,LCD_DATA_MSB(LCD_CMD_SET_FUNCTION | LCD_FUNCTION_4BIT));
  // Set Function: 4 bit interface, two lines, 5x8 characters
  LCDWrite(0,LCD_CMD_SET_FUNCTION | LCD_FUNCTION_4BIT | LCD_FUNCTION_TWO_LINES | LCD_FUNCTION_5X8);
  // TODO: wait for more than 37us
  // Display Control: display on, cursor off, blink off
  LCDWrite(0,LCD_CMD_DISPLAY_CONTOL | LCD_DISPLAY_CONTROL_DISPLAY_ON | LCD_DISPLAY_CONTROL_CURSOR_OFF | LCD_DISPLAY_CONTROL_BLINK_OFF);
  // TODO: wait for more than 37us
  // Clear display
  LCDWrite(0,LCD_CMD_CLEAR_DISPLAY);
  // TODO: wait for more than 1.52ms
  // Set Entry Mode: increment address counter, don't shift
  LCDWrite(0,LCD_CMD_SET_ENTRY_MODE | LCD_ENTRY_MODE_FIXED | LCD_ENTRY_MODE_INC);
  // Initialization done
}


unsigned int i = 0;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // LCD control and data signals are outputs
  LCD_CTRL_DIR |= LCD_CTRL;
  LCD_DATA_DIR |= LCD_DATA;

  LCDInit();

  LCDGoto(LCD_DDRAM_ADDR(0,0));
  LCDWriteString("Hello World!");

  // main loop
  for (;;) {
    // toggle LEDs
    P1OUT ^= 0x01 | 0x40;
    // delay
    for(i=0; i< 20000; i++);
  }
}
