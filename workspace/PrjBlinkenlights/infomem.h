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

typedef struct {
  uint8_t Version;
  uint16_t Word1;
  uint16_t Word2;
} TPersistent;  // attribute "packed" seems not to be supported :-(

extern TPersistent PersistentRam;

void infomem_init();
void infomem_read();
void infomem_write();

#endif /* INFOMEM_H_ */
