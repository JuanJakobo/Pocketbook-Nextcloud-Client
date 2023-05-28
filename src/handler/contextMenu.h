#pragma once
//------------------------------------------------------------------
// contextMenu.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      Handles the menubar and the menu
//-------------------------------------------------------------------

#include "inkview.h"
#include "webDAVModel.h"

enum class ContextMenuOption
{
    Open,
    Sync,
    Remove
};

class ContextMenu
{
  public:
    ContextMenu();

    ~ContextMenu() = default;

    /**
     * Shows the menu on the screen, lets the user choose menu options and then
     * redirects the handler to the caller
     *
     * @param y y-coordinate of the item
     * @param FileState status of the item for that the menu is created
     * @param handler  which action does the menu buttons start
     */
    void createMenu(int p_yLocation, FileState p_fileState, iv_menuhandler p_handler) const;
};
