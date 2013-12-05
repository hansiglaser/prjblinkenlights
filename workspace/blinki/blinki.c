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
 * Stand alone (without Eclipse)
 *  $ msp430-gcc -mmcu=msp430g2231 -o blinki.elf blinki.c   # LaunchPad Rev. 1.4
 *  $ msp430-gcc -mmcu=msp430g2553 -o blinki.elf blinki.c   # LaunchPad Rev. 1.5
 *  $ msp430-gcc -mmcu=msp430f5438 -o blinki.elf blinki.c   # MSP-EXP430F5438
 *  $ mspdebug rf2500
 *  > prog blinki.elf
 *  > run
 *
 * Note: How to find out a list of all GCC preprocessor defines
 *  $ msp430-gcc -mmcu=msp430f2331 -E -dM blinki.c | grep MSP430 | sort
 */

#ifdef __MSP430G2231__
#  include <msp430g2231.h>
#endif
#ifdef __MSP430G2553__
#  include <msp430g2553.h>
#endif
#ifdef __MSP430F5438__
#  include <msp430f5438.h>
#endif

#if __MSP430G2231__ || __MSP430G2553__ 
// LaunchPad: P1.0 (red LED) and P1.6 (green LED)
#  define LED_R 0x01
#  define LED_G 0x40
#endif

#if __MSP430F5438__
// MSP-EXP430F5438: P1.0 (LED1) and P1.1 (LED2)
#  define LED_R 0x01
#  define LED_G 0x02
#endif
#define LEDS  (LED_R | LED_G)

unsigned int i = 0;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // enable P1.0 (red LED) and P1.6 (green LED) as outputs
  P1DIR |= LEDS;
  // switch on red LED and switch off green LED
  P1OUT = (P1OUT & ~LEDS) | LED_R;

  // main loop
  for (;;) {
    // toggle LEDs
    P1OUT ^= LEDS;
    // delay
    for(i=0; i< 20000; i++);
  }
}
