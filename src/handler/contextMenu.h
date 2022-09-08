//------------------------------------------------------------------
// contextMenu.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      Handles the menubar and the menu
//-------------------------------------------------------------------

#ifndef CONTEXT_MENU
#define CONTEXT_MENU

#include "inkview.h"
#include "webDAVModel.h"

#include <string>

class ContextMenu
{
public:
    ContextMenu();

    ~ContextMenu();

    /**
        * Shows the menu on the screen, lets the user choose menu options and then redirects the handler to the caller
        *
        * @param y y-coordinate of the item
        * @param FileState status of the item for that the menu is created
        * @param handler  which action does the menu buttons start
        * @return int returns if the event was handled
        */
    int createMenu(int y, FileState itemstate, iv_menuhandler handler);

private:
    char *_menu = strdup("Menu");
    char *_open = strdup("Open");
    char *_sync = strdup("Sync");
    char *_remove = strdup("Remove local");
};
#endif
