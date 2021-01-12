//------------------------------------------------------------------
// menuHandler.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "menuHandler.h"

#include <string>

using std::string;

MenuHandler::MenuHandler(const string &name)
{
    //Define panel size
    _panelMenuHeight = ScreenHeight() / 18;
    _panelMenuBeginY = 0;
    _mainMenuWidth = ScreenWidth() / 3;
    _panelMenuBeginX = ScreenWidth() - _mainMenuWidth;

    _menuButtonRect = iRect(_mainMenuWidth * 2, _panelMenuBeginY, _mainMenuWidth, _panelMenuHeight, ALIGN_RIGHT);

    _menuFont = OpenFont("LiberationMono-Bold", _panelMenuHeight / 2, 1);

    SetFont(_menuFont, BLACK);
    DrawTextRect(0, _panelMenuBeginY, ScreenWidth(), _panelMenuHeight, name.c_str(), ALIGN_CENTER);
    DrawTextRect2(&_menuButtonRect, "Menu");
    DrawLine(0, _panelMenuHeight - 1, ScreenWidth(), _panelMenuHeight - 1, BLACK);

    _contentRect = iRect(0, _panelMenuHeight, ScreenWidth(), (ScreenHeight() - PanelHeight() - _panelMenuHeight), 0);

    _loadingScreenRect = iRect(_contentRect.w / 2 - 125, _contentRect.h / 2 - 50, 250, 100, ALIGN_CENTER);
    _loadingFont = OpenFont("LiberationMono", _loadingScreenRect.h / 4, 1);

    SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
    DrawPanel(NULL, "", NULL, -1);
}

MenuHandler::~MenuHandler()
{
    CloseFont(_menuFont);
    CloseFont(_loadingFont);
}

void MenuHandler::panelHandlerStatic()
{
    DrawPanel(NULL, "", NULL, -1);
    SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
}

int MenuHandler::createMenu(bool loggedIn, bool workOffline, iv_menuhandler handler)
{
    char *text = "Work offline";
    if (workOffline)
        text = "Work online";

    imenu mainMenu[] =
        {
            {ITEM_HEADER, 0, "Menu", NULL},
            //show logged in
            {loggedIn ? ITEM_ACTIVE : ITEM_HIDDEN, 101, text, NULL},
            {loggedIn ? ITEM_ACTIVE : ITEM_HIDDEN, 102, "Make startfolder", NULL},
            {loggedIn ? ITEM_ACTIVE : ITEM_HIDDEN, 103, "Logout", NULL},
            //show always
            {ITEM_ACTIVE, 104, "Info"},
            {ITEM_ACTIVE, 105, "Exit"},
            {0, 0, NULL, NULL}};

    if (loggedIn)
    {
        mainMenu[3].type = ITEM_ACTIVE;
    }
    else
    {
        mainMenu[2].type = ITEM_ACTIVE;
    }

    OpenMenu(mainMenu, 0, _panelMenuBeginX, _panelMenuBeginY, handler);

    return 1;
}

void MenuHandler::drawLoadingScreen()
{
    SetFont(_loadingFont, BLACK);
    DrawTextRect2(&_loadingScreenRect, "Loading...");
    PartialUpdate(_loadingScreenRect.x, _loadingScreenRect.y, _loadingScreenRect.w, _loadingScreenRect.h);
}
