//------------------------------------------------------------------
// mainMenu.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "mainMenu.h"

#include <string>

using std::string;

MainMenu::MainMenu(const string &name)
{
    _panelMenuHeight = ScreenHeight() / 18;
    _mainMenuWidth = ScreenWidth() / 3;
    _panelMenuBeginY = 0;
    _panelMenuBeginX = ScreenWidth() - _mainMenuWidth;

    _menuButtonRect = iRect(_mainMenuWidth * 2, _panelMenuBeginY, _mainMenuWidth, _panelMenuHeight, ALIGN_RIGHT);

    _menuFont = OpenFont("LiberationMono-Bold", _panelMenuHeight / 2, FONT_STD);

    SetFont(_menuFont, BLACK);
    DrawTextRect(0, _panelMenuBeginY, ScreenWidth(), _panelMenuHeight, name.c_str(), ALIGN_CENTER);
    DrawTextRect2(&_menuButtonRect, "Menu");
    DrawLine(0, _panelMenuHeight - 1, ScreenWidth(), _panelMenuHeight - 1, BLACK);

    _contentRect = iRect(0, _panelMenuHeight, ScreenWidth(), (ScreenHeight() - _panelMenuHeight), 0);

    SetPanelType(0);
    PartialUpdate(0, _panelMenuBeginY, ScreenWidth(), _panelMenuHeight);
}

MainMenu::~MainMenu()
{
    CloseFont(_menuFont);
    free(_syncFolder);
    free(_menu);
    free(_logout);
    free(_sortBy);
    free(_info);
    free(_exit);
    free(_chooseFolder);
}

void MainMenu::panelHandlerStatic()
{
    DrawPanel(NULL, "", NULL, -1);
    SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
}

int MainMenu::createMenu(bool filePicker, bool loggedIn, iv_menuhandler handler)
{
    imenu mainMenu[] =
        {
            {ITEM_HEADER, 0, _menu, NULL},
            //show logged in
            {loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 101, _syncFolder, NULL},
            {loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 103, _sortBy, NULL},
            {loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 102, _logout, NULL},
            //show if filePicker is shown
            {filePicker ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, 104, _chooseFolder, NULL},
            //show always
            {ITEM_ACTIVE, 105, _info, NULL},
            {ITEM_ACTIVE, 106, _exit, NULL},
            {0, 0, NULL, NULL}};

    OpenMenu(mainMenu, 0, _panelMenuBeginX, _panelMenuBeginY, handler);

    return 1;
}
