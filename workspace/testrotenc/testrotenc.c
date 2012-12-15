/**
 * Test MSP430 with the rotary encoder
 *
 * Pins:
 *  - P1.0: red LED, blinks
 *  - P1.1: rotary encoder, terminal A
 *  - P1.2: rotary encoder, terminal A
 *  - P1.3: rotary encoder, push button
 *  - P1.6: green LED, PWM
 *
 * This test program uses Timer A0 and its CCR1 to generate a PWM. This is
 * sent to P1.6 which is connected to the green LED on the LaunchPad.
 *
 *
 *     P         P
 * +--| |-------| |--+
 * |                 |
 * |                 |
 * |      -----      |
 * |     /     \     |
 * |    |   *   |    |
 * |     \     /     |
 * |      -----      |
 * |                 |
 * |                 |
 * +--| |--| |--| |--+
 *     A    C    B
 *
 * P-P is closed when the button us pressed
 *
 *
 * A-C  Off ----+     +-----+     +----
 *              |     |     |     |
 *      On      +-----+     +-----+
 *
 * B-C  Off  +-----+     +-----+     +-
 *           |     |     |     |     |
 *      On  -+     +-----+     +-----+
 *
 *           ^           ^           ^
 *           +-----------+-----------+-- detent position
 *
 * Phases: TODO
 *
 */

#include <msp430g2231.h>

#define LED_R   BIT0
#define LED_G   BIT6
#define LEDS    (LED_R | LED_G)
#define LED_OUT P1OUT
#define LED_DIR P1DIR

#define ROTENC_A   BIT1   // terminal A
#define ROTENC_B   BIT2   // terminal B
#define ROTENC_P   BIT3   // push button
#define ROTENC_ALL (ROTENC_A | ROTENC_B | ROTENC_P)
#define ROTENC_IN  P1IN
#define ROTENC_DIR P1DIR
#define ROTENC_OUT P1OUT
#define ROTENC_RES P1REN
//#define ROTENC_PHASE (((ROTENC_IN & ROTENC_A)?1:0) | ((ROTENC_IN & ROTENC_B)?2:0))
#define ROTENC_PHASE ((ROTENC_IN & (ROTENC_A | ROTENC_B))>>1)
#define ROTENC_PUSH  (~(ROTENC_IN & ROTENC_P))

#define PWM_STEP 5000

// Setup LED toggle frequency
//  - SMCLK = 16MHz as setup in main()
//  - TimerA period is 65536
//  - divide by 122 -> 2 Hz -> periode on/off = 1s
#define TIMER_DIV   122

unsigned int timerCount = 0;

signed int RotEncPhase;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // setup IOs for the LEDs
  LED_DIR |= LEDS;  // Set LED ports as to output direction
  LED_OUT &= ~LEDS; // Set the LEDs off
  // setup IOs for rotary encoder
  ROTENC_DIR &= ~ROTENC_ALL;   // set as input
  ROTENC_RES |=  ROTENC_ALL;   // enable pullup/pulldown resistor
  ROTENC_OUT |=  ROTENC_ALL;   // set pullup
  RotEncPhase = ROTENC_PHASE;

  // Set DCO to 16 MHz
  DCOCTL = 0;              // Select lowest DCOx and MODx settings
  // RSEL = 15 -> 15.25MHz at DCO=3 ([DS] p. 23)
  BCSCTL1 = XT2OFF | (RSEL3 | RSEL2 | RSEL1 | RSEL0);
  DCOCTL  = DCO2;   // DCO = 4 -> 15.25 * 1.08 = 16.47MHz

  // Setup TimerA
  TACTL   = TASSEL_2 | MC_2 | TAIE;    // Clk source is SMCLK, continuous mode, interrupt on reset
  TACCTL1 = OUTMOD_7;                  // CCR1 output is reset when CCR1 is reached and set when CCR0 is reached
  TACCR0  = 0x0000;                    // set to "end of periode"
  TACCR1  = 0x0000;                    // CCR1 PWM duty cycle, start at 0, will be incremented in the ISR

  P1SEL |= LED_G;                      // set green LED output as primary peripheral function

  // Clear the timer and enable timer interrupt
  __enable_interrupt();

  // LPM0 with interrupts enabled
  __bis_SR_register(LPM0_bits + GIE);

  return 0;
}

signed int Phase;
int Dec = 0;
int Inc = 0;

// Timer A0 interrupt service routine for CC1 and TA interrupt
#pragma vector = TIMERA1_VECTOR
__interrupt void Timer_A (void) {
  TACTL &= ~TAIFG;      // clear TAIFG (seems necessary!)
  // divide ISR rate for slower blinking LED
  timerCount++;
  if (timerCount >= TIMER_DIV) {
    timerCount = 0;
    // toggle
    P1OUT ^= LED_R;
  }

  // read in rotary encoder
  Phase = ROTENC_PHASE;
  int X = P1IN;
  if ((RotEncPhase == 3) && (Phase == 2)) {
    // clock-wise
    Inc++;
    if (TACCR1 < (0xFFFF - PWM_STEP)) {
      TACCR1 += PWM_STEP;
    } else {
      TACCR1 = 0xFFFF;
    }
  } else if ((RotEncPhase == 3) && (Phase == 1)) {
    // counter-clock-wise
    Dec++;
    // when decrementing, the LED sometimes flickers, this is because the timer can overflow
    if (TACCR1 > PWM_STEP) {
      TACCR1 -= PWM_STEP;
    } else {
      TACCR1 = 0;
    }
  }
  RotEncPhase = Phase;
}
