/**
 * Project Blinkenlights -- Main Program
 *
 * RGB LED strip controller with an LCD, rotary encoder with push button and
 * one more button.
 *
 * The device is built of the following components:
 *  - TI MSP430 LaunchPad Rev. 1.5 with an MSP430G2553
 *  - custom BoosterPack add-on board to host all additional electronics
 *     - 12V RGB LED strip (via connector)
 *     - 12V power supply
 *     - 12V -> 5V and 5V -> 3.3V LDO
 *     - 4x20 LCD display
 *     - rotary encoder with push button
 *     - additional button
 *
 * The whole function is accomplished with two parts:
 *  - int main()
 *  - timer ISR
 *
 * The main() has a loop which enters LPM0 at its start. Note that LPM1 saves
 * more power by deactivating the DCO and its DC generator, but only if it is
 * not used for SMCLK. Since we use SMCLK for the timer, LPM1 is identical to
 * LPM0.
 *
 * From this low-power mode the main loop is woken up by the timer ISR, if
 * something has to be done. All main functionality is then performed by
 * main(). So, there are three reasons for the ISR to wakeup main():
 *  - rotary encoder was rotated
 *  - a button was pressed
 *  - main() asked for it using a semaphore
 *  - a timeout was reached
 *
 * The timers are additionally used to generate PWMs for the LEDs (see
 * init_timer()):
 *  - Timer A0 is used with CCR1 to generate a PWM for the LCD backlight.
 *  - Timer A1 is used with all three CCRs including CCR0 to generate PWMs for the RGB LED strip.
 *
 * A trick is necessary to use CCR0, CCR1 and CCR2 for PWM, because the
 * Reset/Set mode is not available for CCR0. Therefore we use the Set output
 * mode with the timer in continuous mode. The outputs are reset in the timer
 * overflow ISR (which is a bit tricky, though).
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
 * Timer A1 is setup to count the full 16 bit register and overflow after
 * 0xFFFF. Upon each overflow, an ISR is executed. With the Sub-main clock
 * SMCLK = 16MHz this results in 244.14Hz interrupt rate = 4.096 ms periode.
 *
 * Timeouts:
 * ---------
 * Global variables counted downward in the timer ISR. It reaching 0, the
 * main() is notified.
 *
 * Rotary Encoder:
 * ---------------
 * The NOBLE RE0124 rotary encoder has 24 steps per 360° rotation plus a push
 * button. The rotation causes two switches to close and therefore connect the
 * terminals A and B to GND. The MSP430 GPIO pins used have the pull-ups
 * activated.
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
 * The two inputs A and B are summarized by the macro ROTENC_PHASE. At the
 * detent positions, both switches are open, so 0x03 is seen. When rotating
 * clock-wise, first terminal A goes to 0, so 0x02 is seen. When rotating
 * counter-clock-wise, first terminal B goes to 0, so 0x01 is seen. These two
 * values are used to identify rotation direction.
 *
 * The rotation speed is identified using the upward counter variable
 * RotEncCount in the ISR. The further this value has counted, the slower the
 * rotation was.
 *
 * When the user turns the knob, the direction (and speed) is communicated to
 * main() by setting RotEncValue to a non-zero value (positive or negative)
 * and LPM0 is exited after the ISR, so main() can handle the user input.
 *
 * Buttons:
 * --------
 * The rotary encoder also has a push button (if the knob is pushed).
 * Additionally, an extra button is soldered to the PCB. Both are normally
 * open and connect the signal wire to GND when pressed. The MSP430 GPIO pins
 * are configured to apply an internal pull-up.
 *
 * The current state of each button is stored as bits in the variable
 * ButtonValue. The old state is also stored in this variable by a shift-left
 * of the old state. Therefore a rising and falling edge can easily be
 * detected.
 *
 * When the user has pressed one of these buttons, LPM0 is exited after the
 * ISR, so main() can handle the user input.
 *
 * Time-dependent Functions:
 * -------------------------
 * All time-dependent functions (like the fade-in/-out of the LCD backlight
 * LED PWM) need the help of the timer interrupt. Some semaphores are used to
 * ask the ISR to exit LPM0 after its execution. In main() the timed operation
 * is then performed.
 *
 * LCD Backlight Fade-In/-Out:
 * ---------------------------
 * The semaphores SEM_LCD_FADE_IN/_OUT are used so that the timer interrupt
 * exits LPM0 after execution. In main(), the corresponding semaphore is used
 * to determin whetner fade-in or -out is requested. In either case, the
 * constants LCD_FADE_IN_STEP/LCD_FADE_OUT_STEP are added/subtracted to the
 * current value of LedLcdBacklight. Its value is then converted via a
 * non-linear transfer function to the PWM value.
 *
 * Rainbow:
 * --------
 * TODO
 *
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

/****************************************************************************
 **** Global Variables ******************************************************
 ****************************************************************************/

