/*
 * color.h
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#ifndef COLOR_H_
#define COLOR_H_

#include <stdint.h>

// msp430g2553.h line defines "V" as one of the status register bits, try if
// just undefining "V" works :-)
#undef V

typedef struct {
  union {
    struct {
      uint16_t R;
      uint16_t G;
      uint16_t B;
    } RGB;
    struct {
      uint16_t H;
      uint16_t S;
      uint16_t V;
    } HSV;
  };
} TColor;

uint16_t Brightness2PWM(uint16_t Brightness);

#endif /* COLOR_H_ */
