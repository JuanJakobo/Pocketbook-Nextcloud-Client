//------------------------------------------------------------------
// menuHandler.cpp
//
// Author:           JuanJakobo          
// Date:             14.06.2020
//   
//-------------------------------------------------------------------

#include "inkview.h"
#include "menuHandler.h"
#include "util.h"

#include <string>
#include <ctime>

using namespace std;

MenuHandler::MenuHandler(const string &name)
{
    //Define panel size
    panelMenuHeight = ScreenHeight() / 18;
    panelMenuBeginY = 0;
    mainMenuWidth = ScreenWidth()/3;
    panelMenuBeginX = ScreenWidth() - mainMenuWidth;

    menuButtonRect = iRect(mainMenuWidth*2,panelMenuBeginY,mainMenuWidth,panelMenuHeight,ALIGN_RIGHT);

    menuFont = OpenFont("LiberationMono-Bold",panelMenuHeight/2,1);
    SetFont(menuFont, BLACK);

    DrawTextRect(0,panelMenuBeginY,ScreenWidth(),panelMenuHeight,name.c_str(),ALIGN_CENTER);
    DrawTextRect2(&menuButtonRect,"Menu");
    DrawLine(0,panelMenuHeight-1,ScreenWidth(),panelMenuHeight-1,BLACK);

    contentRect = iRect(0,panelMenuHeight,ScreenWidth(),(ScreenHeight() - PanelHeight() - panelMenuHeight),0);

    SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
	DrawPanel(NULL, "", NULL, -1);

}

MenuHandler::~MenuHandler()
{
    CloseFont(menuFont);
}

void MenuHandler::panelHandlerStatic() 
{
    DrawPanel(NULL, "", NULL, -1);
    SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
}


int MenuHandler::createMenu(bool loggedIn, iv_menuhandler handler)
{
    imenu mainMenu[]=
    {
        { ITEM_HEADER, 0, "Menu", NULL},
        //show always
        { ITEM_ACTIVE, 101, "Exit"},
        { 0, 0, NULL, NULL}
    };

    if(loggedIn)
    {
        mainMenu[1].type=ITEM_ACTIVE;
    }
    else
    {
        mainMenu[1].type=ITEM_ACTIVE;
    }   

    OpenMenu(mainMenu,0,panelMenuBeginX,panelMenuBeginY,handler);
    
    return 1;
    
}