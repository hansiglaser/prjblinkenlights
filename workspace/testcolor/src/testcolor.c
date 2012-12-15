/*
 ============================================================================
 Name        : testcolor.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include <gtk/gtk.h>

#define MAXDIFF 10

#include "color.h"

typedef double gdouble;

void       gtk_hsv_to_rgb       (gdouble    h,
                                 gdouble    s,
                                 gdouble    v,
                                 gdouble   *r,
                                 gdouble   *g,
                                 gdouble   *b);
void       gtk_rgb_to_hsv       (gdouble    r,
                                 gdouble    g,
                                 gdouble    b,
                                 gdouble   *h,
                                 gdouble   *s,
                                 gdouble   *v);

int main(void) {
  gdouble h,s,v,r,g,b;
  uint16_t H,S,V,R,G,B;
  TColor RGB;
  TColor HSV;
  int Hi,Si,Vi,Ri,Gi,Bi;
  int T;
  int Errors = 0;
  int Error;
  int TotalError = 0;

  // Test HSV2RGB
  for (Vi = 0; Vi <= 100; Vi++) {
    v = Vi*1.0/100.0;
    for (Si = 0; Si <= 100; Si++) {
      s = Si*1.0/100.0;
      for (Hi = 0; Hi < 360; Hi++) {
        h = Hi*1.0/360.0;
        gtk_hsv_to_rgb(h,s,v,&r,&g,&b);
        H = round(h*65535.0);
        S = round(s*65535.0);
        V = round(v*65535.0);
        R = round(r*65535.0);
        G = round(g*65535.0);
        B = round(b*65535.0);
        HSV.HSV.H = H;
        HSV.HSV.S = S;
        HSV.HSV.V = V;
        HSV2RGB(&HSV,&RGB);
        Error = abs((int)RGB.RGB.R-(int)R) + abs((int)RGB.RGB.G-(int)G) + abs((int)RGB.RGB.B-(int)B);
        if (Error > MAXDIFF) {
          printf("HSV2RGB %3d %3d %3d: %5d %5d %5d -> %5d %5d %5d",
              Hi,Si,Vi,
              H,S,V,
              RGB.RGB.R,RGB.RGB.G,RGB.RGB.B);
          printf(" (should be %5d %5d %5d, Error = %d)",R,G,B,Error);
          printf("\n");
          TotalError += Error;
          Errors++;
        }
      }
    }
  }

  // Test RGB2HSV
  for (Bi = 0; Bi <= 100; Bi++) {
    b = Bi*1.0/100.0;
    for (Gi = 0; Gi <= 100; Gi++) {
      g = Gi*1.0/100.0;
      for (Ri = 0; Ri <= 100; Ri++) {
        r = Ri*1.0/100.0;
        gtk_rgb_to_hsv(r,g,b,&h,&s,&v);
        R = round(r*65535.0);
        G = round(g*65535.0);
        B = round(b*65535.0);
        H = round(h*65535.0);
        S = round(s*65535.0);
        V = round(v*65535.0);
        RGB.RGB.R = R;
        RGB.RGB.G = G;
        RGB.RGB.B = B;
        RGB2HSV(&RGB,&HSV);
        Error = abs((int)HSV.HSV.H-(int)H) + abs((int)HSV.HSV.S-(int)S) + abs((int)HSV.HSV.V-(int)V);
        if (Error > MAXDIFF) {
          printf("RGB2HSV %3d %3d %3d: %5d %5d %5d -> %5d %5d %5d",
              Ri,Gi,Bi,
              R,G,B,
              HSV.HSV.H,HSV.HSV.S,HSV.HSV.V);
          printf(" (should be %5d %5d %5d, Error = %d)",H,S,V,Error);
          printf("\n");
          TotalError += Error;
          Errors++;
        }
      }
    }
  }

  printf("<pre>\n");
  for (T = 1000; T <= 40000; T+=100) {
    White2RGB(T,&RGB);
    printf("<span style=\"background:#%02x%02x%02x\"> %5d K -> %5d %5d %5d - #%02x%02x%02x</span>\n",
        RGB.RGB.R >> 8,RGB.RGB.G >> 8,RGB.RGB.B >> 8,
        T,
        RGB.RGB.R,RGB.RGB.G,RGB.RGB.B,
        RGB.RGB.R >> 8,RGB.RGB.G >> 8,RGB.RGB.B >> 8);
  }
  printf("</pre>\n");

  printf("%d errors found, total deviation is %d.\n",Errors,TotalError);
  return (Errors == 0 ? 0 : 1);
}
