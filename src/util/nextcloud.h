//------------------------------------------------------------------
// nextcloud.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Handles the userdata and issues, login and download of data structure for nextcloud WEBDAV
//
//-------------------------------------------------------------------

#ifndef NEXTCLOUD
#define NEXTCLOUD

#include "inkview.h"
#include "item.h"

#include <string>
#include <vector>
#include <memory>

using std::string;
using std::vector;

const string NEXTCLOUD_PATH = "/mnt/ext1/system/config/nextcloud";
const string NEXTCLOUD_CONFIG_PATH = NEXTCLOUD_PATH + "/nextcloud.cfg";
const string NEXTCLOUD_FILE_PATH = "/mnt/ext1/nextcloud";
const string NEXTCLOUD_ROOT_PATH = "/remote.php/dav/files/";
const string NEXTCLOUD_STRUCTURE_EXTENSION = ".structure";
const string NEXTCLOUD_END_PATH = "/nextcloud";

class Nextcloud
{
public:
    explicit Nextcloud();

    void setURL(const string &Url);
    void setUsername(const string &Username);
    void setUUID(const string &UUID);
    void setPassword(const string &Pass);
    void setStartFolder(const string &Path);
    bool setItems(const vector<Item> &tempItems);

    const vector<Item> &getItems() const { return _items; };
    bool isLoggedIn() const { return _loggedIn; };
    bool isWorkOffline() const { return _workOffline; };
    void switchWorkOffline() { _workOffline = !_workOffline; };

    /**
        * Handles login by receiving userdat from config
        * 
        * @return true - login succeeded, false - login failed
        */
    bool login();

    /**
        * Handles first login to nextcloud, if sucessfull saves userdata
        * 
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return true - login succeeded, false - login failed
         */
    bool login(const string &Url, const string &Username, const string &Pass);

    /**
        * Deletes the config files and deletes are temp files 
        * @param deleteFiles default false, true - local files are deleted, false local files are kept
        */
    void logout(bool deleteFiles = false);

    /**
        * Downloads a certain item from the Nextcloud and saves it locally 
        * @param tempItems set of items where the item is in
        * @param itemID id of the item 
        */
    void downloadItem(vector<Item> &tempItems, int itemID);

    /**
        * Downloads a certain folder from the Nextcloud and saves it locally 
        * @param tempItems set of items where the item is in
        * @param itemID id of the item 
        */
    void downloadFolder(vector<Item> &tempItems, int itemId);

    /**
        * Checks the network connection and starts the download of the certain itemId 
        * 
        * @param itemID id of the item that shall be downloaded
        */
    void download(int itemID);

    /**
        * Removes the item
        * 
        * @param itemID  id of the item that shall be removed
        * @return bool true - success, false - failure
        */
    bool removeItem(int itemID);

    /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector, reads Userdata from configfile
        * 
        * @param pathUrl URL to get the dataStructure of
        * @return vector of items
        */
    vector<Item> getDataStructure(string &pathUrl);

    /**
        * formats the pathUrl to a local path
        * 
        * @param path online path
        * @return local path
        */
    static string getLocalPath(string path);

    /**
        * checks the local storage checks if the items are locally available
        * 
        * @param tempItems items that shall be compared
        * @param localPath path that has to be checked
        */
    void getLocalFileStructure(vector<Item> &tempItems, const string &localPath);

private:
    vector<Item> _items;
    bool _loggedIn{false};
    string _url;
    bool _workOffline{false};

    /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector
        *         
        * @param pathUrl URL to get the dataStructure of
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return vector of Items 
        */
    vector<Item> getDataStructure(const string &pathUrl, const string &Username, const string &Pass);

    string getUrl();
    string getUsername();
    string getUUID();
    string getPassword();
    string getStartFolder();

    /**
        * Handles the end of the game dialog and lets the user
        * choose inbetween starting a new game or closing the app
        * 
        * @param Button id of the button that was pressed
        * @return void
        */
    static void DialogHandlerStatic(int Button);

    /**
     * Reads in the xml and creates a vector of items of it
     * 
     * @param xml xml formatted result from curl call
     * @return vector<Item> the items that have been read from the xml
     */
    vector<Item> readInXML(string xml);

    /**
     * Checks if a .structure file exists and if that is the case return a vector of items 
     * @param pathUrl path that shall be checked
     * @return return the items for the structure if available offline
     */
    vector<Item> getOfflineStructure(const string &pathUrl);
};

#endif