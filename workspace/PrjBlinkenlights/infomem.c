/**
 * infomem.c
 *
 * See also:
 *  - http://www.mikrocontroller.net/articles/MSP430_Codebeispiele#Persistente_Daten_im_Information_Memory
 */

#include <msp430g2553.h>

#include "infomem.h"

/**
 * Persistent data structure in Info Memory
 *
 * Note: regardless whether the variable is initialized or not, the ELF file
 * will contain initialization data (which defaults to all zeros if no values
 * are given here).
 */
TPersistent PersistentFlash __attribute__((section(".infomem"))) = {
  .Version           = 0,
  .Mode              = MODE_OFF,
  .LCDTimeout        = 10,                     // seconds
  .ColorTemp         = 25,                     // 6000K
  .Intensity         = 0x8000,                 // 50% intensity
  .RGB               = {.RGB.R =   0, .RGB.G =      0, .RGB.B = 0x8000},
  .HSV               = {.HSV.V =   0, .HSV.S = 0xFFFF, .HSV.V = 0x8000},
  .RainbowSpeed      = (65536*20+32768)/100,   // 20% (rounded)
  .RainbowSaturation = 0xFFFF,                 // 100% saturation
  .RainbowValue      = 0x8000,                 // 50% intensity
};

/**
 * The same data in RAM
 *
 * Note: No initialization data is stored in the ELF file.
 */
TPersistent PersistentRam;

/**
 * Setup Flash controller
 */
void infomem_init() {
  FCTL2 = FWKEY | FSSEL_2 | 39;     // Clk source is SMCLK, 16MHz / (39+1) = 400kHz, is within 257-467kHz
}

/**
 * Read data from Info Memory to RAM
 */
void infomem_read() {
  PersistentRam = PersistentFlash;
}

/**
 * Write data from RAM to Info Memory
 */
void infomem_write() {
  FCTL3 = FWKEY | 0;                 // unset LOCK bit
  FCTL1 = FWKEY | ERASE;             // set ERASE mode
  *((uint8_t*)&PersistentFlash) = 0; // initiate erase of the segment, 4819 cycles ~ 12ms, CPU is halted
  FCTL1 = FWKEY | WRT;               // set WRITE mode
  PersistentFlash = PersistentRam;   // byte wise or word wise write, 30 cycles ~ 75us per byte/word, must not exceed 10ms in total!
  FCTL1 = FWKEY | 0;                 // disable WRITE mode
  FCTL3 = FWKEY | LOCK;              // set LOCK bit
}
