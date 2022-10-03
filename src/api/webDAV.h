//------------------------------------------------------------------
// webdav.h
//
// Author:           JuanJakobo
// Date:             06.07.2022
// Description: Interface to the webdav API
//
//-------------------------------------------------------------------

#ifndef WEBDAV
#define WEBDAV

#include "webDAVModel.h"
#include "fileHandler.h"

#include <string>
#include <vector>

#include <memory>

const static std::string NEXTCLOUD_ROOT_PATH = "/remote.php/dav/files/";
const std::string NEXTCLOUD_START_PATH = "/remote.php/";
const std::string NEXTCLOUD_PATH = "/mnt/ext1/system/config/nextcloud";

class WebDAV
{
    public:
        /**
         * Creates a new pocket object containing token to access the api
         *
         */
        WebDAV();
        ~WebDAV();

        std::vector<WebDAVItem> login(const std::string &Url, const std::string &Username, const std::string &Pass, bool ignoreCert = false);

        void logout(bool deleteFiles = false);

        std::vector<WebDAVItem> getDataStructure(const std::string &pathUrl);

    /**
        * gets the dataStructure of the given URL and writes its WEBDAV items to the items vector
        *
        * @param pathUrl URL to get the dataStructure of
        * @param Username the username of the Nextcloud instance
        * @param Pass the pass of the Nextcloud instance
        * @return vector of Items
        */
        std::string propfind(const std::string &pathUrl);

        bool get(WebDAVItem &item);

    private:
        std::string _username;
        std::string _password;
        std::string _url;
        bool _ignoreCert;

        std::shared_ptr<FileHandler> _fileHandler;

};
#endif
