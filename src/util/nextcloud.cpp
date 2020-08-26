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

using std::string;

Nextcloud::Nextcloud()
{
    _loggedIn = false;

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
    _url.clear();
    _loggedIn = false;
    //TODO remove files
}

bool Nextcloud::downloadItem(int itemID)
{
    Log::writeLog("started download of " + _items[itemID].getPath() + " to " + _items[itemID].getLocalPath());

    if (!Util::connectToNetwork())
        return false;

    //create neccesary subfolders

    if (iv_access(_items[itemID].getLocalPath().c_str(), W_OK) != 0)
    {
        iv_buildpath(_items[itemID].getLocalPath().c_str());
        Log::writeLog("Created new path " + _items[itemID].getLocalPath());
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
                //TEMP
                Message(ICON_INFORMATION, "Information", ("finished download of " + _items[itemID].getPath() + " to " + _items[itemID].getLocalPath()).c_str(), 1200);
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

bool Nextcloud::getDataStructure(string &pathUrl)
{
    return getDataStructure(pathUrl, this->getUsername(), this->getPassword());
}

bool Nextcloud::getDataStructure(const string &pathUrl, const string &Username, const string &Pass)
{
    if (!Util::connectToNetwork())
        return false;

    if (_url.empty())
        _url = this->getUrl();

    if (Username.empty() || Pass.empty())
    {
        Message(ICON_ERROR, "Error", "Username/password not set.", 1200);
        return false;
    }

    _items.clear();
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
                size_t begin;
                size_t end;
                string beginItem = "<d:response>";
                string endItem = "</d:response>";

                begin = readBuffer.find(beginItem);

                while (begin != std::string::npos)
                {
                    end = readBuffer.find(endItem);

                    this->_items.push_back(Item(readBuffer.substr(begin, end)));

                    readBuffer = readBuffer.substr(end + endItem.length());

                    begin = readBuffer.find(beginItem);
                }

                if (_items.size() < 1)
                    return false;

                string tes = _items[0].getPath();
                tes = tes.substr(0, tes.find_last_of("/"));
                tes = tes.substr(0, tes.find_last_of("/") + 1);
                _items[0].setPath(tes);
                _items[0].setTitle("...");

                if (_items[0].getPath().compare(NEXTCLOUD_ROOT_PATH) == 0)
                    _items.erase(_items.begin());

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