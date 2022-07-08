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

    if (iv_access(NEXTCLOUD_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_PATH.c_str(), 0777);

    if (iv_access(NEXTCLOUD_FILE_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_FILE_PATH.c_str(), 0777);

    if (iv_access(CONFIG_PATH.c_str(), W_OK) == 0)
    {
        _username = Util::accessConfig(Action::IReadSecret,"Username");
        _password = Util::accessConfig(Action::IReadSecret,"Password");
    }

    if(_username.empty())
        Log::writeErrorLog("username not set");
    //test login
    //
    //
//create database
}

//TODO pas as reversne and no return
string WebDAV::getLocalPath(string path)
{
    Util::decodeUrl(path);
    if (path.find(NEXTCLOUD_ROOT_PATH) != string::npos)
        path = path.substr(NEXTCLOUD_ROOT_PATH.length());

    return NEXTCLOUD_FILE_PATH + "/" + path;
}

//TODO SQL CHeck before calling this function
vector<WebDAVItem> WebDAV::getDataStructure(const string &pathUrl)
{
    string xmlItem = propfind(pathUrl);


    size_t begin;
    size_t end;
    string beginItem = "<d:response>";
    string endItem = "</d:response>";
    vector<WebDAVItem> tempItems;
    WebDAVItem tempItem;

    begin = xmlItem.find(beginItem);

    while (begin != std::string::npos)
    {
        end = xmlItem.find(endItem);
        //tempItems.push_back(xml.substr(begin, end));

        tempItem.etag = Util::getXMLAttribute(xmlItem, "d:getetag");
        Log::writeInfoLog("etag " + tempItem.etag);
        Util::decodeUrl(tempItem.etag);
        Log::writeInfoLog("etag " + tempItem.etag);
        //TODO escape quot

        tempItem.path = Util::getXMLAttribute(xmlItem, "d:href");
        //TODO dont remove?
        //replaces everthing in front of /remote.php as this is already part of the url
        Log::writeInfoLog("path before transformation " + tempItem.path);
        if(tempItem.path.find(NEXTCLOUD_START_PATH) != 0)
        {
            tempItem.path.erase(0,tempItem.path.find(NEXTCLOUD_START_PATH));
            Log::writeInfoLog("path after transformation " + tempItem.path);
        }

    tempItem.lastEditDate = Util::getXMLAttribute(xmlItem, "d:getlastmodified");
    tempItem.title = tempItem.path; //TODO why twice?
    tempItem.localPath = getLocalPath(tempItem.path);

    if (tempItem.path.back() == '/')
    {
        tempItem.localPath = tempItem.localPath.substr(0, tempItem.localPath.length() - 1);
        //tempItem.type = Itemtype::IFOLDER;
        tempItem.title = tempItem.title.substr(0, tempItem.path.length() - 1);
        //_size = atof(Util::getXMLAttribute(xmlItem, "d:quota-used-bytes").c_str());
        //TODO is always 0?
        //Log::writeInfoLog("Size of folder " + std::to_string(_size));
        //TODO set sync status of folders --> use sqlite?
    }
    else
    {
        //tempItem.type = Itemtype::IFILE;
        tempItem.size = atof(Util::getXMLAttribute(xmlItem, "d:getcontentlength").c_str());
        tempItem.fileType = Util::getXMLAttribute(xmlItem, "d:getcontenttype");

        if (iv_access(tempItem.localPath.c_str(), W_OK) != 0)
        {
            //tempItem.state = FileState::ICLOUD;
        }
        else
        {
            //tempItem.state = FileState::ISYNCED;
        }
    }

    tempItem.title = tempItem.title.substr(tempItem.title.find_last_of("/") + 1, tempItem.title.length());
    Util::decodeUrl(tempItem.title);

    tempItems.push_back(tempItem);

        //TODO no creation mode known, fill in here

        xmlItem = xmlItem.substr(end + endItem.length());

        begin = xmlItem.find(beginItem);
    }

    if (tempItems.empty())
        return {};

    string localPath = getLocalPath(pathUrl);

    if (iv_access(localPath.c_str(), W_OK) != 0)
    {
        //if the current folder does not exist locally, create it
        iv_buildpath(localPath.c_str());
    }
    else
    {
        //get items from local path
        if (iv_access(localPath.c_str(), R_OK) != 0)
        {
            Log::writeInfoLog("Local structure of " + localPath + " found.");
        }

        //getLocalFileStructure(tempItems, localPath);
    }

    //update the .structure file acording to items in the folder
    //do not use anymore
    //localPath = localPath + NEXTCLOUD_STRUCTURE_EXTENSION;

    //save xml to make the structure available offline
    /*
    ofstream outFile(localPath);
    if (outFile)
    {
        outFile << readBuffer;
        Log::writeInfoLog("Saved local copy of tree structure to " + localPath);
    }
    else
    {
        Log::writeInfoLog(localPath + " Couldnt save copy of tree structure locally.");
    }
    */

    //outFile.close();
    return tempItems;
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

    auto _url = "test";

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

        /*
           if (iv_access(CACERT_PATH.c_str(), R_OK) == 0)
           curl_easy_setopt(curl, CURLOPT_CAINFO, CACERT_PATH.c_str());
           else
           Log::writeErrorLog("could not find cacert");
           */

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
            }
        }
        else
        {
            string response = std::string("An error occured. (") + curl_easy_strerror(res) + " (Curl Error Code: " + std::to_string(res) + ")). Please try again.";
            if(res == 60)
                response = "Seems as if you are using Let's Encrypt Certs. Please follow the guide on Github (https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client) to use a custom Cert Store on PB.";
            Message(ICON_ERROR, "Error", response.c_str(), 4000);
        }
    }
    return NULL;
}
