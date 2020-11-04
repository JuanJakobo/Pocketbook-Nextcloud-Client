//------------------------------------------------------------------
// nextcloud.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "nextcloud.h"
#include "util.h"
#include "item.h"
#include "log.h"

#include <string>
#include <curl/curl.h>
#include <fstream>
#include <sstream>

using std::ifstream;
using std::ofstream;
using std::string;

//neccesary to use Dialog method
Nextcloud *Nextcloud::nextcloudStatic;

Nextcloud::Nextcloud()
{
    nextcloudStatic = this;

    if (iv_access(NEXTCLOUD_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_PATH.c_str(), 0777);

    if (iv_access(NEXTCLOUD_FILE_PATH.c_str(), W_OK) != 0)
        iv_mkdir(NEXTCLOUD_FILE_PATH.c_str(), 0777);
}

void Nextcloud::setURL(const string &Url)
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    WriteString(nextcloudConfig, "url", Url.c_str());
    CloseConfig(nextcloudConfig);
}

void Nextcloud::setUsername(const string &Username)
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    WriteString(nextcloudConfig, "username", Username.c_str());
    CloseConfig(nextcloudConfig);
}

void Nextcloud::setPassword(const string &Pass)
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    WriteSecret(nextcloudConfig, "password", Pass.c_str());
    CloseConfig(nextcloudConfig);
}

bool Nextcloud::login()
{
    if (getDataStructure(NEXTCLOUD_ROOT_PATH + this->getUsername() + "/", this->getUsername(), this->getPassword()))
    {
        _loggedIn = true;
        return true;
    }

    return false;
}

bool Nextcloud::login(const string &Url, const string &Username, const string &Pass)
{
    _url = Url;
    if (getDataStructure(NEXTCLOUD_ROOT_PATH + Username + "/", Username, Pass))
    {
        if (iv_access(NEXTCLOUD_CONFIG_PATH.c_str(), W_OK) != 0)
            iv_buildpath(NEXTCLOUD_CONFIG_PATH.c_str());
        this->setUsername(Username);
        this->setPassword(Pass);
        this->setURL(_url);
        _loggedIn = true;
        return true;
    }
    return false;
}

void Nextcloud::logout()
{
    remove(NEXTCLOUD_CONFIG_PATH.c_str());
    remove((NEXTCLOUD_CONFIG_PATH + ".back.").c_str());
    _url.clear();
    _loggedIn = false;
}

bool Nextcloud::downloadItem(int itemID)
{
    Log::writeLog("started download of " + _items[itemID].getPath() + " to " + _items[itemID].getLocalPath());

    if (_workOffline)
    {
        int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "You are in offline modus. Go back online?", "Yes", "No", "Cancel");

        if (dialogResult == 2 || dialogResult == 3)
            return false;
    }

    if (!Util::connectToNetwork())
    {
        Message(3, "Warning", "Can not connect to the Internet. Switching to offline modus.", 200);
        _workOffline = true;
        return false;
    }

    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = this->getUsername() + std::string(":") + this->getPassword();

        FILE *fp;
        fp = iv_fopen(_items[itemID].getLocalPath().c_str(), "wb");

        curl_easy_setopt(curl, CURLOPT_URL, (_url + _items[itemID].getPath()).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, Util::progress_callback);

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
                Log::writeLog("finished download of " + _items[itemID].getPath() + " to " + _items[itemID].getLocalPath());
                _items[itemID].setDownloaded(true);
                return true;
            case 401:
                Message(ICON_ERROR, "Error", "Username/password incorrect.", 1200);
                break;
            default:
                Message(ICON_ERROR, "Error", "An unknown error occured.", 1200);
                break;
            }
        }
    }
    return false;
}


bool Nextcloud::removeFile(int itemID)
{
    remove(_items[itemID].getLocalPath().c_str());
    _items[itemID].setDownloaded(false);
}

bool Nextcloud::getDataStructure(string &pathUrl)
{
    return getDataStructure(pathUrl, this->getUsername(), this->getPassword());
}

