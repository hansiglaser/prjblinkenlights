/*
 * infomem.h
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#ifndef INFOMEM_H_
#define INFOMEM_H_

#include <stdint.h>
#include <stdbool.h>

#include "color.h"

#define MODE_OFF      0x00
#define MODE_WHITE    0x01
#define MODE_RGB      0x02
#define MODE_HSV      0x03
#define MODE_RAINBOW  0x04

typedef struct {
  uint8_t Version;
  uint8_t Mode;           ///< use MODE_*
  TColor RGB;
  TColor HSV;
  uint16_t ColorTemp;
  uint16_t Intensity;
  uint16_t RainbowSpeed;
  uint16_t RainbowSaturation;
  uint16_t RainbowValue;
} TPersistent;  // attribute "packed" seems not to be supported :-(

extern TPersistent PersistentRam;

void infomem_init();
void infomem_read();
void infomem_write();

#endif /* INFOMEM_H_ */
