//------------------------------------------------------------------
// eventHandler.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Handles all events and directs them
//-------------------------------------------------------------------

#ifndef EVENT_HANDLER
#define EVENT_HANDLER

#include "contextMenu.h"
#include "mainMenu.h"
#include "nextcloud.h"
#include "listView.h"
#include "loginView.h"

#include <memory>

const std::string LOG_PATH = "/mnt/ext1/system/config/nextcloud";

class EventHandler
{
public:
    /**
        * Defines fonds, sets global Event Handler and starts new content 
        */
    EventHandler();

    /**
        * Handles events and redirects them
        * 
        * @param type event type
        * @param par1 first argument of the event
        * @param par2 second argument of the event
        * @return int returns if the event was handled
        */
    int eventDistributor(const int type, const int par1, const int par2);

private:
    static std::unique_ptr<EventHandler> _eventHandlerStatic;
    std::unique_ptr<ListView> _listView;
    std::unique_ptr<LoginView> _loginView;
    std::unique_ptr<ContextMenu> _contextMenu;
    MainMenu _menu = MainMenu("Nextcloud");
    Nextcloud _nextcloud = Nextcloud();
    std::string _tempPath;

    /**
        * Function needed to call C function, redirects to real function
        * 
        *  @param index int of the menu that is set
        */
    static void mainMenuHandlerStatic(const int index);
    
    /**
        * Handles menu events and redirects them
        * 
        * @param index int of the menu that is set
        */
    void mainMenuHandler(const int index);

    /**
        * Handles pointer Events
        * 
        * @param type event type
        * @param par1 first argument of the event
        * @param par2 second argument of the event
        * @return int returns if the event was handled
        */
    int pointerHandler(const int type, const int par1, const int par2);

    /**
        * Handles the cancel button on the progressbar
        * 
        * @param clicked button that has been clicked
        */
    static void DialogHandlerStatic(const int clicked);
};
#endif