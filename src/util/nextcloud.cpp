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

Nextcloud::Nextcloud()
{

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

void Nextcloud::setStartFolder(const string &Path)
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    WriteString(nextcloudConfig, "startFolder", Path.c_str());
    CloseConfig(nextcloudConfig);
}

bool Nextcloud::setItems(const vector<Item> &tempItems)
{

    if (tempItems.empty())
        return false;

    if (!_items.empty())
        _items.clear();

    _items = tempItems;

    //resize item 1
    string header = _items.at(0).getPath();
    header = header.substr(0, header.find_last_of("/"));
    header = header.substr(0, header.find_last_of("/") + 1);
    _items.at(0).setPath(header);
    _items.at(0).setTitle("...");
    _items.at(0).setLastEditDate("");

    if (_items.at(0).getPath().compare(NEXTCLOUD_ROOT_PATH) == 0)
        _items.erase(_items.begin());

    return true;
}

bool Nextcloud::login()
{
    string tempPath = getStartFolder();

    if (tempPath.empty())
        tempPath = NEXTCLOUD_ROOT_PATH + this->getUsername() + "/";

    if (setItems(getDataStructure(tempPath)))
    {
        _loggedIn = true;
        return true;
    }

    return false;
}

bool Nextcloud::login(const string &Url, const string &Username, const string &Pass)
{
    _url = Url;
    string tempPath = NEXTCLOUD_ROOT_PATH + Username + "/";
    if (setItems(getDataStructure(tempPath, Username, Pass)))
    {
        Log::writeLog("Got items");

        if (iv_access(NEXTCLOUD_CONFIG_PATH.c_str(), W_OK) != 0)
            iv_buildpath(NEXTCLOUD_CONFIG_PATH.c_str());
        this->setUsername(Username);
        this->setPassword(Pass);
        this->setURL(_url);
        this->setStartFolder(tempPath);
        _loggedIn = true;
        return true;
    }
    return false;
}

void Nextcloud::logout(bool deleteFiles)
{
    if (deleteFiles)
    {
        string cmd = "rm -rf " + NEXTCLOUD_FILE_PATH + "/" + getUsername() + "/";
        system(cmd.c_str());
    }
    remove(NEXTCLOUD_CONFIG_PATH.c_str());
    remove((NEXTCLOUD_CONFIG_PATH + ".back.").c_str());

    _url.clear();
    _items.clear();
    _workOffline = false;
    _loggedIn = false;
}

void Nextcloud::downloadItem(vector<Item> &tempItems, int itemID)
{
    if(tempItems.at(itemID).getState() == FileState::ISYNCED)
    {
        UpdateProgressbar(("The newest version of file " + tempItems.at(itemID).getLocalPath() + " is already downloaded.").c_str(), 0);
        return;
    }

    if (tempItems.at(itemID).getPath().empty())
    {
        Message(ICON_ERROR, "Error", "Download path is not set, therefore cannot download the file.", 2000);
        return;
    }

    UpdateProgressbar(("Starting Download of " + tempItems.at(itemID).getLocalPath()).c_str(), 0);

    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = this->getUsername() + std::string(":") + this->getPassword();

        FILE *fp;
        fp = iv_fopen(tempItems.at(itemID).getTitle().c_str(), "wb");

        curl_easy_setopt(curl, CURLOPT_URL, (_url + tempItems.at(itemID).getPath()).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, Util::progress_callback);
        //Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
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
                Log::writeLog("finished download of " + tempItems.at(itemID).getPath() + " to " + tempItems.at(itemID).getLocalPath());
                tempItems.at(itemID).setState(FileState::ISYNCED);
                break;
            case 401:
                Message(ICON_ERROR, "Error", "Username/password incorrect.", 2000);
                break;
            default:
                Message(ICON_ERROR, "Error", ("An unknown error occured. (Curl Response Code " + Util::valueToString(response_code) + ")").c_str(), 2000);
                break;
            }
        }
        else
        {
            Message(ICON_ERROR, "Error", ("An curl error occured (Error Code: " + Util::valueToString(res) + ").").c_str(), 4000);
        }
    }
}

void Nextcloud::downloadFolder(vector<Item> &tempItems, int itemID)
{
    BanSleep(2000);
    if (tempItems.at(itemID).getType() == Itemtype::IFOLDER)
    {
        string temp = tempItems.at(itemID).getPath();
        Log::writeLog("Path to look for " + temp);
        vector<Item> tempItems = getDataStructure(temp);

        //first item of the vector is the root path itself
        for (auto i = 1; i < tempItems.size(); i++)
        {
            Log::writeLog("Item: " + tempItems.at(i).getPath());
            downloadFolder(tempItems, i);
        }
    }
    else
    {
        Log::writeLog("started download of " + _items.at(itemID).getPath() + " to " + _items.at(itemID).getLocalPath());
        downloadItem(tempItems, itemID);
    }

    return;
}

void Nextcloud::download(int itemID)
{
    if (!Util::connectToNetwork())
    {
        Message(ICON_WARNING, "Warning", "Can not connect to the Internet. Switching to offline modus.", 2000);
        _workOffline = true;
        return;
    }

    this->downloadFolder(_items, itemID);
}

bool Nextcloud::removeItem(int itemID)
{
    Log::writeLog("removing file " + _items.at(itemID).getPath());
    if (!_items.at(itemID).removeFile())
        return false;

    return true;
}

vector<Item> Nextcloud::getDataStructure(string &pathUrl)
{
    return getDataStructure(pathUrl, this->getUsername(), this->getPassword());
}

