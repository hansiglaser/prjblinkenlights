/*
 * menu.h
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#ifndef MENU_H_
#define MENU_H_

#include <stdint.h>

#define MENU_MAX_LEVELS 3

#define MENU_NUM_ROWS 4

/****************************************************************************
 **** Menu Definitions ******************************************************
 ****************************************************************************/

typedef enum {metSimple,metSubmenu,metReturn,metNumber,metString} TMenuEntryType;

typedef int (*TMenuSimpleCallback)(void* Data);
typedef int (*TMenuNumberValueCallback)(int Delta,void* Data);
typedef void (*TMenuNumberChangeCallback)();
typedef void (*TMenuSubmenuCallback)();

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
      TMenuSubmenuCallback CBEnter;
      TMenuSubmenuCallback CBExit;
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

/****************************************************************************
 **** Stock Callback Functions **********************************************
 ****************************************************************************/

int cbPercent(int Delta, void* Data);
int cbPercent16bit(int Delta, void* Data);
int cbCircle(int Delta, void* Data);
int cbCircle16bit(int Delta, void* Data);

/****************************************************************************
 **** Internal Data Structures **********************************************
 ****************************************************************************/

#define SUBMENU_STATE_FLAG_EDIT   0x01 ///< editing an metNumber or metString entry

typedef struct {
  const TMenuEntry* Menu;  ///< points to a TMenuEntry[]
  int Item;   ///< Index within the menu
  int First;  ///< Index within the menu of the first line shown on the display
  int Count;  ///< number of menu items in this submenu
  uint16_t Flags;   ///< see SUBMENU_STATE_FLAG_*
} TSubmenuState;

typedef struct {
  TSubmenuState MenuStack[MENU_MAX_LEVELS];
  int MenuStackIndex;
} TMenuState;

/****************************************************************************
 **** Menu Handling *********************************************************
 ****************************************************************************/

typedef enum {mePress,meBack,meRotate} TMenuEvent;

void menu_init(const TMenuEntry* Main, int Count, TMenuState* State);
void menu_draw(const TMenuState* State);
void menu_handle_event(TMenuState* State, TMenuEvent Event, int Rotate);

#endif /* MENU_H_ */
