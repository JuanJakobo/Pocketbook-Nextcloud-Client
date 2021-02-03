//------------------------------------------------------------------
// util.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------
#include "util.h"
#include "inkview.h"

#include <string>
#include <math.h>
#include <curl/curl.h>

using std::string;

size_t Util::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

size_t Util::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = iv_fwrite(ptr, size, nmemb, stream);
    return written;
}

bool Util::connectToNetwork()
{
    iv_netinfo *netinfo = NetInfo();
    if (netinfo->connected)
        return true;

    const char *network_name = nullptr;
    int result = NetConnect2(network_name, 1);
    if (result != 0)
    {
        return false;
    }

    netinfo = NetInfo();
    if (netinfo->connected)
        return true;

    return false;
}

int Util::progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (dltotal <= 0.0)
        return 0;

    int percentage = round(dlnow / dltotal * 100);
    if (percentage % 10 == 0)
        UpdateProgressbar("Downloading file", percentage);

    return 0;
}

string Util::getXMLAttribute(const string &buffer, const string &name)
{
    string returnString = buffer;
    string searchString = "<" + name + ">";
    size_t found = buffer.find(searchString);

    if (found != std::string::npos)
    {
        returnString = returnString.substr(found + searchString.length());

        return returnString.substr(0, returnString.find("</" + name + ">"));
    }

    return NULL;
}

void Util::decodeUrl(string &item)
{
    char *buffer;
    CURL *curl = curl_easy_init();

    buffer = curl_easy_unescape(curl,item.c_str(),0,NULL);
    item =  buffer;

    curl_free(buffer)
    curl_easy_cleanup(curl);
}