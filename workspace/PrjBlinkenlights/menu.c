/*
 * menu.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#include "menu.h"

int cbPercent(int Delta, void* Data) {
  if (Delta == 0)
    return *((int*)Data);

  int i = *((int*)Data);
  i += Delta;
  if (i < 0)
    i = 0;
  if (i > 100)
    i = 100;
  *((int*)Data) = i;
  return i;
}

int cbCircle(int Delta, void* Data) {
  if (Delta == 0)
    return *((int*)Data);

  int i = *((int*)Data);
  i += Delta;
  while (i < 0)
    i += 360;
  while (i > 360)
    i -= 360;
  *((int*)Data) = i;
  return i;
}

