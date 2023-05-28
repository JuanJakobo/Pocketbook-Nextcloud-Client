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

#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

#include "inkview.h"
#include "log.h"

namespace
{
constexpr auto SCANNER_APP_LOCATION{"/ebrmain/bin/scanner.app"};

constexpr auto TEXT_DOWNLOADING_FILE{"Downloading file"};
constexpr auto TEXT_FAILED_TO_READ_IN_DATA{"Failed to read in data format."};
constexpr auto TEXT_FAILED_TO_READ_OUT_DATA{"Failed to read out data format."};
constexpr auto TEXT_NO_INTERNET_CONNECTION{"It was not possible to establish an internet connection."};
constexpr auto TEXT_UPDATING_PB_LIB{"Updating PB library"};

} // namespace

pid_t child_pid = -1; // Global

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

void kill_child(int sig)
{
    // SIGKILL
    kill(child_pid, SIGTERM);
}

void Util::updatePBLibrary(int seconds)
{
    UpdateProgressbar(TEXT_UPDATING_PB_LIB, 99);
    // https://stackoverflow.com/questions/6501522/how-to-kill-a-child-process-by-the-parent-process
    signal(SIGALRM, (void (*)(int))kill_child);
    child_pid = fork();
    if (child_pid > 0)
    {
        // parent
        alarm(seconds);
        wait(NULL);
    }
    else if (child_pid == 0)
    {
        // child
        // TODO parse in response of exec to determine when to kill?
        execlp(SCANNER_APP_LOCATION, SCANNER_APP_LOCATION, (char *)NULL);
        exit(1);
    }
}

tm Util::webDAVStringToTm(const std::string &timestring)
{
    tm t = {0};
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
