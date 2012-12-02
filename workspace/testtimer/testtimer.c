//***************************************************************************************
// MSP430 Timer Blink LED Demo - Timer A Software Toggle P1.0 & P1.6
//
// Description; Toggle P1.0 and P1.6 by xor'ing them inside of a software loop.
// Since the clock is running at 1Mhz, an overflow counter will count to 8 and then toggle
// the LED. This way the LED toggles every 0.5s.
// ACLK = n/a, MCLK = SMCLK = default DCO
//
// MSP430G2xx
// -----------------
// /|\| XIN|-
// | | |
// --|RST XOUT|-
// | P1.6|-->LED
// | P1.0|-->LED
//
// Aldo Briano
// Texas Instruments, Inc
// June 2010
// Built with Code Composer Studio v4
//
// http://processors.wiki.ti.com/index.php/MSP430_LaunchPad_LED_Timer
//***************************************************************************************
#include <msp430g2231.h>


#define LED_R   BIT0
#define LED_G   BIT6
#define LEDS    (LED_R | LED_G)
#define LED_OUT P1OUT
#define LED_DIR P1DIR

#define TIMER_DIV   (8*16)

unsigned int timerCount = 0;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // setup IOs for the LEDs
  LED_DIR |= LEDS;  // Set LED ports as to output direction
  LED_OUT &= ~LEDS; // Set the LEDs off
  LED_OUT |= LED_R; // Set on the red LED

  // Set DCO to 1 MHz:
  DCOCTL = 0;              // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;   // Set range
  DCOCTL  = CALDCO_1MHZ;   // Set DCO step + modulation
  // MSP430G2231 has one calibrated frequency (1MHz)
  // BCSCTL1 = 0x86
  // DCOCTL  = 0xB7
  // MSP430G2553 has four calibrated frequencies (1, 8, 12, 16MHz)

  // Set DCO to 16 MHz
  DCOCTL = 0;              // Select lowest DCOx and MODx settings
  // RSEL = 15 -> 15.25MHz at DCO=3 ([DS] p. 23)
  BCSCTL1 = XT2OFF | (RSEL3 | RSEL2 | RSEL1 | RSEL0);
//DCOCTL  = (       DCO1 | DCO0) | 0;   // DCO = 3 -> 15.25MHz
  DCOCTL  = (DCO2              ) | 0;   // DCO = 4 -> 15.25 * 1.08 = 16.47MHz

  CCTL0 = CCIE;  // Capture/compare interrupt enable
  TACTL = TASSEL_2 + MC_2; // Set the timer A to SMCLCK, Continuous

  // Clear the timer and enable timer interrupt
  __enable_interrupt();

  // LPM0 with interrupts enabled
  __bis_SR_register(LPM0_bits + GIE);

  return 0;
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR

__interrupt void Timer_A (void) {
  // SMCLK = 1MHz (default after Reset)
  // TACCR0 = 0 -> period of 65536
  timerCount++;
  // divide by 8 -> 524,288 SMCLK cycles -> ~0.5s
  if (timerCount >= TIMER_DIV) {
    timerCount = 0;
    // toggle
    P1OUT ^= LEDS;
  }
}
