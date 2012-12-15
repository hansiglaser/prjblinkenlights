/*
 * menu.h
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>

typedef enum {metSimple,metSubmenu,metNumber,metString} TMenuEntryType;

typedef int (*TMenuSimpleCallback)(void* Data);
typedef int (*TMenuNumberValueCallback)(int Delta,void* Data);
typedef void (*TMenuNumberChangeCallback)();

//typedef struct TMenuEntry TMenuEntry;

typedef struct {
  TMenuEntryType Type;
  char Label[14];
  union {
    struct {
      TMenuSimpleCallback Callback;
      void* CBData;
    } SimpleData;
    struct {
      int NumEntries;
      void* SubMenu;
    } SubMenuData;
    struct {
      char Unit;
      TMenuNumberValueCallback CBValue;
      void* CBData;
      TMenuNumberChangeCallback CBChange;
    } NumberData;
    struct {
      char* String;
      uint8_t Length;
      // TODO: Callbacks
    } StringData;
  };
} TMenuEntry;

int cbPercent(int Delta, void* Data);
int cbCircle(int Delta, void* Data);

#endif /* MENU_H_ */
