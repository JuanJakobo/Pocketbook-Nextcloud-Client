//------------------------------------------------------------------
// contextMenu.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "contextMenu.h"
#include "inkview.h"

namespace{
    constexpr auto MENU_TITLE{"Menu"};
    constexpr auto OPEN_TITLE{"Open"};
    constexpr auto REMOVE_TITLE{"Remove Local"};
    constexpr auto SYNC_TITLE{"Sync"};
    constexpr auto HEADER{0};
}


ContextMenu::ContextMenu() {}

void ContextMenu::createMenu(int p_yLocation, FileState p_fileState,
                            iv_menuhandler p_handler) const {
  imenu contextMenu[] = {{ITEM_HEADER, HEADER, const_cast<char*>(MENU_TITLE), NULL},
                         {(p_fileState != FileState::ICLOUD) ? (short)ITEM_ACTIVE
                                                           : (short)ITEM_HIDDEN,
                          static_cast<short>(ContextMenuOption::Open), const_cast<char*>(OPEN_TITLE), NULL},
                         {ITEM_ACTIVE, static_cast<short>(ContextMenuOption::Sync), const_cast<char*>(SYNC_TITLE), NULL},
                         {(p_fileState != FileState::ICLOUD) ? (short)ITEM_ACTIVE
                                                           : (short)ITEM_HIDDEN,
                          static_cast<short>(ContextMenuOption::Remove), const_cast<char*>(REMOVE_TITLE), NULL},
                         {0, 0, NULL, NULL}};

  OpenMenu(contextMenu, 0, ScreenWidth(), p_yLocation, p_handler);
}
