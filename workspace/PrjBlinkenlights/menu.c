/*
 * menu.c
 *
 *  Created on: Dec 9, 2012
 *      Author: hansi
 */

#include "menu.h"
#include "lcd.h"

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
  while (i >= 360)
    i -= 360;
  *((int*)Data) = i;
  return i;
}

/****************************************************************************
 **** Menu Handling *********************************************************
 ****************************************************************************/

/**
 * Initialize the menu system
 *
 * @param  Main   pointer to main menu
 * @param  Count  number of entries in the main menu
 * @param  State  pointer to menu state variable
 */
void menu_init(const TMenuEntry* Main, int Count, TMenuState* State) {
  State->MenuStackIndex = 0;
  State->MenuStack[0].Menu  = Main;
  State->MenuStack[0].Item  = 0;
  State->MenuStack[0].First = 0;
  State->MenuStack[0].Count = Count;
  State->MenuStack[0].Flags = 0;
}

/*
 * Flags used for the parameter "Flags" of menu_draw_entry()
 */
#define DRAW_ENTRY_FLAG_SELECTED    0x01    ///< the entry is selected by the pointer
#define DRAW_ENTRY_FLAG_EDIT        0x02    ///< the entry is currently edited (only for metNumber and metString)

/**
 * Draw a menu entry
 *
 * @param  Row    LCD display row (0-based) where to draw the menu entry
 * @param  Entry  menu entry
 * @param  Flags  meta-info about the entry, see DRAW_ENTRY_FLAG_*
 */
void menu_draw_entry(const int Row, const TMenuEntry* Entry, uint8_t Flags) {
  LCDGotoXY(0,Row);
  // draw pointer
  LCDWrite(LCD_RS,(Flags & DRAW_ENTRY_FLAG_SELECTED ? '>' : ' '));
  // print main text
  LCDWriteString(Entry->Label);
  // print entry specific data
  switch (Entry->Type) {
  case metSimple:
    // nothing to do
    break;
  case metSubmenu:
    //LCDGotoXY(19,Row);
    LCDWrite(LCD_RS,' ');
    LCDWrite(LCD_RS,rarr);  // right arrow symbol
    break;
  case metReturn:
    //LCDGotoXY(19,Row);
    LCDWrite(LCD_RS,' ');
    LCDWrite(LCD_RS,larr);  // left arrow symbol
    break;
  case metNumber:
    LCDGotoXY(14,Row);
    // draw pointer
    LCDWrite(LCD_RS,(Flags & DRAW_ENTRY_FLAG_EDIT ? '>' : ' '));
    // get curent value
    int Value = Entry->NumberData.CBValue(0,Entry->NumberData.CBData);
    LCDWriteBCD(Int2BCD(Value));
    LCDWrite(LCD_RS,Entry->NumberData.Unit);
    break;
  case metString:
    // TODO
    break;
  }
}

/**
 * Draw pointer of currently selected menu entry
 *
 * This is a faster method than redrawing multiple menu entries.
 *
 * @param  MarkRow  LCD display row where to draw the marker, all others are removed
 */
void menu_mark_entry(int MarkRow) {
  int Row;
  for (Row = 0; Row < MENU_NUM_ROWS; Row++) {
    LCDGotoXY(0,Row);
    LCDWrite(LCD_RS,(Row == MarkRow ? '>' : ' '));
  }
}

/**
 * Draw the whole menu
 */
void menu_draw(const TMenuState* State) {
  const TSubmenuState* SubState = State->MenuStack + State->MenuStackIndex;
  const TMenuEntry* Menu  = SubState->Menu;
  int Row;

  // draw all menu entries including the pointer to the selected entry
  LCDClearScreen();
  for (Row = 0; Row < MENU_NUM_ROWS; Row++) {
    if (SubState->First + Row < SubState->Count) {
      const TMenuEntry* Entry = Menu + SubState->First + Row;
      menu_draw_entry(Row,Entry,(SubState->First + Row == SubState->Item ? DRAW_ENTRY_FLAG_SELECTED : 0));
    }
  }

  // TODO: draw scroll bar
}

/**
 * Handle key input events
 */
void menu_handle_event(TMenuState* State, TMenuEvent Event, int Rotate) {
  TSubmenuState* SubState = State->MenuStack + State->MenuStackIndex;
  const TMenuEntry* Menu  = SubState->Menu;
  const TMenuEntry* Entry = Menu + SubState->Item;

  if ((State->MenuStack[State->MenuStackIndex].Flags & SUBMENU_STATE_FLAG_EDIT) == 0) {
    /*
     * normal menu operation
     */
    switch (Event) {
    case mePress:
      switch (Entry->Type) {
      case metSimple:
        // menu entry was selected
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
        State->MenuStack[State->MenuStackIndex].Flags = 0;
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
        // menu entry was selected -> edit
        State->MenuStack[State->MenuStackIndex].Flags |= SUBMENU_STATE_FLAG_EDIT;
        // hide marker of current menu entry and show marker of current menu entry at edit position
        menu_draw_entry(SubState->Item - SubState->First,Entry,DRAW_ENTRY_FLAG_EDIT);
        break;
      case metString:
        // menu entry was selected -> edit
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
      // up/down
      if ((Rotate < 0) && (SubState->Item > 0)) {
        // up
        SubState->Item--;
        if (SubState->First > SubState->Item) {
          // scroll
          SubState->First = SubState->Item;
          menu_draw(State);
        } else {
          // shift marker of current menu entry
          menu_mark_entry(SubState->Item - SubState->First);
        }
      } else if ((Rotate > 0) && (SubState->Item < SubState->Count-1)) {
        // down
        SubState->Item++;
        if (SubState->Item >= SubState->First+MENU_NUM_ROWS) {
          // scroll
          SubState->First = SubState->Item-MENU_NUM_ROWS+1;
          menu_draw(State);
        } else {
          // shift marker of current menu entry
          menu_mark_entry(SubState->Item - SubState->First);
        }
      }
      break;
    }
  } else {
    /*
     * interacting with / edit the menu entry
     */
    switch (Event) {
    case mePress:
    case meBack:
      // done editing
      State->MenuStack[State->MenuStackIndex].Flags &= ~SUBMENU_STATE_FLAG_EDIT;
      switch (Entry->Type) {
      case metNumber:
        // redraw menu entry without the edit-marker but with the pointer
        menu_draw_entry(SubState->Item - SubState->First,Entry,DRAW_ENTRY_FLAG_SELECTED);
        break;
      case metString:
        // TODO: Entry->StringData.CBChange();
        break;
      default: break;  // this shouldn't happen anyway
      }
      break;
    case meRotate:
      // edit entry
      switch (Entry->Type) {
      case metNumber:
        Entry->NumberData.CBValue(Rotate,Entry->NumberData.CBData);
        Entry->NumberData.CBChange();
        break;
      case metString:
        // TODO: Entry->StringData.CBChange();
        break;
      default: break;  // this shouldn't happen anyway
      }
      // redraw menu entry with the edit-marker
      menu_draw_entry(SubState->Item - SubState->First,Entry,DRAW_ENTRY_FLAG_EDIT);
      break;
    }
  }
}
