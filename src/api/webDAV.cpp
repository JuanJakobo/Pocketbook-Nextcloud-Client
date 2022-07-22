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

#include <string>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <math.h>

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
        _username = Util::accessConfig(CONFIG_PATH,Action::IReadString,"username");
        _password = Util::accessConfig(CONFIG_PATH,Action::IReadSecret,"password");
        _url = Util::accessConfig(CONFIG_PATH, Action::IReadString, "url");
    }
}


//TODO pass in vector and change that one?
std::vector<WebDAVItem> WebDAV::login(const string &Url, const string &Username, const string &Pass)
{
    string uuid;
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
    std::vector<WebDAVItem> tempItems = getDataStructure(tempPath);
    if (!tempItems.empty())
    {
        if (iv_access(CONFIG_PATH.c_str(), W_OK) != 0)
            iv_buildpath(CONFIG_PATH.c_str());
        Util::accessConfig(CONFIG_PATH, Action::IWriteString, "url", _url);
        Util::accessConfig(CONFIG_PATH, Action::IWriteString, "username", Username);
        Util::accessConfig(CONFIG_PATH, Action::IWriteString, "uuid", uuid);
        Util::accessConfig(CONFIG_PATH, Action::IWriteSecret, "password", Pass);
    }
    return tempItems;
}

void WebDAV::logout(bool deleteFiles)
{
    if (deleteFiles)
    {
        //string cmd = "rm -rf " + NEXTCLOUD_FILE_PATH + "/" + getUUID() + "/";
        //system(cmd.c_str());
    }
    remove(CONFIG_PATH.c_str());
    remove((CONFIG_PATH + ".back.").c_str());

    //_url.clear();
    //TODO where?
    //tempItems.clear();
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

        //resize item 1
        string header = tempItems.at(0).path;
        header = header.substr(0, header.find_last_of("/"));
        header = header.substr(0, header.find_last_of("/") + 1);
        tempItems.at(0).path = header;
        tempItems.at(0).title += "\nclick to go back";
        tempItems.at(0).lastEditDate = "";
        if (tempItems.at(0).path.compare(NEXTCLOUD_ROOT_PATH) == 0)
            tempItems.erase(tempItems.begin());

        string localPath = getLocalPath(pathUrl);

        //if the current folder does not exist locally, create it
        if (iv_access(localPath.c_str(), W_OK) != 0)
            iv_buildpath(localPath.c_str());

        return tempItems;
    }
    return {};
    //TODO return empty items?
}
/*
void Nextcloud::downloadFolder(vector<Item> &tempItems, int itemID)
{

    //TODO etag
    BanSleep(2000);

    if (tempItems.at(itemID).getState() == FileState::ILOCAL)
    {
        UpdateProgressbar(("Removing local item " + tempItems.at(itemID).getLocalPath()).c_str(), 0);
        tempItems.at(itemID).removeFile();
    if (_type == Itemtype::IFOLDER)
    {
        string cmd = "rm -rf " + _localPath + "/";
        system(cmd.c_str());
        return true;
    }

    if (remove(_localPath.c_str()) != 0)
        return false;
    if (_state == FileState::ISYNCED || _state == FileState::IOUTSYNCED)
    {
        _state = FileState::ICLOUD;
    }
    else
    {
        //TODO applies if file is only local
        //only show if person is inside this folder
        Message(ICON_INFORMATION, "Warning", "The file will be shown until next folder update.", 1200);
    }
    return true;
        return;
    }

    //check files; where modified date --> modified date of structure
    //first check cloud, then check local

    if (tempItems.at(itemID).getType() == Itemtype::IFOLDER)
    {
        //check etag

        //for folders
        //1. check etag --> if is different, cloud has been updated --> need to dig in deeper here;
        //2. if is the same, only check local file system and dont show folder anymore
        string temp = tempItems.at(itemID).getPath();
        Log::writeInfoLog("Path to look for " + temp);
        vector<Item> tempItems = getDataStructure(temp);

        //first item of the vector is the root path itself
        for (size_t i = 1; i < tempItems.size(); i++)
        {
            Log::writeInfoLog("Item: " + tempItems.at(i).getPath());
            downloadFolder(tempItems, i);
        }
    }
    else
    {
        //for files
        //1. check etag --> if is differnt, cloud has been updated
        //2. check modification date and file size locally --> if is different, local has been updated

        //3. if both --> create conflict
        //4. if first, renew file --> reset etag
        //5. if second --> upload the local file; test if it has not been update in the cloud
        Log::writeInfoLog("started download of " + tempItems.at(itemID).getPath() + " to " + tempItems.at(itemID).getLocalPath());
        get(tempItems, itemID);
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

    this->downloadFolder(tempItems, itemID);

    UpdateProgressbar("Download completed", 100);
}
*/


string WebDAV::propfind(const string &pathUrl)
{
       //TODO catch here pathUrl is empty!
       if (pathUrl.empty())
           return "";

       //where to test this??
       //include error messsages
       if(_username.empty() || _password.empty())
           return "";

       //string localPath = getLocalPath(pathUrl);

       if (!Util::connectToNetwork())
       {
           Message(ICON_WARNING, "Warning", "Cannot connect to the internet. Switching to offline modus. To work online turn on online modus in the menu.", 2000);
           return "";
       }
       //
        //get etag from current and then send request with FT_ENC_TAG
        //need path url and also the etag

        //can handle multiple etags --> * if exists

*/


    string readBuffer;
    CURLcode res;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        string post = _username + ":" + _password;
        Log::writeInfoLog(_url + pathUrl);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Depth: 1");
        curl_easy_setopt(curl, CURLOPT_URL, (_url + pathUrl).c_str());
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

/*
void WebDAV::get(vector<Item> &tempItems, int itemID)
{
    //CHECK id
    if (tempItems.at(itemID).getState() == FileState::ISYNCED)
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
        fp = iv_fopen(tempItems.at(itemID).getLocalPath().c_str(), "wb");

        curl_easy_setopt(curl, CURLOPT_URL, (_url + tempItems.at(itemID).getPath()).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, Util::progress_callback);
        //in case that cacert is available use it
        if (iv_access(CACERT_PATH.c_str(), W_OK) == 0)
            curl_easy_setopt(curl, CURLOPT_CAINFO, CACERT_PATH.c_str());
        else
            Log::writeErrorLog("could not find cacert");
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
                Log::writeInfoLog("finished download of " + tempItems.at(itemID).getPath() + " to " + tempItems.at(itemID).getLocalPath());
                tempItems.at(itemID).setState(FileState::ISYNCED);
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
            if(res == 60)
                response = "Seems as if you are using Let's Encrypt Certs. Please follow the guide on Github (https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client) to use a custom Cert Store on PB.";
            Message(ICON_ERROR, "Error", response.c_str(), 4000);
        }
    }
}
*/
