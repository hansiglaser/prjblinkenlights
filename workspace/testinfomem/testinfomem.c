/**
 * Test MSP430 Info Memory
 *
 * http://www.mikrocontroller.net/articles/MSP430_Codebeispiele#Persistente_Daten_im_Information_Memory
 */

#include <msp430g2231.h>
#include <stdint.h>

typedef struct {
  uint8_t Version;
  uint16_t Word1;
  uint16_t Word2;
} TPersistent;  // attribute packed seems not to be supported :-(

// Persistent data structure in Info Memory
// Note: regardless whether the variable is initialized or not, the ELF file
// will contain initialization data (which defaults to all zeros if no values
// are given here).
TPersistent epersistent __attribute__((section(".infomem"))) = {
  1, 2, 3
};

TPersistent rpersistent;

int main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // Set DCO to 16 MHz
  DCOCTL = 0;              // Select lowest DCOx and MODx settings
  // RSEL = 15 -> 15.25MHz at DCO=3 ([DS] p. 23)
  BCSCTL1 = XT2OFF | (RSEL3 | RSEL2 | RSEL1 | RSEL0);
  DCOCTL  = DCO2;   // DCO = 4 -> 15.25 * 1.08 = 16.47MHz

  // Setup Flash controller
  FCTL2 = FWKEY | FSSEL_2 | 39;                // Clk source is SMCLK, 16MHz / (39+1) = 400kHz, is within 257-467kHz

  // get data from Info Memory to RAM
  rpersistent = epersistent;

  // fill with new values
  rpersistent.Version = 0x5A;
  rpersistent.Word1   = 0x5b79;
  rpersistent.Word2   = 0x1acb;

  // write data from RAM to Info Memory
  FCTL3 = FWKEY | 0;            // unset LOCK bit
  FCTL1 = FWKEY | ERASE;        // set ERASE mode
  epersistent.Version = 0;      // initiate erase of the segment, 4819 cycles ~ 12ms, CPU is halted
  FCTL1 = FWKEY | WRT;          // set WRITE mode
  epersistent=rpersistent;      // byte wise or word wise write, 30 cycles ~ 75us per byte/word, must not exceed 10ms in total!
  FCTL1 = FWKEY | 0;            // disable WRITE mode
  FCTL3 = FWKEY | LOCK;         // set LOCK bit

  while (1) {}

  // LPM3, effectively stopping execution
  LPM3;

  return 0;
}
