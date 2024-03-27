//
//------------------------------------------------------------------
// webdav.cpp
//
// Author:           JuanJakobo
// Date:             06.07.2022
//
//-------------------------------------------------------------------

#include "webDAV.h"

#include <curl/curl.h>
#include <math.h>

#include <experimental/filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include "eventHandler.h"
#include "fileHandler.h"
#include "log.h"
#include "util.h"

using namespace std::string_literals;

namespace
{
constexpr auto BEGIN_XML_ITEM{"<d:response>"};
const auto END_XML_ITEM{"</d:response>"s};
constexpr auto TEXT_CACERT_FILE_NOT_FOUND{"CACert.pem not found. You can download it at 'https://curl.se/ca/cacert.pem'. Then place it into the application folder."};
} // namespace

using std::string;
using std::vector;

namespace fs = std::experimental::filesystem;

WebDAV::WebDAV()
{
    _fileHandler = std::make_shared<FileHandler>();

    if (iv_access(CONFIG_FILE_LOCATION.c_str(), W_OK) == 0)
    {
        _username = Util::getConfig<string>(CONF_USERNAME);
        _password = Util::getConfig<string>(CONF_PASSWORD, "", true);
        _url = Util::getConfig<string>(CONF_URL);
        _ignoreCert = Util::getConfig<int>(CONF_IGNORE_CERT, -1);
    }
}

std::vector<WebDAVItem> WebDAV::login(const string &Url, const string &Username, const string &Pass, bool ignoreCert)
{
    string uuid;

    _password = Pass;
    _username = Username;
    _ignoreCert = ignoreCert;

    const auto found{Url.find(NEXTCLOUD_ROOT_PATH)};
    if (found != std::string::npos)
    {
        _url = Url.substr(0, found);
        uuid = Url.substr(found + NEXTCLOUD_ROOT_PATH.length());
    }
    else
    {
        _url = Url;
        uuid = Username;
    }
    Util::encodeUrl(uuid);
    if (iv_access(CONFIG_FOLDER_LOCATION, W_OK) != 0)
    {
        iv_mkdir(CONFIG_FOLDER_LOCATION, 0777);
    }

    Util::writeConfig<string>(CONF_STORAGE_LOCATION, DEFAULT_STORAGE_LOCATION);

    const auto tempPath{NEXTCLOUD_ROOT_PATH + uuid + "/"};
    const auto tempItems = getDataStructure(tempPath);
    if (!tempItems.empty())
    {
        if (iv_access(CONFIG_FILE_LOCATION.c_str(), W_OK) != 0)
            iv_buildpath(CONFIG_FILE_LOCATION.c_str());
        Util::writeConfig<string>(CONF_URL, _url);
        Util::writeConfig<string>(CONF_USERNAME, _username);
        Util::writeConfig<string>(CONF_UUID, uuid);
        Util::writeConfig<string>(CONF_PASSWORD, _password, true);
        Util::writeConfig<int>(CONF_IGNORE_CERT, _ignoreCert);
    }
    else
    {
        _password = {};
        _username = {};
        _url = {};
    }
    return tempItems;
}

void WebDAV::logout(bool deleteFiles)
{
    _url = {};
    _password = {};
    _username = {};

    if (deleteFiles)
    {
        const auto filesPath{Util::getConfig<string>(CONF_STORAGE_LOCATION) + "/" + Util::getConfig<string>(CONF_UUID) +
                             '/'};
        if (fs::exists(filesPath))
        {
            fs::remove_all(filesPath);
        }
    }

    const auto configPath{CONFIG_FOLDER_LOCATION + "/"s};

    if (fs::exists(configPath))
    {
        fs::remove_all(configPath);
    }
}

