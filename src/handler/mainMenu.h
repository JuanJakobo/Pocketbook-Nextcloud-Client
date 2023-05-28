#pragma once
//------------------------------------------------------------------
// mainMenu.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      Handles the menubar and the menu
//-------------------------------------------------------------------
#include <string>

#include "inkview.h"

enum class MainMenuOption
{
    ActualizeCurrentFolder,
    Logout,
    SortBy,
    ExcludeFiles,
    ChooseFolder,
    Info,
    Exit
};

class MainMenu
{
  public:
    /**
     * Defines fonds, sets global Event Handler and starts new content
     *
     * @param name name of the application
     */
    MainMenu(const std::string &p_name);

    ~MainMenu() = default;

    const irect &getContentRect() const
    {
        return m_contentRect;
    };
    const irect &getMenuButtonRect() const
    {
        return m_menuButtonRect;
    };

    /**
     * Shows the menu on the screen, lets the user choose menu options and then
     * redirects the handler to the caller
     *
     * @param filePicker true if the filepicker is shown
     * @param loogedIn the status if the user is logged in
     * @param handler handles the clicks on the menu
     */
    void drawMenu(bool p_filePicker, bool p_loggedIn, iv_menuhandler p_menu_handler) const;

  private:
    uint16_t m_menuPanelBeginX;
    uint16_t m_menuPanelBeginY;
    irect m_contentRect;
    irect m_menuButtonRect;

    /**
     * Functions needed to call C function, handles the panel
     */
    static void panelHandlerStatic();
};
