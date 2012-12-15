/*
 * color.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#include <math.h>
#include <stdint.h>
#include "color.h"

/**
 * Convert intensity to PWM with non-linear function
 */
uint16_t Brightness2PWM(uint16_t Brightness) {
  float x;
  uint16_t y;
  if (Brightness == 0) return 0;
  if (Brightness == 0xFFFF) return 0xFFFF;
  // TODO: find a better fixed-point solution (this adds ~4kB to the executable!)
  x = expf(Brightness*0.0001);
  y = floor((x-1.0) * (65535.0 / (exp(6.5535)-1.0)));
  return y;
}

/**
 * Convert RGB color to HSV
 *
 * see http://de.wikipedia.org/wiki/HSV-Farbraum#Transformation_von_RGB_und_HSV
 *
 * assume R,G,B = [0,1], H = [0,360°), S,V = [0,1].
 *
 * MIN = min(R,G,B)
 * MAX = max(R,G,B)
 *
 * if MIN == MAX then H = 0    // -> R = G = B
 * else if MAX == R then H =   0° + 60° * (G-B)/(MAX-MIN)
 * else if MAX == G then H = 120° + 60° * (B-R)/(MAX-MIN)
 * else if MAX == B then H = 240° + 60° * (R-G)/(MAX-MIN)
 *
 * if H < 0° then H = H * 360°
 *
 * if MAX = 0 then S = 0    // -> R = G = B = 0
 * else S = (MAX-MIN)/MAX
 *
 * V = MAX
 *
 */
void RGB2HSV(const TColor* RGB, TColor* HSV) {

}

/**
 * Convert HSV color to RGB
 *
 * http://de.wikipedia.org/wiki/HSV-Farbraum#Transformation_von_RGB_und_HSV
 *
 * assume H = [0,360°), S,V = [0,1], R,G,B = [0,1].
 *
 * hi = RoundDown(H / 60°)
 * f = (H / 60° - hi)
 * p = V*(1-S)
 * q = V*(1-S*f)
 * t = V*(1-S*(1-f))
 *
 * if hi = 0 or 6 then (R,G,B) = (V,t,p)
 * else if h = 1  then (R,G,B) = (q,V,p)
 * else if h = 2  then (R,G,B) = (p,V,t)
 * else if h = 3  then (R,G,B) = (p,q,V)
 * else if h = 4  then (R,G,B) = (t,p,V)
 * else if h = 5  then (R,G,B) = (V,p,q)
 *
 *
 */
void HSV2RGB(const TColor* HSV, TColor* RGB) {

}

/**
 *
 */
void White2RGB(const uint16_t Temp, TColor* RGB) {
/*
  float F;
  double d;

  F = logf(12.345*Temp);
*/


}
