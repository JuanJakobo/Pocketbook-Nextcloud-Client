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

using namespace std;

const string NEXTCLOUD_PATH = "/mnt/ext1/system/config/nextcloud";
const string NEXTCLOUD_CONFIG_PATH = NEXTCLOUD_PATH + "/nextcloud.cfg";
const string NEXTCLOUD_FILE_PATH = "/mnt/ext1/nextcloud/";
const string NEXTCLOUD_URL = "https://cloud.jjohannssen.de";
const string NEXTCLOUD_ROOT_PATH = "/remote.php/dav/files/";

class Nextcloud {
    public:

        explicit Nextcloud();

        void setUsername(const string& Username);
        void setPassword(const string& Pass);

        vector<Item> &getItems(){return items;};
        bool isLoggedIn(){return loggedIn;};
        
        /**
        * Handles first login to nextcloud, if sucessfull saves userdata
        * 
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return true - sucessfull login, false - failed login
        */
        bool login(const string& Username, const string& Pass);

         /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector, reads Userdata from configfile
        * 
        * @param pathUrl URL to get the dataStructure of
        * @return true - sucessfull, false - error
        */
        bool getDataStructure(string& pathUrl);

        /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector
        *         
        * @param pathUrl URL to get the dataStructure of
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return true - sucessfull, false - error
        */
        bool getDataStructure(const string& pathUrl, const string& Username, const string& Pass);

    private:
        vector<Item> items;
        bool loggedIn;  
        
        string getUsername();        
        string getPassword();
};



#endif