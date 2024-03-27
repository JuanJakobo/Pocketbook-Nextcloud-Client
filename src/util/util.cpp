//------------------------------------------------------------------
// util.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------
#include "util.h"

#include <curl/curl.h>
#include <math.h>
#include <signal.h>

#include <experimental/filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <tuple>

#include "inkview.h"
#include "log.h"

#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace fs = std::experimental::filesystem;

namespace
{
constexpr auto UPDATE_FILE{"/mnt/ext1/system/config/nextcloud/updater"};
const auto SCANNER_APP_CMD{"/ebrmain/bin/scanner.app 1> "s + UPDATE_FILE + " 2>&1"s};
constexpr auto UPDATE_END{"Scan total"};

constexpr auto TEXT_DOWNLOADING_FILE{"Downloading file"};
constexpr auto TEXT_FAILED_TO_READ_IN_DATA{"Failed to read in data format."};
constexpr auto TEXT_FAILED_TO_READ_OUT_DATA{"Failed to read out data format."};
constexpr auto TEXT_NO_INTERNET_CONNECTION{"It was not possible to establish an internet connection."};
constexpr auto TEXT_UPDATING_PB_LIB{"Updating PB library"};

} // namespace

size_t Util::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int Util::writeData(void *ptr, int size, int nmemb, FILE *stream)
{
    auto written{iv_fwrite(ptr, size, nmemb, stream)};
    return written;
}

// https://github.com/pmartin/pocketbook-demo/blob/master/devutils/wifi.cpp
bool Util::connectToNetwork()
{
    iv_netinfo *netinfo = NetInfo();
    if (netinfo->connected)
        return true;

    const char *network_name = nullptr;
    int result = NetConnect2(network_name, true);
    if (result)
    {
        Message(ICON_WARNING, TEXT_MESSAGE_WARNING, TEXT_NO_INTERNET_CONNECTION, TIMEOUT_MESSAGE);
        return false;
    }

    netinfo = NetInfo();
    if (netinfo->connected)
        return true;

    Message(ICON_WARNING, TEXT_MESSAGE_WARNING, TEXT_NO_INTERNET_CONNECTION, TIMEOUT_MESSAGE);
    return false;
}

int Util::progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    std::ignore = ultotal;
    std::ignore = ulnow;
    free(clientp);
    if (dltotal <= 0.0)
        return 0;

    int percentage = round(dlnow / dltotal * 100);
    if (percentage % 10 == 0)
        UpdateProgressbar(TEXT_DOWNLOADING_FILE, percentage);

    return 0;
}

string Util::getXMLAttribute(const string &buffer, const string &name)
{
    auto returnString{buffer};
    const auto searchString{"<" + name + ">"};
    const auto found{buffer.find(searchString)};

    if (found != std::string::npos)
    {
        returnString = returnString.substr(found + searchString.length());

        return returnString.substr(0, returnString.find("</" + name + ">"));
    }

    return NULL;
}

void Util::decodeUrl(string &text)
{
    char *buffer;
    auto curl{curl_easy_init()};

    buffer = curl_easy_unescape(curl, text.c_str(), 0, NULL);
    text = buffer;

    curl_free(buffer);
    curl_easy_cleanup(curl);
}

void Util::encodeUrl(string &text)
{
    char *buffer;
    auto curl{curl_easy_init()};

    buffer = curl_easy_escape(curl, text.c_str(), 0);
    text = buffer;

    curl_free(buffer);
    curl_easy_cleanup(curl);
}

void Util::updatePBLibrary()
{
    UpdateProgressbar(TEXT_UPDATING_PB_LIB, 99);
    auto child_pid{fork()};
    if (child_pid > 0)
    {
        //parent
        std::string result;
        auto counter{0};
        while (result.find(UPDATE_END) == std::string::npos && counter <= 5)
        {
            sleep(2);
            std::ifstream ifs(UPDATE_FILE);
            std::ostringstream sstr;
            sstr << ifs.rdbuf();
            result = sstr.str();
            counter++;

        }
        fs::remove(UPDATE_FILE);
        //TODO does not kill
        kill(child_pid, SIGKILL);
    }
    else if (child_pid == 0)
    {
        //child
        setpgid(getpid(), getpid());
        std::system(SCANNER_APP_CMD.c_str());
        //command runs forever, therefore use command above to kill it
        exit(1);
    }
}

tm Util::webDAVStringToTm(const std::string &timestring)
{
    tm t{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::istringstream ss(timestring);
    // format depends on nextcloud config?
    // weekday, day month year Hour Minute Second Timezone
    ss >> std::get_time(&t, "%a, %d %b %Y %H:%M:%S");
    if (ss.fail())
        Log::writeErrorLog(TEXT_FAILED_TO_READ_IN_DATA);
    return t;
}

string Util::webDAVTmToString(const tm &timestring)
{
    std::ostringstream ss;
    string result = {};
    // format depends on nextcloud config?
    // weekday, day month year Hour Minute Second Timezone
    ss << std::put_time(&timestring, "%a, %d %b %Y %H:%M:%S");
    if (ss.fail())
        Log::writeErrorLog(TEXT_FAILED_TO_READ_OUT_DATA);
    else
        result = ss.str();
    return result;
}
