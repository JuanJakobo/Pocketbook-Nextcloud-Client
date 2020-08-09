//------------------------------------------------------------------
// item.cpp
//
// Author:           JuanJakobo          
// Date:             04.08.2020
//  
//-------------------------------------------------------------------

#include "item.h"
#include "inkview.h"
#include "util.h"
#include "nextcloud.h"
#include "log.h"

#include <string>
#include <curl/curl.h>

using namespace std;

Item::Item(const string& xmlItem)
{
        path = Util::getXMLAttribute(xmlItem,"d:href");
        lastEdithDate = Util::getXMLAttribute(xmlItem,"d:getlastmodified");
        title = path;

        if(path.back()=='/')
        {
            type=IFOLDER;
            title = title.substr(0,path.length()-1);
            size = Util::getXMLAttribute(xmlItem,"d:quota-used-bytes");
        }
        else
        {
            type=IFILE;
            size = Util::getXMLAttribute(xmlItem,"d:getcontentlength");
            fileType = Util::getXMLAttribute(xmlItem,"d:getcontenttype");
            downloaded = false;
        }

        title = title.substr(title.find_last_of("/")+1,title.length());
}

string Item::isClicked()
{
    if(type==IFILE)
    {
        //downloadFile();
    }
    else
    {
        return path;
    }

    return "";
}