vector<WebDAVItem> WebDAV::getDataStructure(const string &pathUrl)
{
    auto xmlItem{propfind(pathUrl)};
    if (!xmlItem.empty())
    {
        vector<WebDAVItem> tempItems;
        WebDAVItem tempItem;
        auto begin_location = xmlItem.find(BEGIN_XML_ITEM);
        size_t end_location;

        const auto prefix{NEXTCLOUD_ROOT_PATH + _username + "/"};
        while (begin_location != std::string::npos)
        {
            end_location = xmlItem.find(END_XML_ITEM);

            // TODO fav is int?
            // Log::writeInfoLog(Util::getXMLAttribute(xmlItem, "d:favorite"));

            tempItem.etag = Util::getXMLAttribute(xmlItem, "d:getetag");
            tempItem.path = Util::getXMLAttribute(xmlItem, "d:href");
            tempItem.lastEditDate = Util::webDAVStringToTm(Util::getXMLAttribute(xmlItem, "d:getlastmodified"));

            double size = atof(Util::getXMLAttribute(xmlItem, "oc:size").c_str());
            if (size < 1024)
                tempItem.size = "< 1 KB";
            else
            {
                double departBy;
                double tempSize;
                string unit;

                if (size < 1048576)
                {
                    departBy = 1024;
                    unit = "KB";
                }
                else if (size < 1073741824)
                {
                    departBy = 1048576;
                    unit = "MB";
                }
                else
                {
                    departBy = 1073741824;
                    unit = "GB";
                }
                tempSize = round((size / departBy) * 10.0) / 10.0;
                std::ostringstream stringStream;
                stringStream << tempSize;
                tempItem.size = stringStream.str() + " " + unit;
            }

            // replaces everthing in front of /remote.php as this is already part of
            // the url
            if (tempItem.path.find(NEXTCLOUD_START_PATH) != 0)
                tempItem.path.erase(0, tempItem.path.find(NEXTCLOUD_START_PATH));

            tempItem.title = tempItem.path;
            tempItem.localPath = tempItem.path;
            Util::decodeUrl(tempItem.localPath);
            if (tempItem.localPath.find(NEXTCLOUD_ROOT_PATH) != string::npos)
                tempItem.localPath = tempItem.localPath.substr(NEXTCLOUD_ROOT_PATH.length());
            tempItem.localPath = Util::getConfig<string>(CONF_STORAGE_LOCATION) + "/" + tempItem.localPath;

            if (tempItem.path.back() == '/')
            {
                tempItem.localPath = tempItem.localPath.substr(0, tempItem.localPath.length() - 1);
                tempItem.type = Itemtype::IFOLDER;
                tempItem.title = tempItem.title.substr(0, tempItem.path.length() - 1);
            }
            else
            {
                tempItem.type = Itemtype::IFILE;
                tempItem.fileType = Util::getXMLAttribute(xmlItem, "d:getcontenttype");
            }

            tempItem.title = tempItem.title.substr(tempItem.title.find_last_of("/") + 1, tempItem.title.length());
            Util::decodeUrl(tempItem.title);

            auto pathDecoded{tempItem.path};
            Util::decodeUrl(pathDecoded);
            tempItem.hide = _fileHandler->getHideState(tempItem.type, prefix, pathDecoded, tempItem.title);

            tempItems.push_back(tempItem);
            xmlItem = xmlItem.substr(end_location + END_XML_ITEM.length());
            begin_location = xmlItem.find(BEGIN_XML_ITEM);
        }

        if (!tempItems.empty())
            return tempItems;
    }
    return {};
}

std::string WebDAV::getRootPath(bool encode)
{
    auto rootPath{Util::getConfig<std::string>(CONF_EXTENSION_RELATIVE_ROOT_PATH, "/")};
    if (rootPath.empty())
    {
        rootPath += "/";
    }

    auto user{Util::getConfig<std::string>(CONF_UUID, "")};
    if (user.empty())
    {
        // TODO change return code stuff, optionals?
        user = Util::getConfig<std::string>(CONF_USERNAME, "error");
    }

    string rtc = NEXTCLOUD_ROOT_PATH + user + rootPath;

    if (encode)
    {
        Util::encodeUrl(rtc);
        rtc = std::regex_replace(rtc, std::regex("%2F"), "/");
    }

    return rtc;
}