vector<Item> Nextcloud::getDataStructure(const string &pathUrl, const string &Username, const string &Pass)
{
    if (_workOffline)
        return getOfflineStructure(pathUrl);

    if (!Util::connectToNetwork())
    {
        Message(ICON_WARNING, "Warning", "Cannot connect to the internet. Switching to offline modus. To work online turn on online modus in the menu.", 2000);
        _workOffline = true;
        return getOfflineStructure(pathUrl);
    }

    if (_url.empty())
        _url = this->getUrl();

    if (Username.empty() || Pass.empty())
    {
        Message(ICON_ERROR, "Error", "Username/password not set.", 2000);
        return {};
    }

    Log::writeLog("Starting download of DataStructure");

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
            case 404:
                Message(ICON_ERROR, "Error", "The nextcloud URL seems to be incorrect. You can look up the WebDav URL in the files app->seetings. (The nextcloud URL is the part till \"/remote.php...\".)", 4000);
                break;
            case 401:
                Message(ICON_ERROR, "Error", "Username/password incorrect.", 4000);
                break;
            case 207:
            {
                string localPath = this->getLocalPath(pathUrl);

                //create items_
                vector<Item> tempItems = readInXML(readBuffer);
                if (tempItems.empty())
                    return {};

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
                        Log::writeLog("Local structure of " + localPath + " found.");
                    }

                    getLocalFileStructure(tempItems, localPath);
                }

                //update the .structure file acording to items in the folder
                localPath = localPath + NEXTCLOUD_STRUCTURE_EXTENSION;

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

                outFile.close();
                return tempItems;
            }
            default:
                Message(ICON_ERROR, "Error", ("An unknown error occured. Switching to offline modus. To work online turn on online modus in the menu. (Curl Response Code " + Util::valueToString(response_code) + ")").c_str(), 4000);
                _workOffline = true;
                return getOfflineStructure(pathUrl);
            }
        }
        else
        {
            Message(ICON_ERROR, "Error", ("An curl error occured (Error Code: " + Util::valueToString(res) + ").").c_str(), 4000);
        }
    }
    return {};
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

string Nextcloud::getStartFolder()
{
    iconfigedit *temp = nullptr;
    iconfig *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(), temp);
    string startFolder = ReadString(nextcloudConfig, "startFolder", "");
    CloseConfigNoSave(nextcloudConfig);
    return startFolder;
}

vector<Item> Nextcloud::readInXML(string xml)
{
    size_t begin;
    size_t end;
    string beginItem = "<d:response>";
    string endItem = "</d:response>";
    vector<Item> tempItems;

    begin = xml.find(beginItem);

    while (begin != std::string::npos)
    {
        end = xml.find(endItem);

        tempItems.push_back(xml.substr(begin, end));

        xml = xml.substr(end + endItem.length());

        begin = xml.find(beginItem);
    }

    return tempItems;
}

string Nextcloud::getLocalPath(string path)
{
    Util::decodeUrl(path);
    if (path.find(NEXTCLOUD_ROOT_PATH) != string::npos)
        path = path.substr(NEXTCLOUD_ROOT_PATH.length());

    return NEXTCLOUD_FILE_PATH + "/" + path;
}

vector<Item> Nextcloud::getOfflineStructure(const string &pathUrl)
{
    string localPath = this->getLocalPath(pathUrl) + NEXTCLOUD_STRUCTURE_EXTENSION;
    if (iv_access(localPath.c_str(), W_OK) == 0)
    {
        ifstream inFile(localPath);
        std::stringstream buffer;
        buffer << inFile.rdbuf();

        vector<Item> tempItems = readInXML(buffer.str());

        if (tempItems.empty())
            return {};

        getLocalFileStructure(tempItems, this->getLocalPath(pathUrl));
        return tempItems;
    }
    else
    {
        if (pathUrl.compare(NEXTCLOUD_ROOT_PATH + getUsername() + "/") == 0)
        {
            Message(ICON_ERROR, "Error", "The root structure is not available offline. Please try again to login.", 2000);
            logout();
        }
        else
        {
            //Structure is not available offline, stay at the tree
            Message(ICON_ERROR, "Error", "The selected structure is not available offline.", 2000);
        }
    }
    return {};
}

void Nextcloud::getLocalFileStructure(vector<Item> &tempItems, const string &localPath)
{
    //get local files, https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
    DIR *dir;
    class dirent *ent;
    class stat st;

    dir = opendir(localPath.c_str());
    while ((ent = readdir(dir)) != NULL)
    {
        const string fileName = ent->d_name;
        const string fullFileName = localPath + fileName;

        if (fileName[0] == '.')
            continue;

        if (stat(fullFileName.c_str(), &st) == -1)
            continue;

        const bool isDirectory = (st.st_mode & S_IFDIR) != 0;
        if (isDirectory)
            continue;

        bool found = false;
        for (auto i = 0; i < tempItems.size(); i++)
        {
            if (tempItems.at(i).getLocalPath().compare(fullFileName) == 0)
            {
                std::ifstream in(fullFileName, std::ifstream::binary | std::ifstream::ate );
                Log::writeLog(Util::valueToString(in.tellg()));
                Log::writeLog(Util::valueToString(tempItems.at(i).getSize()));
                if(in.tellg() != tempItems.at(i).getSize())
                {
                    tempItems.at(i).setState(FileState::IOUTSYNCED);
                }
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (isDirectory)
            {
                tempItems.push_back(Item(fullFileName, FileState::ILOCAL, Itemtype::IFOLDER));
            }
            else
            {
                tempItems.push_back(Item(fullFileName, FileState::ILOCAL, Itemtype::IFILE));
            }
        }
    }
    closedir(dir);
}
