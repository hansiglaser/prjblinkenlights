/*
 * menu.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#include "menu.h"

/****************************************************************************
 **** Stock Callback Functions **********************************************
 ****************************************************************************/

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

/****************************************************************************
 **** Menu Handling *********************************************************
 ****************************************************************************/

void menu_init(const TMenuEntry* Main, int Count, TMenuState* State) {
  State->MenuStackIndex = 0;
  State->MenuStack[0].Menu  = Main;
  State->MenuStack[0].Item  = 0;
  State->MenuStack[0].First = 0;
  State->MenuStack[0].Count = Count;
}

void menu_draw_entry(const TMenuEntry* Entry) {
// TODO:  lcd_print(Entry->Label);
  switch (Entry->Type) {
  case metSimple:
    // TODO
    break;
  case metSubmenu:
    // TODO
    break;
  case metReturn:
    // TODO
    break;
  case metNumber:
    // TODO
    break;
  case metString:
    // TODO
    break;
  }
}

void menu_draw(const TMenuState* State) {
  const TSubmenuState* SubState = State->MenuStack + State->MenuStackIndex;
  const TMenuEntry* Menu  = SubState->Menu;
  int Row;

  for (Row = 0; Row < MENU_NUM_ROWS; Row++) {
    const TMenuEntry* Entry = Menu + Row;
    // TODO: lcd_gotoxy(0,Row);
    menu_draw_entry(Entry);
  }

  // TODO: draw scroll bar

  // TODO: mark current menu entry (e.g. with cursor)

}

void menu_handle_event(TMenuState* State, TMenuEvent Event, int Rotate) {
  TSubmenuState* SubState = State->MenuStack + State->MenuStackIndex;
  const TMenuEntry* Menu  = SubState->Menu;
  const TMenuEntry* Entry = Menu + SubState->Item;

  if (1) {
    // normal menu operation
    switch (Event) {
    case mePress:
      switch (Entry->Type) {
      case metSimple:
        Entry->SimpleData.Callback(Entry->SimpleData.CBData);
        // TODO: should we exit the menu here?
        break;
      case metSubmenu:
        // enter submenu
        if (State->MenuStackIndex >= MENU_MAX_LEVELS-1) {
          break;   // ERROR! to deep hierarchy!
        }
        State->MenuStackIndex++;
        State->MenuStack[State->MenuStackIndex].Menu  = Entry->SubMenuData.SubMenu;
        State->MenuStack[State->MenuStackIndex].Item  = 0;
        State->MenuStack[State->MenuStackIndex].First = 0;
        State->MenuStack[State->MenuStackIndex].Count = Entry->SubMenuData.NumEntries;
        menu_draw(State);
        break;
      case metReturn:
        // return from submenu
        if (State->MenuStackIndex == 0)
          break;  // already at top level, can't return from submenu
        State->MenuStackIndex--;
        menu_draw(State);
        break;
      case metNumber:
        // TODO
        break;
      case metString:
        // TODO
        break;
      }
      break;
    case meBack:
      // return from submenu
      if (State->MenuStackIndex == 0)
        break;  // already at top level, can't return from submenu
      State->MenuStackIndex--;
      menu_draw(State);
      break;
    case meRotate:
      if ((Rotate > 0) && (SubState->Item > 0)) {
        SubState->Item--;
        if (SubState->First > SubState->Item) {
          SubState->First = SubState->Item;
          menu_draw(State);
        } else {
          // TODO: shift marker of current menu entry
        }
      } else if ((Rotate < 0) && (SubState->Item < SubState->Count-1)) {
        SubState->Item++;
        if (SubState->First+MENU_NUM_ROWS < SubState->Item) {
          SubState->First = SubState->Item-MENU_NUM_ROWS;
          menu_draw(State);
        } else {
          // TODO: shift marker of current menu entry
        }
      }
      break;
    }
  } else {
    // interacting with the menu entry
    // TODO
  }
}
