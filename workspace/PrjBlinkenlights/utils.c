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

/**
 * Convert an integer to a BCD value
 *
 * Note: only numbers from 0 to 9999 (i.e. 4 digits) are supported by the
 * output data type.
 *
 * The function uses the Double dabble algorithm (see
 * http://en.wikipedia.org/wiki/Double_dabble ) to avoid divisions.
 *
 * This uses a "scratch pad", which is the concatenation of the resulting
 * BCD value and the original integer value. The union TInt2BCDHelper is
 * introduced above to access this as one common 32-bit value as well as
 * individual Integer and BCD values.
 *
 * The algorithm works as follows: For each of the input bits, the whole
 * scratch pad is left-shifted. Before each shift, each BCD digit with a
 * value above 4 is incremented by 3. Therefore 5 (left-shifted equals 10)
 * is increased to 8 (left-shifted equals 16), and thus leading to a proper
 * carry to the next BCD digit.
 */
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
