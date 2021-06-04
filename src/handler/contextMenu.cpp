//------------------------------------------------------------------
// contextMenu.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "contextMenu.h"

#include <string>

using std::string;

ContextMenu::ContextMenu()
{
}

ContextMenu::~ContextMenu()
{
    free(_menu);
    free(_open);
    free(_sync);
    free(_remove);
}

int ContextMenu::createMenu(int y, FileState itemstate, iv_menuhandler handler)
{
    imenu contextMenu[] =
        {
            {ITEM_HEADER, 0, _menu, NULL},
            {(itemstate != FileState::ICLOUD) ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 101, _open, NULL},
            {ITEM_ACTIVE, 102, _sync, NULL},
            {(itemstate != FileState::ICLOUD) ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 103, _remove, NULL},
            {0, 0, NULL, NULL}};

    OpenMenu(contextMenu, 0, ScreenWidth(),y, handler);

    return 1;
}