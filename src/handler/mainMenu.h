//------------------------------------------------------------------
// mainMenu.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      Handles the menubar and the menu
//-------------------------------------------------------------------

#ifndef MAIN_MENU
#define MAIN_MENU

#include "inkview.h"

#include <string>

class MainMenu
{
public:
    /**
        * Defines fonds, sets global Event Handler and starts new content
        *
        * @param name name of the application
        */
    MainMenu(const std::string &name);

    ~MainMenu();

    irect &getContentRect() { return _contentRect; };
    irect &getMenuButtonRect() { return _menuButtonRect; };

    /**
        * Shows the menu on the screen, lets the user choose menu options and then redirects the handler to the caller
        *
        * @param filePicker true if the filepicker is shown
        * @param loogedIn the status if the user is logged in
        * @param handler handles the clicks on the menu
        * @return int returns if the event was handled
        */
    int createMenu(bool filePicker, bool loggedIn, iv_menuhandler handler);

private:
    ifont *_menuFont;

    int _panelMenuBeginX;
    int _panelMenuBeginY;
    int _panelMenuHeight;
    int _mainMenuWidth;
    irect _menuButtonRect;

    imenu _mainMenu;
    irect _contentRect;

    char *_menu = strdup("Menu");
    char *_syncFolder = strdup("Actualize folder");
    char *_logout = strdup("Logout");
    char *_chooseFolder = strdup("Create here");
    char *_info = strdup("Info");
    char *_exit = strdup("Close App");

    /**
        * Functions needed to call C function, handles the panel
        */
    static void panelHandlerStatic();
};
#endif
