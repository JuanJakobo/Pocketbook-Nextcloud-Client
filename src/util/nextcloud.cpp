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

#include <string>
#include <curl/curl.h>

using namespace std;

Nextcloud::Nextcloud()
{
    loggedIn = false;

    if(iv_access(NEXTCLOUD_PATH.c_str(), W_OK)!=0)
        iv_mkdir(NEXTCLOUD_PATH.c_str(),0777);

    if(iv_access(NEXTCLOUD_FILE_PATH.c_str(),W_OK)!=0)
        iv_mkdir(NEXTCLOUD_FILE_PATH.c_str(),0777);
}

void Nextcloud::setUsername(const string& Username)
{
    iconfigedit* temp = nullptr;
    iconfig  *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(),temp);
    WriteString(nextcloudConfig,"username",Username.c_str());
    CloseConfig(nextcloudConfig);
}

void Nextcloud::setPassword(const string& Pass)
{
    iconfigedit* temp = nullptr;
    iconfig  *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(),temp);
    WriteSecret(nextcloudConfig,"password",Pass.c_str());
    CloseConfig(nextcloudConfig);
}

string Nextcloud::getUsername()
{
    iconfigedit* temp = nullptr;
    iconfig  *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(),temp);
    string user = ReadString(nextcloudConfig,"username","");
    CloseConfigNoSave(nextcloudConfig);
    return user;
}

string Nextcloud::getPassword()
{
    iconfigedit* temp = nullptr;
    iconfig  *nextcloudConfig = OpenConfig(NEXTCLOUD_CONFIG_PATH.c_str(),temp);
    string pass = ReadSecret(nextcloudConfig,"password","");
    CloseConfigNoSave(nextcloudConfig);
    return pass;
}

bool Nextcloud::login(const string& Username, const string& Pass)
{
    if(getDataStructure(NEXTCLOUD_ROOT_PATH + Username + "/",Username,Pass))
    {
        if(iv_access(NEXTCLOUD_CONFIG_PATH.c_str(), W_OK)!=0)
            iv_buildpath(NEXTCLOUD_CONFIG_PATH.c_str());
        this->setUsername(Username);
        this->setPassword(Pass);
        loggedIn = true;
        return true;
    }
    return false;
}

bool Nextcloud::getDataStructure(string& pathUrl)
{
    return getDataStructure(pathUrl,this->getUsername(),this->getPassword());
}


bool Nextcloud::getDataStructure(const string& pathUrl, const string& Username, const string& Pass)
{
    if(!Util::connectToNetwork())
        return false;

    if(Username.empty() || Pass.empty())
    {
        Message(ICON_ERROR, "Error", "Username/password not set.", 1200);
        return false;
    }

    items.clear();
    string readBuffer;
    CURLcode res;
    CURL *curl = curl_easy_init();

    if(curl)
    {
        string post = Username + ":" + Pass;

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Depth: 1");
        curl_easy_setopt(curl, CURLOPT_URL, (NEXTCLOUD_URL + pathUrl).c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, post.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            switch (response_code)
            {
            case 207:
                {
                    size_t found;
                    string searchString = "<d:href>";            
                    do
                    {
                        found = std::string::npos;
                        found = readBuffer.find(searchString);
                        if (found!=std::string::npos)
                        {
                            readBuffer = readBuffer.substr(found+searchString.length());
                            found = readBuffer.find("</d:href>");
                            this->items.push_back(Item(readBuffer.substr(0,found)));
                        }
                    } while (found!=std::string::npos);

                    if(items.size() < 1)
                        return false;

                    string tes = items[0].getPath();
                    tes = tes.substr(0,tes.find_last_of("/"));
                    tes = tes.substr(0,tes.find_last_of("/")+1);
                    items[0].setPath(tes);
                    if(items[0].getPath().compare(NEXTCLOUD_ROOT_PATH) == 0)
                        items.erase(items.begin());

                    return true;
                    break;
                }
            case 401:
                Message(ICON_ERROR, "Error","Username/password incorrect.", 1200);
                break;
            default:
                Message(ICON_ERROR,"Error","An unknown error occured." + response_code,1200);
                break;
            }
        }
    }
    return false;
}
