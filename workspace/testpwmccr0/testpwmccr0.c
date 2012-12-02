/**
 * Test MSP430 PWM
 *
 * This test program uses Timer A0 and its CCR0 and CCR1 to generate a PWM.
 * These are sent to P1.1 and P1.6, respectively, which are connected to the
 * an external and the green LED on the LaunchPad, respectively.
 *
 * A trick is necessary to use CCR0 and CCR1 for PWM, because the Reset/Set
 * mode is not available for CCR0. Therefore we use the Set output mode with
 * the timer in continuous mode. The outputs are reset in the timer overflow
 * ISR (which is a bit tricky, though).
 *
 * Note that this reverses the PWM value, because the output is LOW from
 * 0..CCRx and high from CCRx..0xFFFF.
 *
 * MSP430G2231:
 *  - TA0.0 is available on P1.1, P1.5
 *  - TA0.1 is available on P1.2, P1.6, P2.6
 * MSP430G2553:
 *  - TA0.0 is available on P1.1, P1.5
 *  - TA0.1 is available on P1.2, P1.6, P2.6
 *  - TA0.2 is available on P3.0. P3.6        // damn, only available with 28-pin devices!
 *  - TA1.0 is available on P2.0. P2.3, P3.1
 *  - TA1.1 is available on P2.1, P2.2, P3.2
 *  - TA1.2 is available on P2.4, P2.5, P3.3
 *
 */

#include <msp430g2231.h>

#define LED_R   BIT0
#define LED_G   BIT6
#define LED_X   BIT1
#define LEDS    (LED_R | LED_G | LED_X)
#define LED_OUT P1OUT
#define LED_DIR P1DIR

// Setup LED toggle frequency
//  - SMCLK = 16MHz as setup in main()
//  - TimerA period is 65536
//  - divide by 122 -> 2 Hz -> periode on/off = 1s
#define TIMER_DIV   122

unsigned int timerCount = 0;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // setup IOs for the LEDs
  LED_DIR |= LEDS;  // Set LED ports as to output direction
  LED_OUT &= ~LEDS; // Set the LEDs off

  // Set DCO to 16 MHz
  DCOCTL = 0;              // Select lowest DCOx and MODx settings
  // RSEL = 15 -> 15.25MHz at DCO=3 ([DS] p. 23)
  BCSCTL1 = XT2OFF | (RSEL3 | RSEL2 | RSEL1 | RSEL0);
  DCOCTL  = DCO2;   // DCO = 4 -> 15.25 * 1.08 = 16.47MHz

  // Setup TimerA
  TACTL   = TASSEL_2 | MC_2 | TAIE;    // Clk source is SMCLK, continuous mode, interrupt on reset
  TACCTL0 = OUTMOD_1;                  // CCR0 output is set when CCR0 is reached
  TACCTL1 = OUTMOD_1;                  // CCR1 output is set when CCR1 is reached
  TACCR0  = 0x0000;                    // CCR0 PWM duty cycle, start at 0, will be incremented in the ISR
  TACCR1  = 0x0000;                    // CCR1 PWM duty cycle, start at 0, will be incremented in the ISR

  P1SEL |= LED_G | LED_X;              // set green LED and external LED outputs as primary peripheral function

  // Clear the timer and enable timer interrupt
  __enable_interrupt();

  // LPM0 with interrupts enabled
  __bis_SR_register(LPM0_bits + GIE);

  return 0;
}

// Timer A0 interrupt service routine for CC1 and TA interrupt
#pragma vector = TIMERA1_VECTOR
__interrupt void Timer_A (void) {
  TACTL &= ~TAIFG;              // clear TAIFG (seems necessary!)

  // change output mode to reset the output signal
  TACCTL0 = OUTMOD_0;
  TACCTL0 = OUTMOD_1;
  TACCTL1 = OUTMOD_0;
  TACCTL1 = OUTMOD_1;

  // divide ISR rate for slower blinking LED
  timerCount++;
  if (timerCount >= TIMER_DIV) {
    timerCount = 0;
    // toggle
    P1OUT ^= (LED_R /*| LED_X*/);
  }

  // fade in with PWM from 0 to 0xFFFF within 1s
  TACCR0 -= 192;
  TACCR1 -= 269;   // 65536 / 244Hz = 268.59 -> increment by 269
}
