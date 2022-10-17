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
#include "webDAV.h"
#include "webDAVView.h"
#include "loginView.h"
#include "fileView.h"
#include "excludeFileView.h"
#include "sqliteConnector.h"
#include "log.h"
#include "fileHandler.h"

#include <memory>

const std::string CONFIG_FOLDER = "/mnt/ext1/system/config/nextcloud";
const std::string DB_PATH = CONFIG_FOLDER + "/data.db";

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
    std::unique_ptr<WebDAVView> _webDAVView;
    std::unique_ptr<LoginView> _loginView;
    std::unique_ptr<FileView> _fileView;
    std::unique_ptr<ExcludeFileView> _excludeFileView;
    std::unique_ptr<MainMenu> _menu;

    std::shared_ptr<FileHandler> _fileHandler;

    ContextMenu _contextMenu = ContextMenu();
    WebDAV _webDAV = WebDAV();
    SqliteConnector _sqllite = SqliteConnector(DB_PATH);
    std::string _currentPath;

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
        * Function needed to call C function, redirects to real function
        *
        *  @param index int of the menu that is set
        */
    static void contextMenuHandlerStatic(const int index);

    /**
        * Handlescontext  menu events and redirects them
        *
        * @param index int of the menu that is set
        */

    void contextMenuHandler(const int index);

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
     * Updates PB Library
     *
     */
    void updatePBLibrary();

    /**
        * Open a folder
        *
        */
    void openFolder();


    /**
        * Open a item
        *
        */
    void openItem();

    /**
        * Handles key Events
        *
        * @param type event type
        * @param par1 first argument of the event (is the key)
        * @param par2 second argument of the event
        * @return int returns if the event was handled
        */
    int keyHandler(const int type, const int par1, const int par2);

    void getLocalFileStructure(std::vector<WebDAVItem> &tempItems);

    void downloadFolder(std::vector<WebDAVItem> &items, int itemID);

    void startDownload();

    bool checkIfIsDownloaded(std::vector<WebDAVItem> &items, int itemID);

    void updateItems(std::vector<WebDAVItem> &items);

    void drawWebDAVItems(std::vector<WebDAVItem> &items);

};
#endif
