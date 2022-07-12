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
#include <string>
#include <curl/curl.h>
#include <fstream>
#include <sstream>

using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;


WebDAV::WebDAV()
{

    //TODO update on first login only start and create update button, update others just if etag changed
    //save all to sqlite

    Log::writeInfoLog(NEXTCLOUD_PATH);
    if (iv_access(NEXTCLOUD_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_PATH.c_str(), 0777);

    Log::writeInfoLog(NEXTCLOUD_FILE_PATH);
    if (iv_access(NEXTCLOUD_FILE_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_FILE_PATH.c_str(), 0777);

    Log::writeInfoLog(CONFIG_PATH);
    if (iv_access(CONFIG_PATH.c_str(), W_OK) == 0)
    {
        _username = Util::accessConfig(Action::IReadString,"username");
        _password = Util::accessConfig(Action::IReadSecret,"password");
    }

    if(_username.empty())
        Log::writeErrorLog("username not set");
    //test login
}

//TODO pas as reversne and no return
string WebDAV::getLocalPath(string path)
{
    Util::decodeUrl(path);
    if (path.find(NEXTCLOUD_ROOT_PATH) != string::npos)
        path = path.substr(NEXTCLOUD_ROOT_PATH.length());

    return NEXTCLOUD_FILE_PATH + "/" + path;
}

//TODO SQL CHeck before calling this function --> if is needed...
///TODO rename function
vector<WebDAVItem> WebDAV::getDataStructure(const string &pathUrl)
{
    string xmlItem = propfind(pathUrl);
    if(!xmlItem.empty())
    {
        string beginItem = "<d:response>";
        string endItem = "</d:response>";
        vector<WebDAVItem> tempItems;
        WebDAVItem tempItem;
        size_t begin = xmlItem.find(beginItem);
        size_t end;

        while (begin != std::string::npos)
        {
            end = xmlItem.find(endItem);
            //TODO use xml lib?

            //TODO fav is int?
            //Log::writeInfoLog(Util::getXMLAttribute(xmlItem, "d:favorite"));

            tempItem.etag = Util::getXMLAttribute(xmlItem, "d:getetag");
            tempItem.path = Util::getXMLAttribute(xmlItem, "d:href");
            tempItem.lastEditDate = Util::getXMLAttribute(xmlItem, "d:getlastmodified");
            tempItem.size = atof(Util::getXMLAttribute(xmlItem, "oc:size").c_str());
            //replaces everthing in front of /remote.php as this is already part of the url
            if(tempItem.path.find(NEXTCLOUD_START_PATH) != 0)
                tempItem.path.erase(0,tempItem.path.find(NEXTCLOUD_START_PATH));

            tempItem.title = tempItem.path;
            tempItem.localPath = getLocalPath(tempItem.path);

            if (tempItem.path.back() == '/')
            {
                tempItem.localPath = tempItem.localPath.substr(0, tempItem.localPath.length() - 1);
                tempItem.type = Itemtype::IFOLDER;
                tempItem.title = tempItem.title.substr(0, tempItem.path.length() - 1);
                //TODO set sync status of folders --> use sqlite?
            }
            else
            {
                tempItem.type = Itemtype::IFILE;
                tempItem.fileType = Util::getXMLAttribute(xmlItem, "d:getcontenttype");

                if (iv_access(tempItem.localPath.c_str(), W_OK) != 0)
                    tempItem.state = FileState::ICLOUD;
                else
                    tempItem.state = FileState::ISYNCED;
            }

            tempItem.title = tempItem.title.substr(tempItem.title.find_last_of("/") + 1, tempItem.title.length());
            Util::decodeUrl(tempItem.title);

            tempItems.push_back(tempItem);
            xmlItem = xmlItem.substr(end + endItem.length());
            begin = xmlItem.find(beginItem);
        }


        //TODO doppelt
        if (tempItems.empty())
            return {};

        string localPath = getLocalPath(pathUrl);

        //if the current folder does not exist locally, create it
        if (iv_access(localPath.c_str(), W_OK) != 0)
            iv_buildpath(localPath.c_str());

        return tempItems;
    }
    return {};
    //TODO return empty items?
}

string WebDAV::propfind(const string &pathUrl)
{
    /*
       string localPath = getLocalPath(pathUrl);

       if (!Util::connectToNetwork())
       {
       Message(ICON_WARNING, "Warning", "Cannot connect to the internet. Switching to offline modus. To work online turn on online modus in the menu.", 2000);
       _workOffline = true;
       return getOfflineStructure(pathUrl);
       }

       if (_url.empty())
       _url = this->getUrl();

*/

    auto _url = Util::accessConfig(Action::IReadString, "url");

    string readBuffer;
    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = _username + ":" + _password;
        Log::writeInfoLog(post);
        Log::writeInfoLog(_url + NEXTCLOUD_ROOT_PATH + pathUrl);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Depth: 1");
        curl_easy_setopt(curl, CURLOPT_URL, (_url + NEXTCLOUD_ROOT_PATH + pathUrl).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
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
                    Message(ICON_ERROR, "Error", "The URL seems to be incorrect. You can look up the WebDav URL in the files app under settings. ", 4000);
                    break;
                case 401:
                    Message(ICON_ERROR, "Error", "Username/password incorrect.", 4000);
                    break;
                case 207:
                    return readBuffer;
                    break;
                default:
                    Message(ICON_ERROR, "Error", ("An unknown error occured. Switching to offline modus. To work online turn on online modus in the menu. (Curl Response Code " + std::to_string(response_code) + ")").c_str(), 4000);
                    //TODO change default msg
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
