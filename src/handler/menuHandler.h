//------------------------------------------------------------------
// eventHandler.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      Handles the menubar and the menu
//-------------------------------------------------------------------

#ifndef MENU_HANDLER
#define MENU_HANDLER

#include <string>

using std::string;

class MenuHandler
{
public:
    /**
        * Defines fonds, sets global Event Handler and starts new content 
        * 
        * @param name name of the application
        */
    MenuHandler(const string &name);

    ~MenuHandler();

    irect *getContentRect() { return &_contentRect; };
    irect *getMenuButtonRect() { return &_menuButtonRect; };

    /**
        * Shows the menu on the screen, lets the user choose menu options and then redirects the handler to the caller
        * 
        * @param loogedIn the status if the user is logged in
        * @param handler handles the clicks on the menu 
        * @return int returns if the event was handled
        */
    int createMenu(bool loggedIn, bool workOffline, iv_menuhandler handler);

    /**
        * Draws a loading screen at the top of the screen 
        */ 
    void drawLoadingScreen();

    /**
        * Clears the area of the loading screen
        */ 
    void clearLoadingScreen();

private:
    ifont *_menuFont;
    ifont *_loadingFont;

    int _panelMenuBeginX;
    int _panelMenuBeginY;
    int _panelMenuHeight;
    int _mainMenuWidth;
    irect _menuButtonRect;
    irect _loadingScreenRect;

    imenu _mainMenu;
    irect _contentRect;

    char *_text;
    char *_menu = strdup("Menu");
    char *_makeStartfolder = strdup("Make startfolder");
    char *_logout = strdup("Logout");
    char *_info = strdup("Info");
    char *_exit = strdup("Close App");

    /**
        * Functions needed to call C function, handles the panel
        */
    static void panelHandlerStatic();
};
#endif