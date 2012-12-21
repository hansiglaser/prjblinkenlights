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
// this lookup table was generated using Brightness2PWM.m
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
 * if H < 0° then H = H + 360°
 *
 * if MAX = 0 then S = 0    // -> R = G = B = 0
 * else S = (MAX-MIN)/MAX
 *
 * V = MAX
 *
 */
void RGB2HSV(const TColor* RGB, TColor* HSV) {
  uint16_t Min = RGB->RGB.R;
  if (RGB->RGB.G < Min) Min = RGB->RGB.G;
  if (RGB->RGB.B < Min) Min = RGB->RGB.B;
  uint8_t MaxX = 0;
  uint16_t Max = RGB->RGB.R;
  if (RGB->RGB.G > Max) { Max = RGB->RGB.G; MaxX = 1; }
  if (RGB->RGB.B > Max) { Max = RGB->RGB.B; MaxX = 2; }
#if COLOR_FLOAT == 0
  if (Min == Max) {
    HSV->HSV.H = 0;
    HSV->HSV.S = 0;
  } else {
    uint32_t Scale = (uint32_t)715827883 / (Max-Min);  // 65536*65536/6;
    int32_t Diff;
    switch (MaxX) {
      case 0:  Diff = RGB->RGB.G - RGB->RGB.B; break;
      case 1:  Diff = RGB->RGB.B - RGB->RGB.R; break;
      default: Diff = RGB->RGB.R - RGB->RGB.G; break;
    }
    HSV->HSV.H = (((uint32_t)Scale * Diff + 0x7FFF) >> 16);
    switch (MaxX) {
      case 0:  break;
      case 1:  HSV->HSV.H += 65536/3;   break;
      default: HSV->HSV.H += 65536*2/3; break;
    }

    HSV->HSV.S = ((uint32_t)(Max-Min) * 65535) / Max;
  }

#else
  float Scale = (65536.0/6.0) / ((Max-Min)*1.0);

  if (Min == Max) {
    HSV->HSV.H = 0;
  } else if (Max == RGB->RGB.R) {
    HSV->HSV.H = floor((RGB->RGB.G - RGB->RGB.B)*1.0 * Scale);
  } else if (Max == RGB->RGB.G) {
    HSV->HSV.H = floor((RGB->RGB.B - RGB->RGB.R)*1.0 * Scale) + (65536/3);
  } else /*  Max == RGB->RGB.B */ {
    HSV->HSV.H = floor((RGB->RGB.R - RGB->RGB.G)*1.0 * Scale) + (65536/3)*2;
  }

  if (Max == 0) {
    HSV->HSV.S = 0;
  } else {
    HSV->HSV.S = floor(((Max-Min)*65535.0)/(Max*1.0));
  }

#endif // COLOR_FLOAT == 0
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
#if COLOR_FLOAT == 0
  uint32_t fi = ((uint32_t)HSV->HSV.H << 16) / (10923);   // 65536/6
  int hi = fi >> 16; //& 0xFFFF0000;
  uint32_t f = fi & 0x0000FFFF;

  int32_t p = HSV->HSV.V * (65536 - HSV->HSV.S) >> 16;
  int32_t q = (HSV->HSV.V * (65536 - (int32_t)((((uint32_t)HSV->HSV.S*f        ) + 0x7FFF) >> 16)) + 0x7FFF) >> 16;
  int32_t t = (HSV->HSV.V * (65536 - (int32_t)((((uint32_t)HSV->HSV.S*(65536-f)) + 0x7FFF) >> 16)) + 0x7FFF) >> 16;
  uint16_t V = HSV->HSV.V;
#else
  float f = ((HSV->HSV.H*1.0) / (65536.0/6.0));
  int hi = floor(f);
  f = f - hi;
  int32_t p = HSV->HSV.V * (65536 - HSV->HSV.S) >> 16;
  int32_t q = HSV->HSV.V * (65536 - (int32_t)floor((HSV->HSV.S*1.0)*f)) >> 16;
  int32_t t = HSV->HSV.V * (65536 - (int32_t)floor((HSV->HSV.S*1.0)*(1.0-f))) >> 16;
  int16_t V = HSV->HSV.V;
#endif // COLOR_FLOAT == 0

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

#if COLOR_FLOAT == 0
// these lookup tables were generated using White2RGB.m
#define WHITE2RGB_VALUES_BITS    6
#define WHITE2RGB_VALUES_START   430
#define WHITE2RGB_VALUES_SHIFT   (16 - WHITE2RGB_VALUES_BITS)
#define WHITE2RGB_VALUES_MASK    ((1 << WHITE2RGB_VALUES_SHIFT)-1)
#define WHITE2RGB_VALUES_COUNT   39
const uint16_t White2RGBRed[WHITE2RGB_VALUES_COUNT] = {
  65535, 65535, 65535, 65535, 65535, 65535, 65352, 59722, 55764, 52898, 50737, 49061, 47724, 46637, 45739, 44986,
  44343, 43794, 43316, 42896, 42528, 42200, 41906, 41644, 41406, 41189, 40991, 40812, 40645, 40492, 40350, 40222,
  40096, 39986, 39881, 39781, 39686, 39602, 39520
};
const uint16_t White2RGBGreen[WHITE2RGB_VALUES_COUNT] = {
    429, 26946, 40822, 50017, 56258, 60739, 63886, 60688, 58346, 56609, 55274, 54224, 53378, 52683, 52103, 51612,
  51192, 50831, 50513, 50236, 49992, 49773, 49575, 49395, 49237, 49091, 48957, 48835, 48722, 48621, 48524, 48434,
  48351, 48276, 48202, 48134, 48071, 48013, 47956
};
const uint16_t White2RGBBlue[WHITE2RGB_VALUES_COUNT] = {
      0,     0, 18350, 35053, 47752, 57714, 65421, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535,
  65535, 65535, 65535, 65535, 65535, 65535, 65535
};
#endif // COLOR_FLOAT == 0

/**
 * Create an RGB color from a color temperature
 *
 * @param  Temp  color temperature in Kelvin, 1000-40000
 * @param  RGB   resulting RGB value
 *
 * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
 */
void White2RGB(uint16_t Temp, TColor* RGB) {
#if COLOR_FLOAT == 0
  if (Temp < 1000) {
    return;
  }
  Temp = Temp - WHITE2RGB_VALUES_START;

  uint16_t Index = Temp >> WHITE2RGB_VALUES_SHIFT;
  if (Index >= WHITE2RGB_VALUES_COUNT-1) {
    return;
  }
  uint16_t Inter = Temp &  WHITE2RGB_VALUES_MASK;
  uint16_t a,b;
  int32_t d,y;

  // red
  a = White2RGBRed[Index];
  b = White2RGBRed[Index + 1];
  d = b-a;
  y = (uint32_t)d * (uint32_t)Inter + (WHITE2RGB_VALUES_MASK >> 1);  // round
  RGB->RGB.R = a + (y >> WHITE2RGB_VALUES_SHIFT);

  // green
  a = White2RGBGreen[Index];
  b = White2RGBGreen[Index + 1];
  d = b-a;
  y = (uint32_t)d * (uint32_t)Inter + (WHITE2RGB_VALUES_MASK >> 1);  // round
  RGB->RGB.G = a + (y >> WHITE2RGB_VALUES_SHIFT);

  // blue
  if (Temp < 1900-WHITE2RGB_VALUES_START) {
    RGB->RGB.B = 0;
  } else {
    a = White2RGBBlue[Index];
    b = White2RGBBlue[Index + 1];
    d = b-a;
    y = (uint32_t)d * (uint32_t)Inter + (WHITE2RGB_VALUES_MASK >> 1);  // round
    RGB->RGB.B = a + (y >> WHITE2RGB_VALUES_SHIFT);
  }
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
