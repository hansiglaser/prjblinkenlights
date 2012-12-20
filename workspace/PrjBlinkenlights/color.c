/*
 * color.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#include <math.h>
#include <stdint.h>
#include "color.h"

#define COLOR_FLOAT 0

#if COLOR_FLOAT == 0
#define BRIGHTNESS2PWM_VALUES_BITS    5
#define BRIGHTNESS2PWM_VALUES_SHIFT   (16 - BRIGHTNESS2PWM_VALUES_BITS)
#define BRIGHTNESS2PWM_VALUES_MASK    ((1 << BRIGHTNESS2PWM_VALUES_SHIFT)-1)
#define BRIGHTNESS2PWM_VALUES_COUNT   ((1 << BRIGHTNESS2PWM_VALUES_BITS) + 1)
const uint16_t Brightness2PWMValues[BRIGHTNESS2PWM_VALUES_COUNT] = {
      0,    21,    47,    79,   119,   167,   226,   299,   388,   497,   632,   796,   999,  1247,  1551,  1925,
   2384,  2947,  3638,  4487,  5527,  6805,  8373, 10297, 12659, 15557, 19114, 23480, 28837, 35413, 43483, 53387,
  65535
};
#endif // COLOR_FLOAT == 0

/**
 * Convert intensity to PWM with non-linear function
 */
uint16_t Brightness2PWM(uint16_t Brightness) {
#if COLOR_FLOAT == 0
  uint16_t Index = Brightness >> BRIGHTNESS2PWM_VALUES_SHIFT;
  uint16_t Inter = Brightness &  BRIGHTNESS2PWM_VALUES_MASK;
  uint16_t a = Brightness2PWMValues[Index];
  uint16_t b = Brightness2PWMValues[Index + 1];

  uint16_t d = b-a;
  uint32_t y = (uint32_t)d * (uint32_t)Inter + (BRIGHTNESS2PWM_VALUES_MASK >> 1);  // round
  uint16_t z = a + (y >> BRIGHTNESS2PWM_VALUES_SHIFT);

  return z;
#else
  float x;
  uint16_t y;
  if (Brightness == 0) return 0;
  if (Brightness == 0xFFFF) return 0xFFFF;
  // TODO: find a better fixed-point solution (this adds ~4kB to the executable!)
  x = expf(Brightness*0.0001);
  y = floor((x-1.0) * (65535.0 / (exp(6.5535)-1.0)));
  return y;
#endif // COLOR_FLOAT == 0
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
#if COLOR_FLOAT == 0
#else
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
    HSV->HSV.S = floor(((Max-Min)*65535.0)/(Max*1.0));
  }

  HSV->HSV.V = Max;
#endif // COLOR_FLOAT == 0
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
#if COLOR_FLOAT == 0
#else
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
#endif // COLOR_FLOAT == 0
}

/**
 * Create an RGB color from a color temperature
 *
 * @param  Temp  color temperature in Kelvin, 1000-40000
 * @param  RGB   resulting RGB value
 *
 * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
 */
void White2RGB(const uint16_t Temp, TColor* RGB) {
#if COLOR_FLOAT == 0
#else
  float R,G,B;
  int32_t Ri,Gi,Bi;

  if (Temp <= 6600) {
    RGB->RGB.R = 0xFFFF;
  } else {
    R = 329.698727446/255.0*65535.0 * powf((Temp - 6000)*0.01,-0.1332047592);
    Ri = (int32_t)R;
    if      (Ri < 0)      RGB->RGB.R = 0;
    else if (Ri > 0xFFFF) RGB->RGB.R = 0xFFFF;
    else                  RGB->RGB.R = Ri;
  }

  if (Temp <= 6600) {
    G = 99.4708025861/255.0*65535.0 * logf(Temp*0.01) - 161.1195681661/255.0*65535.0;
  } else {
    G = 288.1221695283/255.0*65535.0 * powf((Temp - 6000)*0.01,-0.0755148492);
  }
  Gi = (int32_t)G;
  if      (Gi < 0)      RGB->RGB.G = 0;
  else if (Gi > 0xFFFF) RGB->RGB.G = 0xFFFF;
  else                  RGB->RGB.G = Gi;

  if (Temp >= 6600) {
    RGB->RGB.B = 0xFFFF;
  } else if (Temp <= 1900) {
    RGB->RGB.B = 0;
  } else {
    B = 138.5177312231/255.0*65535.0 * logf((Temp-1000)*0.01) - 305.0447927307/255.0*65535.0;
    Bi = (int32_t)B;
    if      (Bi < 0)      RGB->RGB.B = 0;
    else if (Bi > 0xFFFF) RGB->RGB.B = 0xFFFF;
    else                  RGB->RGB.B = Bi;
  }
#endif // COLOR_FLOAT == 0
}