volatile int8_t RotEncValue = 0;   // ISR -> main(): will be -128 .. +127 depending on rotation speed

volatile uint8_t ButtonValue = 0;  // ISR -> main(): bit field with old and new button value
#define BV_ROTENC_NEW 0x01
#define BV_ROTENC_OLD 0x02
#define BV_ROTENC     0x03
#define BV_BUTTON_NEW 0x04
#define BV_BUTTON_OLD 0x08
#define BV_BUTTON     0x0C
#define BV_SHIFT      ((ButtonValue << 1) & (BV_ROTENC_OLD | BV_BUTTON_OLD))
#define BV_ROTENC_RISE ((ButtonValue & BV_ROTENC) == (BV_ROTENC_NEW))
#define BV_ROTENC_FALL ((ButtonValue & BV_ROTENC) == (BV_ROTENC_OLD))
#define BV_ROTENC_EDGE (BV_ROTENC_RISE || BV_ROTENC_FALL)
#define BV_BUTTON_RISE ((ButtonValue & BV_BUTTON) == (BV_BUTTON_NEW))
#define BV_BUTTON_FALL ((ButtonValue & BV_BUTTON) == (BV_BUTTON_OLD))
#define BV_BUTTON_EDGE (BV_BUTTON_RISE || BV_BUTTON_FALL)
#define BV_EDGE ((ButtonValue & (BV_ROTENC_OLD | BV_BUTTON_OLD)) ^ BV_SHIFT)

volatile uint8_t TimeoutReached;  // ISR -> main(): bit field signalling which timeout was reached
#define TIMEOUT_REACHED_LCD_BACKLIGHT    0x01
volatile uint16_t TimeoutLcdBacklight;

#define TIMEOUT_LCD_BACKLIGHT    244*3    // 3 seconds

volatile uint8_t Semaphores = 0;  // main() -> ISR
#define SEM_PWM_LCD      0x01     // new value for LCD backlight
#define SEM_PWM_RGB      0x02     // new values for RGB LED strip
#define SEM_LCD_FADE_IN  0x04     // leave LPM0 after ISR, so that main() can calculate LCD backlight fade-in
#define SEM_LCD_FADE_OUT 0x08     // leave LPM0 after ISR, so that main() can calculate LCD backlight fade-in
#define SEM_RAINBOW      0x10     // leave LPM0 after ISR, so that main() can calculate rainbow colors
#define SEM_PERIODIC     (SEM_LCD_FADE_IN | SEM_LCD_FADE_OUT | SEM_RAINBOW)
// note: these SEM_PERIODIC semaphores are not reset by the ISR, because they
// are used by main() so it knows it is performing an ongoing task

volatile uint16_t PWMLCD;       // PWM comparison value written to TA0CCR1
volatile uint16_t PWMRGBRed;    // PWM comparison value written to TA1CCR0
volatile uint16_t PWMRGBGreen;  // PWM comparison value written to TA1CCR1
volatile uint16_t PWMRGBBlue;   // PWM comparison value written to TA1CCR2

#define LCD_FADE_IN_STEP     (65536/244)*5   // 1/5 = 0.2s
#define LCD_FADE_OUT_STEP    (65536/244)/2   // 2s

// TODO: do we need this define?
#define TIMER_DIV   (8*16)
// TODO: if no define is needed, move this variable directly above the ISR
unsigned int timerCount = 0;

/****************************************************************************
 **** Initialization ********************************************************
 ****************************************************************************/

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
 **** Functions *************************************************************
 ****************************************************************************/

int cbColorTemperature(int Delta, void* Data) {
  // TODO: either as list or as values with varying step size
}

void cbRGB() {
  // TODO: update HSV values
  // TODO: update PWM
}

void cbHSV() {
  // TODO: update RGB values
  // TODO: update PWM
}

void cbRainbow() {
  // TODO: update rainbow parameters
}

void cbSetUserColor(void* Data) {
  // TODO: user color
}

/****************************************************************************
 **** Menu ******************************************************************
 ****************************************************************************/

