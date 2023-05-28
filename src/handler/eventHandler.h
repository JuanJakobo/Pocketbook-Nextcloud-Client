//------------------------------------------------------------------
// eventHandler.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Handles all events and directs them
//-------------------------------------------------------------------

#pragma once

#include <memory>

#include "contextMenu.h"
#include "excludeFileView.h"
#include "fileHandler.h"
#include "fileView.h"
#include "log.h"
#include "loginView.h"
#include "mainMenu.h"
#include "sqliteConnector.h"
#include "webDAV.h"
#include "webDAVView.h"

using namespace std::string_literals;

constexpr auto APPLICATION_NAME{"Nextcloud Client"};

constexpr auto CONF_STORAGE_LOCATION{"storageLocation"};
constexpr auto CONF_USERNAME{"username"};
constexpr auto CONF_UUID{"UUID"};
constexpr auto CONF_PASSWORD{"password"};
constexpr auto CONF_URL{"url"};
constexpr auto CONF_IGNORE_CERT{"ignoreCert"};
constexpr auto CONF_EXTENSION_LIST{"ex_extensionList"};
constexpr auto CONF_EXTENSION_PATTERN{"ex_pattern"};
constexpr auto CONF_EXTENSION_FOLDER_PATTERN{"ex_folderPattern"};
constexpr auto CONF_EXTENSION_RELATIVE_ROOT_PATH{"ex_relativeRootPath"};
constexpr auto CONF_EXTENSION_INVERT_MATCH{"ex_invertMatch"};

constexpr auto DEFAULT_STORAGE_LOCATION{"/mnt/ext1/nextcloud"};
constexpr auto CONFIG_FOLDER_LOCATION{"/mnt/ext1/system/config/nextcloud"};
const auto CONFIG_FILE_LOCATION{CONFIG_FOLDER_LOCATION + "/nextcloud.cfg"s};
const auto DB_LOCATION{CONFIG_FOLDER_LOCATION + "/data.db"s};

constexpr auto TEXT_MESSAGE_ERROR{"Error"};
constexpr auto TEXT_MESSAGE_WARNING{"Warning"};
constexpr auto TEXT_MESSAGE_INFO{"Info"};
constexpr auto TEXT_MESSAGE_ACTION{"Action"};
constexpr auto TIMEOUT_MESSAGE{2000u};
constexpr auto TEXT_DIALOG_CLOSE_APP{"Close App"};
constexpr auto TEXT_DIALOG_LOGOUT{"Logout"};
constexpr auto TEXT_DIALOG_CANCEL{"Cancel"};
constexpr auto TEXT_DIALOG_NO{"No"};
constexpr auto TEXT_DIALOG_YES{"Yes"};

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
    SqliteConnector _sqllite = SqliteConnector(DB_LOCATION);
    std::string _currentPath;

    /**
     * Function needed to call C function, redirects to real function
     *
     *  @param index int of the menu that is set
     */
    static void mainMenuHandlerStatic(int index);

    /**
     * Handles menu events and redirects them
     *
     * @param option MainMenuOption of the menu that is set
     */
    void mainMenuHandler(MainMenuOption p_mainMenuOption);

    /**
     * Function needed to call C function, redirects to real function
     *
     *  @param index int of the menu that is set
     */
    static void contextMenuHandlerStatic(int p_index);

    /**
     * Handles context menu events and redirects them
     *
     * @param option ContextMenuOption of the menu that is set
     */

    void contextMenuHandler(ContextMenuOption p_contextMenuOption);

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
