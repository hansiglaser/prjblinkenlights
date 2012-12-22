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
  .Version = 0,
  .RGB = {.RGB.R = 0, .RGB.G = 0, .RGB.B = 0},
  .HSV = {.HSV.V = 0, .HSV.S = 0, .HSV.V = 0},
  .ColorTemp = 0,
  .Intensity = 0,
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