const TMenuEntry MenuWhite[] = {
  {.Type = metNumber, .Label = "Helligkeit",    .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = 0 } },
  {.Type = metNumber, .Label = "Farbtemp.",     .NumberData  = {.Unit = 'K', .CBValue = 0, .CBData = 0 } },
  {.Type = metReturn, .Label = "Zur�ck" }
};

const TMenuEntry MenuRGB[] = {
  {.Type = metNumber, .Label = "Rot",           .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = &PersistentRam.RGB.RGB.R, .CBChange = cbRGB } },
  {.Type = metNumber, .Label = "Gr�n",          .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = &PersistentRam.RGB.RGB.G, .CBChange = cbRGB } },
  {.Type = metNumber, .Label = "Blau",          .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = &PersistentRam.RGB.RGB.B, .CBChange = cbRGB } },
  {.Type = metReturn, .Label = "Zur�ck" }
};

const TMenuEntry MenuHSV[] = {
  {.Type = metNumber, .Label = "H: Farbton",    .NumberData  = {.Unit = 'X', .CBValue = &cbCircle,  .CBData = &PersistentRam.HSV.HSV.H, .CBChange = cbHSV } },
  {.Type = metNumber, .Label = "S: Saettigung", .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = &PersistentRam.HSV.HSV.S, .CBChange = cbHSV } },
  {.Type = metNumber, .Label = "V: Helligkeit", .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = &PersistentRam.HSV.HSV.V, .CBChange = cbHSV } },
  {.Type = metReturn, .Label = "Zur�ck" }
};

const TMenuEntry MenuRainbow[] = {
  {.Type = metNumber, .Label = "Geschwindigk.", .NumberData  = {.Unit = '%', .CBValue = 0/*TODO*/, .CBData = cbRainbow } },
  {.Type = metNumber, .Label = "S: Saettigung", .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = 0/*TODO*/, .CBChange = cbRainbow } },
  {.Type = metNumber, .Label = "V: Helligkeit", .NumberData  = {.Unit = '%', .CBValue = &cbPercent, .CBData = 0/*TODO*/, .CBChange = cbRainbow } },
  {.Type = metReturn, .Label = "Zur�ck" }
};

const TMenuEntry MenuSaveUserColor[] = {
  {.Type = metString, .Label = "Name",          .StringData  = {.String = 0/*TODO*/, .Length = 0 } },
  {.Type = metSimple, .Label = "Speichern",     .SimpleData  = {.Callback = 0, .CBData = 0} },
  {.Type = metReturn, .Label = "Zur�ck" },
};

TMenuEntry MenuUserColors[] = {
  {.Type = metSimple, .Label = "Farbe 1",       .SimpleData  = {.Callback = 0, .CBData = 0} },
  {.Type = metSimple, .Label = "Farbe 2",       .SimpleData  = {.Callback = 0, .CBData = 0} },
  {.Type = metSimple, .Label = "Farbe 3",       .SimpleData  = {.Callback = 0, .CBData = 0} },
  {.Type = metSimple, .Label = "Farbe 4",       .SimpleData  = {.Callback = 0, .CBData = 0} },
  {.Type = metSubmenu,.Label = "Speichern",     .SubMenuData = {.NumEntries = 3, .SubMenu = &MenuSaveUserColor} },
  {.Type = metReturn, .Label = "Zur�ck" },
};

const TMenuEntry MenuConfig[] = {
//  {.Type = metNumber, .Label = "Geschwindigk.", .NumberData  = {.Unit = '%', .CBValue = 0, .CBData = 0 } },
  // start color: not necessary if we save the current state
  // LCD backlight on/off after power on -> not really necessary, just let it off and fade-in on a button press
    {.Type = metReturn, .Label = "Zur�ck" },
};

const TMenuEntry MainMenu[] = {
  {.Type = metSimple, .Label = "Aus",           .SimpleData  = {.Callback = 0, .CBData = 0}},
  {.Type = metSubmenu,.Label = "Weiss",         .SubMenuData = {.NumEntries = 3, .SubMenu = &MenuWhite} },
  {.Type = metSubmenu,.Label = "RGB",           .SubMenuData = {.NumEntries = 4, .SubMenu = &MenuRGB} },
  {.Type = metSubmenu,.Label = "HSV",           .SubMenuData = {.NumEntries = 4, .SubMenu = &MenuHSV} },
  {.Type = metSubmenu,.Label = "Regenbogen",    .SubMenuData = {.NumEntries = 2, .SubMenu = &MenuRainbow} },
  {.Type = metSubmenu,.Label = "Eigene Farben", .SubMenuData = {.NumEntries = 2, .SubMenu = &MenuUserColors} },
  {.Type = metSubmenu,.Label = "Konfiguration", .SubMenuData = {.NumEntries = 2, .SubMenu = &MenuConfig} },
};

