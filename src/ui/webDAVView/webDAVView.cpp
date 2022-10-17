//------------------------------------------------------------------
// webDAVView.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "webDAVView.h"
#include "webDAVModel.h"
#include "webDAV.h"
#include "util.h"

#include <string>
#include <vector>
#include <algorithm>

using std::vector;

WebDAVView::WebDAVView(const irect &contentRect, vector<WebDAVItem> &itemsUnfiltered, int page) : ListView(contentRect, page)
{
    vector<WebDAVItem> items;
    std::copy_if (itemsUnfiltered.begin(), itemsUnfiltered.end(), std::back_inserter(items), [](WebDAVItem i)
    {
        return i.hide != HideState::IHIDE;
    });

    auto pageHeight = 0;
    auto contentHeight = _contentRect.h - _footerHeight;
    auto entrycount = items.size();

    _entries.reserve(entrycount);

    //resize item 1
    std::string header = items.at(0).path;
    header = header.substr(0, header.find_last_of("/"));
    header = header.substr(0, header.find_last_of("/") + 1);
    items.at(0).path = header;
    items.at(0).title += "\nclick to go back";
    items.at(0).lastEditDate.tm_year = 2100;

    std::vector<WebDAVItem>::iterator begin;

    string rootPath = WebDAV::getRootPath(false);
    string parentRootPath =  rootPath.substr(0, rootPath.substr(0, rootPath.length() - 1).find_last_of("/") + 1);
    if (items.at(0).path.compare(parentRootPath) == 0)
    {
        items.erase(items.begin());
        begin = items.begin();
    }
    else
    {
        begin = items.begin()+1;
    }

    sort(begin, items.end(), []( WebDAVItem &w1, WebDAVItem &w2) -> bool
    {
        if(Util::getConfig<int>("sortBy", -1) == 2)
        {
            //sort by lastmodified
            time_t t1 = mktime(&w1.lastEditDate);
            time_t t2 = mktime(&w2.lastEditDate);
            return difftime(t1,t2) > 0 ? true : false;
        }
        else
        {
            //folders first then files
            if(w1.type == Itemtype::IFILE && w2.type == Itemtype::IFOLDER)
                return false;
            else if(w1.type == Itemtype::IFOLDER && w2.type == Itemtype::IFILE)
                return true;
            else
                return w1.title < w2.title;
        }
    });

    for(auto item : items)
    {
        auto entrySize = TextRectHeight(contentRect.w, item.title.c_str(), 0);

        if (item.type == Itemtype::IFILE)
            entrySize += _entryFontHeight;

        entrySize += (item.title.find("click to go back") != std::string::npos) ? 0.5 * _entryFontHeight : 2.5 * _entryFontHeight;


        if ((pageHeight + entrySize) > contentHeight)
        {
            pageHeight = 0;
            _page++;
        }
        irect rect = iRect(_contentRect.x, _contentRect.y + pageHeight, _contentRect.w, entrySize, 0);

        _entries.emplace_back(std::unique_ptr<WebDAVViewEntry>(new WebDAVViewEntry(_page, rect, item)));

        pageHeight = pageHeight + entrySize;
    }
    draw();
}
