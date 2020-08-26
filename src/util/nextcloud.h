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

using std::string;
using std::vector;

const string NEXTCLOUD_PATH = "/mnt/ext1/system/config/nextcloud";
const string NEXTCLOUD_CONFIG_PATH = NEXTCLOUD_PATH + "/nextcloud.cfg";
const string NEXTCLOUD_FILE_PATH = "/mnt/ext1/nextcloud";
const string NEXTCLOUD_ROOT_PATH = "/remote.php/dav/files/";

class Nextcloud
{
public:
    explicit Nextcloud();

    /**
        * Handles first login to nextcloud, if sucessfull saves userdata
        * 
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return true - sucessfull login, false - failed login
        */
    bool login(const string &Url, const string &Username, const string &Pass);

    bool login();
    
    void logout();

    bool downloadItem(int itemID);

    /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector, reads Userdata from configfile
        * 
        * @param pathUrl URL to get the dataStructure of
        * @return true - sucessfull, false - error
        */
    bool getDataStructure(string &pathUrl);

    void setURL(const string &Url);
    void setUsername(const string &Username);
    void setPassword(const string &Pass);

    vector<Item> getItems() { return _items; };
    bool isLoggedIn() { return _loggedIn; };

private:
    vector<Item> _items;
    bool _loggedIn;
    string _url;
    //make username and password local variables or get each time? --> it cant change during login??

    /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector
        *         
        * @param pathUrl URL to get the dataStructure of
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return true - sucessfull, false - error
        */
    bool getDataStructure(const string &pathUrl, const string &Username, const string &Pass);

    string getUsername();
    string getPassword();
    string getUrl();
};

#endif