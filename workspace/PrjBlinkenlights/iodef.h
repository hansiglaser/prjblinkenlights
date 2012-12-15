/*
 * iodef.h
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#ifndef IODEF_H_
#define IODEF_H_

/*
 * Port    Timer    LaunchPad   Signal     Direction
 * P1.0             LED gn      LCD-RS     O
 * P1.1    TA0.0                LCD-R/W    O
 * P1.2    TA0.1                LCD-E      O
 * P1.3             Button      PUSH       I
 * P1.4                         ROTENC-A   I
 * P1.5    TA0.0                ROTENC-B   I
 * P1.6    TA0.1    LED rd      GATE-LCD   O       PWM
 * P1.7                         ROTENC-P   I
 * P2.0    TA1.0                GATE-R     O       PWM
 * P2.1    TA1.1                GATE-G     O       PWM
 * P2.2    TA1.1                LCD-D4     B
 * P2.3    TA1.0                LCD-D5     B
 * P2.4    TA1.2                GATE-B     O       PWM
 * P2.5    TA1.2                LCD-D6     B
 * P2.6    TA0.1                LCD-D7     B
 * P2.7
 *
 */

/*
 * LCD
 */

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
 * Buttons
 */

#define ROTENC_A      0x10   // terminal A: P1.4
#define ROTENC_B      0x20   // terminal B: P1.5
#define ROTENC_P      0x80   // push button: P1.7
#define ROTENC_ALL    (ROTENC_A | ROTENC_B | ROTENC_P)
#define ROTENC_IN     P1IN
#define ROTENC_DIR    P1DIR
#define ROTENC_OUT    P1OUT
#define ROTENC_RES    P1REN
//#define ROTENC_PHASE (((ROTENC_IN & ROTENC_A)?1:0) | ((ROTENC_IN & ROTENC_B)?2:0))
#define ROTENC_PHASE ((ROTENC_IN & (ROTENC_A | ROTENC_B))>>4)
#define ROTENC_PUSH  (!(ROTENC_IN & ROTENC_P))

#define BUTTON_P      0x08   // button: P1.3
#define BUTTON_IN     P1IN
#define BUTTON_DIR    P1DIR
#define BUTTON_OUT    P1OUT
#define BUTTON_RES    P1REN
#define BUTTON_PUSH   (!(BUTTON_IN & BUTTON_P))

/*
 * LEDs
 */

#define LED_LCD       0x40   // P1.6
#define LED_LCD_OUT   P1OUT
#define LED_LCD_DIR   P1DIR
#define LED_LCD_SEL   P1SEL

#define LED_PWM_R     0x01   // P2.0
#define LED_PWM_G     0x02   // P2.1
#define LED_PWM_B     0x10   // P2.4
#define LED_PWM       (LED_PWM_R | LED_PWM_G | LED_PWM_B)
#define LED_PWM_OUT   P2OUT
#define LED_PWM_DIR   P2DIR
#define LED_PWM_SEL   P2SEL

#endif /* IODEF_H_ */
