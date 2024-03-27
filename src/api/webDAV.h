#pragma once
//------------------------------------------------------------------
// webdav.h
//
// Author:           JuanJakobo
// Date:             06.07.2022
// Description: Interface to the webdav API
//
//-------------------------------------------------------------------
#include <memory>
#include <string>
#include <vector>

#include "fileHandler.h"
#include "webDAVModel.h"
// TODO use namespaces

using namespace std::string_literals;

const auto NEXTCLOUD_ROOT_PATH{"/remote.php/dav/files/"s};
constexpr auto NEXTCLOUD_START_PATH{"/remote.php/"};
constexpr auto CACERT_PATH{"/mnt/ext1/applications/cacert.pem"};

class WebDAV
{
  public:
    /**
     * Creates a new pocket object containing token to access the api
     *
     */
    WebDAV();
    ~WebDAV() = default;

    std::vector<WebDAVItem> login(const std::string &Url, const std::string &Username, const std::string &Pass,
                                  bool ignoreCert = false);

    void logout(bool deleteFiles = false);

    std::vector<WebDAVItem> getDataStructure(const std::string &pathUrl);

    /**
     * Returns the root path of the nextcloud server
     * (e.g. /remote.php/dav/files/userName/startFolder/)
     */
    static std::string getRootPath(bool encode = false);

    /**
     * gets the dataStructure of the given URL and writes its WEBDAV items to the
     * items vector
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

    void downloadCaPem();
};