string WebDAV::propfind(const string &pathUrl)
{
    if (pathUrl.empty() || _username.empty() || _password.empty())
    {
        Message(ICON_WARNING, TEXT_MESSAGE_ERROR, "Url, username or password is empty.", TIMEOUT_MESSAGE);
        return "";
    }

    if (!Util::connectToNetwork())
        return "";
    ShowHourglassForce();

    // TODO for upload
    // get etag from current and then send request with FT_ENC_TAG
    // need path url and also the etag
    // can handle multiple etags --> * if exists
    // to use for upload
    // curl -I  --header 'If-None-Match: "XX"' -u username:password url/test.md
    // If-Unmodified-Since
    // to use for download
    //  curl -I  --header 'If-Match: "XX"' -u username:password url/test.md
    // If-Modified-Since

    // If-None-Match: "XX"
    //--header 'If-None-Match: "XX"'
    // depth more to get here also childs? --> and depth less to see changes
    // case 412 Precondition failed --> etag matches for file
    // case 304:
    // content not modified

    string readBuffer;
    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = _username + ":" + _password;

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Depth: 1");
        curl_easy_setopt(curl, CURLOPT_URL, (_url + pathUrl).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        if (iv_access(CACERT_PATH, R_OK) == 0)
        {
            curl_easy_setopt(curl, CURLOPT_CAINFO, CACERT_PATH);
        }
        else
        {
            downloadCaPem();
            return {};
        }

        if (_ignoreCert)
        {
            Log::writeInfoLog("Cert ignored");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "<\?xml version=\"1.0\" encoding=\"UTF-8\"\?> \
                                                    <d:propfind xmlns:d=\"DAV:\"><d:prop xmlns:oc=\"http://owncloud.org/ns\"> \
                                                    <d:getlastmodified/> \
                                                    <d:getcontenttype/> \
                                                    <oc:size/> \
                                                    <d:getetag/> \
                                                    <oc:favorite/> \
                                                    </d:prop></d:propfind>");

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            switch (response_code)
            {
            case 404:
                if (getRootPath().compare(NEXTCLOUD_ROOT_PATH + Util::getConfig<std::string>(CONF_UUID, "")) != 0)
                {
                    if (propfind(NEXTCLOUD_ROOT_PATH + Util::getConfig<std::string>(CONF_UUID, "")) != "")
                    {
                        // Own root path defined
                        string output;
                        int dialogResult =
                            DialogSynchro(ICON_ERROR, TEXT_MESSAGE_ERROR,
                                          output
                                              .append("The specified start folder does not seem to "
                                                      "exist:\n")
                                              .append(Util::getConfig<std::string>("ex_relativeRootPath", "/"))
                                              .append("\n\nWhat would you like to do?")
                                              .c_str(),
                                          "Reset start folder", TEXT_DIALOG_CLOSE_APP, NULL);
                        switch (dialogResult)
                        {
                        case 1: {
                            Util::writeConfig<string>(CONF_EXTENSION_RELATIVE_ROOT_PATH, "");
                            return propfind(NEXTCLOUD_ROOT_PATH + Util::getConfig<std::string>(CONF_UUID, ""));
                        }
                        break;
                        case 2:
                        default:
                            CloseApp();
                            break;
                        }
                    }
                }
                else
                {
                    Message(ICON_ERROR, TEXT_MESSAGE_ERROR,
                            "The URL seems to be incorrect. You can look up the WebDav "
                            "URL in the settings of the files webapp.",
                            TIMEOUT_MESSAGE);
                }
                break;
            case 401:
                Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Username/password incorrect.", TIMEOUT_MESSAGE);
                break;
            case 207:
                return readBuffer;
                break;
            default:
                Message(
                    ICON_ERROR, TEXT_MESSAGE_ERROR,
                    ("An unknown error occured. (Curl Response Code " + std::to_string(response_code) + ")").c_str(),
                    TIMEOUT_MESSAGE);
            }
        }
        else
        {
            string response = std::string("An error occured. (") + curl_easy_strerror(res) +
                              " (Curl Error Code: " + std::to_string(res) + ")). Please try again.";
            Log::writeErrorLog(response);
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, response.c_str(), TIMEOUT_MESSAGE);
        }
    }
    return {};
}

bool WebDAV::get(WebDAVItem &item)
{
    if (item.state == FileState::ISYNCED)
    {
        UpdateProgressbar(("The newest version of file " + item.path + " is already downloaded.").c_str(), 0);
        return false;
    }

    if (item.path.empty())
    {
        Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Download path is not set, therefore cannot download the file.",
                TIMEOUT_MESSAGE);
        return false;
    }

    if (!Util::connectToNetwork())
        return false;

    ShowHourglassForce();

    UpdateProgressbar(("Starting Download to " + item.localPath).c_str(), 0);
    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = _username + std::string(":") + _password;

        FILE *fp;
        fp = iv_fopen(item.localPath.c_str(), "wb");

        curl_easy_setopt(curl, CURLOPT_URL, (_url + item.path).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, Util::progress_callback);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        if (iv_access(CACERT_PATH, R_OK) == 0)
        {
            curl_easy_setopt(curl, CURLOPT_CAINFO, CACERT_PATH);
        }
        else
        {
            downloadCaPem();
            return false;
        }

        if (_ignoreCert)
        {
            Log::writeInfoLog("Cert ignored");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        iv_fclose(fp);

        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            switch (response_code)
            {
            case 200:
                Log::writeInfoLog("finished download of " + item.title + " to " + item.localPath);
                return true;
                break;
            case 401:
                Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Username/password incorrect.", TIMEOUT_MESSAGE);
                break;
            default:
                Message(
                    ICON_ERROR, TEXT_MESSAGE_ERROR,
                    ("An unknown error occured. (Curl Response Code " + std::to_string(response_code) + ")").c_str(),
                    TIMEOUT_MESSAGE);
                break;
            }
        }
        else
        {
            string response = std::string("An error occured. (") + curl_easy_strerror(res) +
                              " (Curl Error Code: " + std::to_string(res) + ")). Please try again.";
            if (res == 60)
                response = "Seems as if you are using Let's Encrypt Certs. Please follow the "
                           "guide on Github "
                           "(https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client) to "
                           "use a custom Cert Store on PB.";
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, response.c_str(), TIMEOUT_MESSAGE);
        }
    }
    return false;
}

void WebDAV::downloadCaPem()
{
    //TODO ask for download and then download and save ca.pem in method
    //https://curl.se/ca/cacert.pem
    Log::writeErrorLog(TEXT_CACERT_FILE_NOT_FOUND);
    auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION,TEXT_CACERT_FILE_NOT_FOUND, TEXT_DIALOG_CLOSE_APP, NULL, NULL)};
    switch (dialogResult)
    {
        default:
            CloseApp();
            break;
    }
}