bool Nextcloud::getDataStructure(const string &pathUrl, const string &Username, const string &Pass)
{
    //could not connect to internet, therefore offline modus
    if (_workOffline)
        return getOfflineStructure(pathUrl);

    if (!Util::connectToNetwork())
    {
        Message(3, "Warning", "Cannot connect to the internet. Switching to offline modus. To work online turn on online modus in the menu.", 200);
        _workOffline = true;
        return getOfflineStructure(pathUrl);
    }

    if (_url.empty())
        _url = this->getUrl();

    if (Username.empty() || Pass.empty())
    {
        Message(ICON_ERROR, "Error", "Username/password not set.", 1200);
        return false;
    }

    string readBuffer;
    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = Username + ":" + Pass;

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Depth: 1");
        curl_easy_setopt(curl, CURLOPT_URL, (_url + pathUrl).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            switch (response_code)
            {
            case 207:
            {
                if (!readInXML(readBuffer))
                    return false;

                string localPath = this->getLocalPath(pathUrl);

                //create neccesary subfolders
                if (iv_access(localPath.c_str(), W_OK) != 0)
                    iv_buildpath(localPath.c_str());

                localPath = localPath + NEXTCLOUD_STRUCTURE_EXTENSION;

                //check for difference local and server
                if (iv_access(localPath.c_str(), R_OK) != 0)
                {
                    Log::writeLog("Local structure of " + localPath + " found.");

                    //TODO get items from local path
                    //compare local and online, if online no longer availaible delete
                    //if is in old .structure --> delete from pocketbook and check for last edit date
                    //delete old structure file
                }

                //TODO if local file is newer than of old structure --> upload
                //if is not in old file and not in new, ask if should be uploaded

                //save xml to make the structure available offline
                ofstream outFile(localPath);
                if (outFile)
                {
                    outFile << readBuffer;
                    Log::writeLog("Saved local copy of tree structure to " + localPath);
                }
                else
                {
                    Log::writeLog(localPath + "Couldnt save copy of tree structure locally.");
                }

                return true;
                break;
            }
            case 401:
                Message(ICON_ERROR, "Error", "Username/password incorrect.", 1200);
                break;
            default:
                Message(ICON_ERROR, "Error", "An unknown error occured." + response_code, 1200);
                break;
            }
        }
    }
    return false;
}

string Nextcloud::getUrl()
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    string url = ReadString(nextcloudConfig, "url", "");
    CloseConfigNoSave(nextcloudConfig);
    return url;
}

string Nextcloud::getUsername()
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    string user = ReadString(nextcloudConfig, "username", "");
    CloseConfigNoSave(nextcloudConfig);
    return user;
}

string Nextcloud::getPassword()
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    string pass = ReadSecret(nextcloudConfig, "password", "");
    CloseConfigNoSave(nextcloudConfig);
    return pass;
}

void Nextcloud::DialogHandlerStatic(int Button)
{
    if (Button == 2)
    {
        nextcloudStatic->_workOffline = true;
    }
}

bool Nextcloud::readInXML(string xml)
{
    size_t begin;
    size_t end;
    string beginItem = "<d:response>";
    string endItem = "</d:response>";

    _items.clear();

    begin = xml.find(beginItem);

    while (begin != std::string::npos)
    {
        end = xml.find(endItem);

        //TODO copy array and here only temp
        this->_items.push_back(Item(xml.substr(begin, end)));

        xml = xml.substr(end + endItem.length());

        begin = xml.find(beginItem);
    }

    if (_items.size() < 1)
        return false;

    //resize item 1
    string header = _items[0].getPath();
    header = header.substr(0, header.find_last_of("/"));
    header = header.substr(0, header.find_last_of("/") + 1);
    _items[0].setPath(header);
    _items[0].setTitle("...");
    _items[0].setLastEditDate("");

    if (_items[0].getPath().compare(NEXTCLOUD_ROOT_PATH) == 0)
        _items.erase(_items.begin());

    return true;
}

string Nextcloud::getLocalPath(string path)
{
    if (path.find(NEXTCLOUD_ROOT_PATH) != string::npos)
        path = path.substr(NEXTCLOUD_ROOT_PATH.length());

    return NEXTCLOUD_FILE_PATH + "/" + path;
}

bool Nextcloud::getOfflineStructure(const string &pathUrl)
{
    string localPath = this->getLocalPath(pathUrl) + NEXTCLOUD_STRUCTURE_EXTENSION;
    if (iv_access(localPath.c_str(), W_OK) == 0)
    {
        ifstream inFile(localPath);
        std::stringstream buffer;
        buffer << inFile.rdbuf();

        if (!readInXML(buffer.str()))
            return false;
    }
    else
    {
        if (localPath.find(NEXTCLOUD_ROOT_PATH) != string::npos)
        {
            Message(ICON_ERROR, "Error", "The root structure is not available offline. To try again to connect turn on online modus in the menu.", 1200);

        }
        else
        {
            //Structure is not available offline, stay at the tree
            Message(ICON_ERROR, "Error", "The selected structure is not available offline.", 1200);

            return true;
        }
    }
}
