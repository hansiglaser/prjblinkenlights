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
  int Min = RGB->RGB.R;
  if (RGB->RGB.G < Min) Min = RGB->RGB.G;
  if (RGB->RGB.B < Min) Min = RGB->RGB.B;
  int Max = RGB->RGB.R;
  if (RGB->RGB.G > Max) Max = RGB->RGB.G;
  if (RGB->RGB.B > Max) Max = RGB->RGB.B;
  float Scale = (65536.0/6.0) / ((Max-Min)*1.0);

  if (Min == Max) {
    HSV->HSV.H = 0;
  } else if (Max == RGB->RGB.R) {
    HSV->HSV.H = floor((RGB->RGB.G - RGB->RGB.B)*1.0 * Scale);
  } else if (Max == RGB->RGB.G) {
    HSV->HSV.H = floor((RGB->RGB.B - RGB->RGB.R)*1.0 * Scale) + (65536/3);
  } else /*  Max == RGB->RGB.R */ {
    HSV->HSV.H = floor((RGB->RGB.R - RGB->RGB.G)*1.0 * Scale) + (65536/3)*2;
  }

  if (Max == 0) {
    HSV->HSV.S = 0;
  } else {
    HSV->HSV.S = floor(((Max-Min)*65536.0)/(Max*1.0));
  }

  HSV->HSV.V = Max;
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
  float f = ((HSV->HSV.H*1.0) / (65536.0/6.0));
  int hi = floor(f);
  f = f - hi;
  int32_t p = HSV->HSV.V * (65536 - HSV->HSV.S) >> 16;
  int32_t q = HSV->HSV.V * (65536 - (int32_t)floor((HSV->HSV.S*1.0)*f)) >> 16;
  int32_t t = HSV->HSV.V * (65536 - (int32_t)floor((HSV->HSV.S*1.0)*(1.0-f))) >> 16;
  int16_t V = HSV->HSV.V;

  switch (hi) {
  case 0:
  case 6:
    RGB->RGB.R = V; RGB->RGB.G = t; RGB->RGB.B = p; break;
  case 1:
    RGB->RGB.R = q; RGB->RGB.G = V; RGB->RGB.B = p; break;
  case 2:
    RGB->RGB.R = p; RGB->RGB.G = V; RGB->RGB.B = t; break;
  case 3:
    RGB->RGB.R = p; RGB->RGB.G = q; RGB->RGB.B = V; break;
  case 4:
    RGB->RGB.R = t; RGB->RGB.G = p; RGB->RGB.B = V; break;
  case 5:
    RGB->RGB.R = V; RGB->RGB.G = p; RGB->RGB.B = q; break;
  }
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