/****************************************************************************
 **** Main Program **********************************************************
 ****************************************************************************/

int main(void) {
  TMenuState MenuState;
  uint16_t LedLcdBacklight = 0;
  bool UserAction;

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
  // initialize menu
  menu_init(MainMenu,7,&MenuState);
  menu_draw(&MenuState);
  // initialize Flash controller and load data from Info Memory
  infomem_init();
  infomem_read();

  // Clear the timer and enable timer interrupt
  __enable_interrupt();

  // main loop
  while (true) {
    // LPM0 with interrupts enabled
    __bis_SR_register(LPM0_bits + GIE);
    // wake-up from LPM0 -> we have something to do

    // menu handling /////////////////////////////////////////////////////////
    UserAction = true;
    if      (BV_ROTENC_RISE)   menu_handle_event(&MenuState, mePress,  0);
    else if (BV_BUTTON_RISE)   menu_handle_event(&MenuState, meBack,   0);
    else if (RotEncValue != 0) menu_handle_event(&MenuState, meRotate, RotEncValue);
    else UserAction = false;
    if (UserAction) {
      TimeoutLcdBacklight = TIMEOUT_LCD_BACKLIGHT;  // reset timeout
      // fade-in LCD backlight, 3 cases: on, fade-in, fade-out
      if ((LedLcdBacklight != 0xFFFF) && !(Semaphores & SEM_LCD_FADE_IN)) {
        Semaphores &= ~SEM_LCD_FADE_OUT;
        Semaphores |=  SEM_LCD_FADE_IN;
      }
    }

    // TODO: remove //////////////////////////////////////////////////////////
    if (BV_ROTENC_RISE) {
      Semaphores |= SEM_LCD_FADE_IN;
      TimeoutLcdBacklight = TIMEOUT_LCD_BACKLIGHT;  // 3 seconds
    }
    if (BV_BUTTON_RISE) Semaphores |= SEM_LCD_FADE_OUT;
    if (RotEncValue > 0)
      TA0CCR1 += RotEncValue << 12;
    else if (RotEncValue < 0)
      TA0CCR1 -= (-RotEncValue) << 12;
    if (TimeoutReached & TIMEOUT_REACHED_LCD_BACKLIGHT) {
      //
      Semaphores |= SEM_LCD_FADE_OUT;
      TimeoutReached &= ~TIMEOUT_REACHED_LCD_BACKLIGHT;
    }

    // LCD backlight fade-in/out ///////////////////////////////////////////////
    if (Semaphores & SEM_LCD_FADE_IN) {
      // fade-in
      if (LedLcdBacklight < (0xFFFF - LCD_FADE_IN_STEP)) {
        LedLcdBacklight += LCD_FADE_IN_STEP;
      } else {
        // completely on, done
        LedLcdBacklight = 0xFFFF;
        Semaphores &= ~SEM_LCD_FADE_IN;
      }
      PWMLCD = Brightness2PWM(LedLcdBacklight);
      Semaphores |= SEM_PWM_LCD;  // notify ISR to update timer comparison register
    } else if (Semaphores & SEM_LCD_FADE_OUT) {
      // fade-out
      // when decrementing, the LED sometimes flickers, this is because the timer can overflow
      if (LedLcdBacklight > LCD_FADE_OUT_STEP) {
        LedLcdBacklight -= LCD_FADE_OUT_STEP;
      } else {
        // completely off, done
        LedLcdBacklight = 0;
        Semaphores &= ~SEM_LCD_FADE_OUT;
      }
      PWMLCD = Brightness2PWM(LedLcdBacklight);
      Semaphores |= SEM_PWM_LCD;  // notify ISR to update timer comparison register
    }

    // Rainbow ///////////////////////////////////////////////////////////////
    // TODO
  }

  return 0;
}

/****************************************************************************
 **** Timer ISR *************************************************************
 ****************************************************************************/

