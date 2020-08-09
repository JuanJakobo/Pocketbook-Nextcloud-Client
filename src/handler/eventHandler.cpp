//------------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo          
// Date:             04.08.2020
//   
//-------------------------------------------------------------------

#include "inkview.h"
#include "eventHandler.h"
#include "menuHandler.h"
#include "listView.h"
#include "item.h"

#include <string>

EventHandler * EventHandler::eventHandlerStatic;

EventHandler::EventHandler()
{
    //create a event to create handlers
    eventHandlerStatic = this;

    menu = new MenuHandler("Nextcloud");

    nextcloud = new Nextcloud();
    //TODO SET USER, PASSWORD
    nextcloud->login("USER", "PASSWORD");

    listView = new ListView(menu->getContentRect(),nextcloud->getItems());

    FullUpdate();
}

EventHandler::~EventHandler()
{
    delete nextcloud;
    delete menu;
    delete listView;
}

int EventHandler::eventDistributor(int type, int par1, int par2)
{
    if (ISPOINTEREVENT(type))
	    return EventHandler::pointerHandler(type,par1,par2);

    return 0;
}

void EventHandler::mainMenuHandlerStatic(int index)
{
    eventHandlerStatic->mainMenuHandler(index);
}

void EventHandler::mainMenuHandler(int index)
{
    switch(index)
 	{ 
        //Exit	 
        case 101:
            CloseApp();
            break;
        default:
            break;
 	}
}

int EventHandler::pointerHandler(int type, int par1, int par2)
{

    if(type==EVT_POINTERDOWN)
    {
        if(IsInRect(par1,par2,menu->getMenuButtonRect())==1)
        {
            return menu->createMenu(true,EventHandler::mainMenuHandlerStatic);
        }
        else if(listView!= NULL)
        {
            int itemID = listView->listClicked(par1,par2);
            if(itemID!=-1)
            {
                string tempPath = nextcloud->getItems()[itemID].isClicked();

                if(!tempPath.empty())
                    nextcloud->getDataStructure(tempPath);

                delete listView;
                listView = new ListView(menu->getContentRect(),nextcloud->getItems());
                listView->drawHeader(tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));
            }

            PartialUpdate(menu->getContentRect()->x,menu->getContentRect()->y,menu->getContentRect()->w,menu->getContentRect()->h);
            return 1;
        }
    }
    return 0;
}
