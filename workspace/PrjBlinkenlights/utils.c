/*
 * utils.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */


#include "utils.h"

typedef union {
  uint32_t Full;
  struct {
    uint16_t Int;
    uint16_t BCD;
  };
} TInt2BCDHelper;

uint16_t Int2BCD(uint16_t i) {
  TInt2BCDHelper Scratch;
  int n;

  Scratch.Int = i;
  Scratch.BCD = 0;

  for (n=0; n<16;n++) {
    if ((Scratch.BCD & 0x000F) > 0x0004) Scratch.BCD += 0x0003;
    if ((Scratch.BCD & 0x00F0) > 0x0040) Scratch.BCD += 0x0030;
    if ((Scratch.BCD & 0x0F00) > 0x0400) Scratch.BCD += 0x0300;
    if ((Scratch.BCD & 0xF000) > 0x4000) Scratch.BCD += 0x3000;
    Scratch.Full = Scratch.Full << 1;
  }
  return Scratch.BCD;
}

/**
 * TODO: first find which justification is required
 */
void Int2Str(int i, char* s) {
  // convert to BCD
  i = Int2BCD(i);
  // make a string
  *s += 4;
  do {
    *s-- = (i & 0x000F) + '0';
    i = i >> 4;
  } while (i > 0);
}