uint8_t RotEncPhase = 0;   // initialize to an invalid value so that the if/elseif structure does nothing
int RotEncDec = 0;   // not really necessary
int RotEncInc = 0;   // not really necessary
uint8_t RotEncCount = 0;   // upward counter to measure time between steps for virtual acceleration
int8_t RotEncDir = 0;      // direction of last step, to avoid acceleration on rapid changes of the rotation direction

/**
 * Translate number of counts between successive rotary encoder steps to a
 * virtual step width.
 */
int8_t RotEncSpeed(uint8_t Count) {
  if (Count > 50) {
    return 1;
  } else if (Count > 20) {
    return 2;
  } else if (Count > 10) {
    return 3;
  } else {
    return 4;
  }
}

/**
 * Periodic Timer Interrupt
 *
 * Jobs:
 *  - handle PWM simulation
 *  - read rotary encoder and button -> notify main program
 *  - ramp up/down PWMs
 *  - handle timeouts
 */
// Timer A0 interrupt service routine for CC1 and TA interrupt
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A (void) {
  TA1CTL &= ~TAIFG;              // clear TAIFG (seems necessary!)

  // change output mode to reset the output signal ///////////////////////////
  TA1CCTL0 = OUTMOD_0;
  TA1CCTL0 = OUTMOD_1;
  TA1CCTL1 = OUTMOD_0;
  TA1CCTL1 = OUTMOD_1;
  TA1CCTL2 = OUTMOD_0;
  TA1CCTL2 = OUTMOD_1;

  // set new PWM values //////////////////////////////////////////////////////
  if (Semaphores & SEM_PWM_LCD) {
    TA0CCR1 = PWMLCD;          // LCD backlight
    Semaphores &= ~SEM_PWM_LCD;
  }
  if (Semaphores & SEM_PWM_RGB) {
    TA1CCR0 = PWMRGBRed;       // red
    TA1CCR1 = PWMRGBGreen;     // green
    TA1CCR2 = PWMRGBBlue;      // blue
    Semaphores &= ~SEM_PWM_RGB;
  }
  // when decrementing, the LED sometimes flickers, this is because the timer can overflow

  // handle timeouts /////////////////////////////////////////////////////////
  if (TimeoutLcdBacklight) {
    TimeoutLcdBacklight--;
    if (TimeoutLcdBacklight == 0) {
      TimeoutReached |= TIMEOUT_REACHED_LCD_BACKLIGHT;
      LPM0_EXIT; // exit LPM0 when returning from ISR
    }
  }

/*
  // SMCLK = 16MHz
  // period of 65536 -> 244.14Hz interrupt rate = 4.096 ms periode

  // divide ISR rate for slower blinking LED
  timerCount++;
  if (timerCount >= TIMER_DIV) {
    timerCount = 0;

  }
*/

  // read in rotary encoder //////////////////////////////////////////////////
  uint8_t NewPhase = ROTENC_PHASE;
  if ((RotEncPhase == 3) && (NewPhase == 2)) {
    // clock-wise
    RotEncInc++;
    // tell the main program
    if (RotEncDir == 1) {
      // Acceleration: only if rotation in the same direction
      RotEncValue = RotEncSpeed(RotEncCount);
    } else {
      RotEncValue = 1;
    }
    RotEncDir = 1;
    RotEncCount = 0;
    LPM0_EXIT; // exit LPM0 when returning from ISR
  } else if ((RotEncPhase == 3) && (NewPhase == 1)) {
    // counter-clock-wise
    RotEncDec++;
    // tell the main program
    if (RotEncDir == -1) {
      // Acceleration: only if rotation in the same direction
      RotEncValue = -RotEncSpeed(RotEncCount);
    } else {
      RotEncValue = -1;
    }
    RotEncDir = -1;
    RotEncCount = 0;
    LPM0_EXIT; // exit LPM0 when returning from ISR
  } else {
    RotEncValue = 0;
    if (RotEncCount < 255)
      RotEncCount++;
  }
  RotEncPhase = NewPhase;

  // read in buttons /////////////////////////////////////////////////////////
  ButtonValue = BV_SHIFT;
  if (ROTENC_PUSH) ButtonValue |= BV_ROTENC_NEW;
  if (BUTTON_PUSH) ButtonValue |= BV_BUTTON_NEW;
  if (BV_EDGE)
    LPM0_EXIT; // exit LPM0 when returning from ISR

  // periodic wakeup semaphore ///////////////////////////////////////////////
  if (Semaphores & SEM_PERIODIC) {
    LPM0_EXIT; // exit LPM0 when returning from ISR
    // not reset by ISR!
  }
}
