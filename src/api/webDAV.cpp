//
//------------------------------------------------------------------
// webdav.cpp
//
// Author:           JuanJakobo
// Date:             06.07.2022
//
//-------------------------------------------------------------------

#include "webDAV.h"
#include "util.h"
#include "log.h"
#include "eventHandler.h"
#include "fileHandler.h"

#include <string>
#include <experimental/filesystem>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <math.h>

using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

namespace fs = std::experimental::filesystem;

WebDAV::WebDAV()
{
    if (iv_access(NEXTCLOUD_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_PATH.c_str(), 0777);

    if (iv_access(CONFIG_PATH.c_str(), W_OK) == 0)
    {
        _username = Util::accessConfig<string>(Action::IReadString,"username",{});
        _password = Util::accessConfig<string>(Action::IReadSecret,"password",{});
        _url = Util::accessConfig<string>(Action::IReadString, "url",{});
        _ignoreCert = Util::accessConfig<int>(Action::IReadInt, "ignoreCert",{});
        _fileHandler = std::shared_ptr<FileHandler>(new FileHandler());
    }
}

WebDAV::~WebDAV() 
{
    _fileHandler.reset();
}


std::vector<WebDAVItem> WebDAV::login(const string &Url, const string &Username, const string &Pass, bool ignoreCert)
{
    string uuid;

    _password = Pass;
    _username = Username;
    _ignoreCert = ignoreCert;

    std::size_t found = Url.find(NEXTCLOUD_ROOT_PATH);
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
    auto tempPath = NEXTCLOUD_ROOT_PATH + uuid + "/";
    Util::accessConfig<string>( Action::IWriteString, "storageLocation", "/mnt/ext1/nextcloud");
    std::vector<WebDAVItem> tempItems = getDataStructure(tempPath);
    if (!tempItems.empty())
    {
        if (iv_access(CONFIG_PATH.c_str(), W_OK) != 0)
            iv_buildpath(CONFIG_PATH.c_str());
        Util::accessConfig<string>( Action::IWriteString, "url", _url);
        Util::accessConfig<string>( Action::IWriteString, "username", _username);
        Util::accessConfig<string>( Action::IWriteString, "UUID", uuid);
        Util::accessConfig<string>( Action::IWriteSecret, "password", _password);
        Util::accessConfig<int>( Action::IWriteInt, "ignoreCert", _ignoreCert);
    }
    else
    {
        _password = "";
        _username = "";
        _url = "";
    }
    return tempItems;
}

void WebDAV::logout(bool deleteFiles)
{
    if (deleteFiles)
    {
        fs::remove_all(Util::accessConfig<string>(Action::IReadString, "storageLocation",{}) + "/" + Util::accessConfig<string>(Action::IReadString,"UUID",{}) + '/');
    }
    fs::remove(CONFIG_PATH.c_str());
    fs::remove((CONFIG_PATH + ".back.").c_str());
    fs::remove(DB_PATH.c_str());
    _url = "";
    _password = "";
    _username = "";
}

vector<WebDAVItem> WebDAV::getDataStructure(const string &pathUrl)
{
    string xmlItem = propfind(pathUrl);
    if (!xmlItem.empty())
    {
        string beginItem = "<d:response>";
        string endItem = "</d:response>";
        vector<WebDAVItem> tempItems;
        WebDAVItem tempItem;
        size_t begin = xmlItem.find(beginItem);
        size_t end;

        string prefix = NEXTCLOUD_ROOT_PATH + _username + "/";
        while (begin != std::string::npos)
        {
            end = xmlItem.find(endItem);

            //TODO fav is int?
            //Log::writeInfoLog(Util::getXMLAttribute(xmlItem, "d:favorite"));

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

            //replaces everthing in front of /remote.php as this is already part of the url
            if (tempItem.path.find(NEXTCLOUD_START_PATH) != 0)
                tempItem.path.erase(0,tempItem.path.find(NEXTCLOUD_START_PATH));

            tempItem.title = tempItem.path;
            tempItem.localPath = tempItem.path;
            Util::decodeUrl(tempItem.localPath);
            if (tempItem.localPath.find(NEXTCLOUD_ROOT_PATH) != string::npos)
                tempItem.localPath = tempItem.localPath.substr(NEXTCLOUD_ROOT_PATH.length());
            tempItem.localPath = Util::accessConfig<string>(Action::IReadString, "storageLocation",{}) + "/" + tempItem.localPath;


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

            string &pathDecoded = tempItem.path;
            Util::decodeUrl(pathDecoded);
            tempItem.hide = _fileHandler->getHideState(tempItem.type, prefix,(pathDecoded), tempItem.title);
            
            tempItems.push_back(tempItem);
            xmlItem = xmlItem.substr(end + endItem.length());
            begin = xmlItem.find(beginItem);
        }

        if (!tempItems.empty())
            return tempItems;
    }
    return {};
}

string WebDAV::propfind(const string &pathUrl)
{
       if (pathUrl.empty() || _username.empty() || _password.empty())
       {
           Message(ICON_WARNING, "Warning", "Url, username or password is empty.", 2000);
           return "";
       }

       if (!Util::connectToNetwork())
           return "";
       ShowHourglassForce();

       //TODO for upload
        //get etag from current and then send request with FT_ENC_TAG
        //need path url and also the etag
        //can handle multiple etags --> * if exists
        //to use for upload
        //curl -I  --header 'If-None-Match: "XX"' -u username:password url/test.md
        //If-Unmodified-Since
        //to use for download
        // curl -I  --header 'If-Match: "XX"' -u username:password url/test.md
        //If-Modified-Since

        //If-None-Match: "XX"
        //--header 'If-None-Match: "XX"'
        //depth more to get here also childs? --> and depth less to see changes
        //case 412 Precondition failed --> etag matches for file
        //case 304:
        //content not modified


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

        if(_ignoreCert)
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
                    Message(ICON_ERROR, "Error", "The URL seems to be incorrect. You can look up the WebDav URL in the settings of the files webapp. ", 4000);
                    break;
                case 401:
                    Message(ICON_ERROR, "Error", "Username/password incorrect.", 4000);
                    break;
                case 207:
                    return readBuffer;
                    break;
                default:
                    Message(ICON_ERROR, "Error", ("An unknown error occured. (Curl Response Code " + std::to_string(response_code) + ")").c_str(), 5000);
            }
        }
        else
        {
            string response = std::string("An error occured. (") + curl_easy_strerror(res) + " (Curl Error Code: " + std::to_string(res) + ")). Please try again.";
            Log::writeErrorLog(response);
            Message(ICON_ERROR, "Error", response.c_str(), 4000);
        }
    }
    return "";
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
        Message(ICON_ERROR, "Error", "Download path is not set, therefore cannot download the file.", 2000);
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
        if(_ignoreCert)
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
                Message(ICON_ERROR, "Error", "Username/password incorrect.", 2000);
                break;
            default:
                Message(ICON_ERROR, "Error", ("An unknown error occured. (Curl Response Code " + std::to_string(response_code) + ")").c_str(), 2000);
                break;
            }
        }
        else
        {
            string response = std::string("An error occured. (") + curl_easy_strerror(res) + " (Curl Error Code: " + std::to_string(res) + ")). Please try again.";
            if (res == 60)
                response = "Seems as if you are using Let's Encrypt Certs. Please follow the guide on Github (https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client) to use a custom Cert Store on PB.";
            Message(ICON_ERROR, "Error", response.c_str(), 4000);
        }
    }
    return false;
}
