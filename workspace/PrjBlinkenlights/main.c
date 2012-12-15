/**
 * Project Blinkenlights -- Main Program
 *
 * RGB LED strip controller with an LCD, rotary encoder with button and one
 * more button.
 *
 * The whole function is accomplished with two parts:
 *  - int main()
 *  - timer ISR
 *
 * The timers are additionally used to generate PWMs for the LEDs (see
 * init_timer()):
 *  - Timer A0 is used with CCR1 to generate a PWM for the LCD backlight.
 *  - Timer A1 is used with a trick so that all three CCRs including CCR0
 *    can be used for PWMs for the RGB LED strip.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include <msp430g2553.h>

#include "iodef.h"
#include "infomem.h"
#include "lcd.h"
#include "menu.h"
#include "color.h"
#include "utils.h"


// TODO: do we need this define?
#define TIMER_DIV   (8*16)
// TODO: if no define is needed, move this variable directly above the ISR
unsigned int timerCount = 0;

/**
 * Setup clock generation unit DCO to 16 MHz
 */
void init_clock() {
  // MSP430G2231 has one calibrated frequency (1MHz)
  // BCSCTL1 = 0x86
  // DCOCTL  = 0xB7
  // MSP430G2553 has four calibrated frequencies (1, 8, 12, 16MHz)

  // Set DCO to 16 MHz
  DCOCTL  = 0;              // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;   // Set range
  DCOCTL  = CALDCO_16MHZ;   // Set DCO step + modulation
}

/**
 * Setup IO pin directions, values, pullups, ...
 */
void init_io() {
  // LCD control and data signals are outputs
  LCD_CTRL_DIR |= LCD_CTRL;
  LCD_DATA_DIR |= LCD_DATA;

  // rotary encoder signals are inputs and use pull-ups
  ROTENC_DIR &= ~ROTENC_ALL;   // set as input
  ROTENC_RES |=  ROTENC_ALL;   // enable pullup/pulldown resistor
  ROTENC_OUT |=  ROTENC_ALL;   // set pullup
//  RotEncPhase = ROTENC_PHASE;

  // button signal is input and uses an pull-up
  BUTTON_DIR &= ~BUTTON_P;     // set as input
  BUTTON_RES |=  BUTTON_P;     // enable pullup/pulldown resistor
  BUTTON_OUT |=  BUTTON_P;     // set pullup

  // LCD backlight LED is an output
  LED_LCD_OUT &= ~LED_LCD;     // switch off
  LED_LCD_DIR |=  LED_LCD;     // set as output
  LED_LCD_SEL |=  LED_LCD;     // set output as primary peripheral function

  // RGB LEDs are outputs
  LED_PWM_OUT &= ~LED_PWM;     // switch off
  LED_PWM_DIR |=  LED_PWM;     // set as output
  LED_PWM_SEL |=  LED_PWM;     // set output as primary peripheral function

  // set P3 pins as outputs to reduce leakage, because they are not bonded
  P3DIR = 0xFF;
}

/**
 * Setup timers
 *
 * TA0.1 on pin P1.6 used for LCD backlight
 * TA1.0 on pin P2.0 used for Red
 * TA1.1 on pin P2.1 used for Green
 * TA1.2 on pin P2.4 used for Blue
 *
 * Timer A0 is used with CCR1 to generate a PWM
 *
 * Timer A1 is used with a trick so that CCR0 can also be used for PWM
 */
void init_timer() {
  // Setup TimerA0: CCR1 PWM used for LCD backlight only
  TA0CTL   = TASSEL_2 | MC_2;           // Clk source is SMCLK, continuous mode, no interrupt
  TA0CCTL1 = OUTMOD_7;                  // CCR1 output is reset when CCR1 is reached and set when CCR0 is reached
  TA0CCR0  = 0x0000;                    // set to "end of periode"
  TA0CCR1  = 0x8000;                    // CCR1 PWM duty cycle default value

  // Setup TimerA1: CCR0..CCR2 PWMs used for RGB LED strip
  TA1CTL   = TASSEL_2 | MC_2 | TAIE;    // Clk source is SMCLK, continuous mode, interrupt on reset
  TA1CCTL0 = OUTMOD_1;                  // CCR0 output is set when CCR0 is reached
  TA1CCTL1 = OUTMOD_1;                  // CCR1 output is set when CCR1 is reached
  TA1CCTL2 = OUTMOD_1;                  // CCR2 output is set when CCR1 is reached
  TA1CCR0  = 0x0000;                    // CCR0 PWM duty cycle default value
  TA1CCR1  = 0x0000;                    // CCR1 PWM duty cycle default value
  TA1CCR2  = 0x0000;                    // CCR2 PWM duty cycle default value
}

/****************************************************************************
 **** Main Program **********************************************************
 ****************************************************************************/

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // set DCO to 16 MHz
  init_clock();
  // setup IO pins
  init_io();
  // setup timer
  init_timer();
  // initialize LCD
  // TODO
  // load data from Info Memory
  // TODO

  // Clear the timer and enable timer interrupt
  __enable_interrupt();

  // main loop
  while (true) {
    // LPM0 with interrupts enabled
    __bis_SR_register(LPM0_bits + GIE);
    // wake-up from LPM0 -> we have something to do
    // TODO
  }

  return 0;
}

signed int Phase;
int Dec = 0;
int Inc = 0;
signed int RotEncPhase;
/*volatile uint8_t ButtonPush;
volatile uint8_t RotEncPush;*/


/**
 * Periodic Timer Interrupt
 *
 * Jobs:
 *  - handle PWM simulation
 *  - read rotary encoder and button -> notify main program
 *  - ramp up/down PWMs
 */
// Timer A0 interrupt service routine for CC1 and TA interrupt
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A (void) {
  TA1CTL &= ~TAIFG;              // clear TAIFG (seems necessary!)

  // change output mode to reset the output signal
  TA1CCTL0 = OUTMOD_0;
  TA1CCTL0 = OUTMOD_1;
  TA1CCTL1 = OUTMOD_0;
  TA1CCTL1 = OUTMOD_1;
  TA1CCTL2 = OUTMOD_0;
  TA1CCTL2 = OUTMOD_1;

  // SMCLK = 16MHz
  // period of 65536 -> 244.14Hz interrupt rate = 4.096 ms periode

  // divide ISR rate for slower blinking LED
  timerCount++;
  if (timerCount >= TIMER_DIV) {
    timerCount = 0;

  }

  // read in rotary encoder
  Phase = ROTENC_PHASE;
  if ((RotEncPhase == 3) && (Phase == 2)) {
    // clock-wise
    Inc++;
    // TODO
  } else if ((RotEncPhase == 3) && (Phase == 1)) {
    // counter-clock-wise
    Dec++;
    // TODO
  }
  RotEncPhase = Phase;
  // TODO: handle slow/fast rotation

/*  Phase = ROTENC_PUSH;
  int x = P1IN;
  RotEncPush = ROTENC_PUSH;
  ButtonPush = BUTTON_PUSH;*/

  if (ROTENC_PUSH) {
    TA0CCR1 = 0x0001;
  } else if (BUTTON_PUSH) {
    TA0CCR1 = 0xFFFF;
  }

/*
  // when decrementing, the LED sometimes flickers, this is because the timer can overflow
  TA0CCR1  // LCD backlight
  TA1CCR0  // red
  TA1CCR1  // green
  TA1CCR2  // blue
*/
/*
  LPM0_EXIT; // exit LPM0 when returning from ISR
*/
}
