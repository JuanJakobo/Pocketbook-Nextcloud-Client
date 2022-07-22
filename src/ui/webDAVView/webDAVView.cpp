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

#include <string>
#include <vector>

using std::vector;

WebDAVView::WebDAVView(const irect &contentRect, vector<WebDAVItem> &items, int page) : ListView(contentRect, page)
{
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
    items.at(0).lastEditDate = "";
    if (items.at(0).path.compare(NEXTCLOUD_ROOT_PATH) == 0)
        items.erase(items.begin());

    for(auto item : items)
    {
        auto entrySize = TextRectHeight(contentRect.w, item.title.c_str(), 0);

        if(item.type == IFILE)
            entrySize += _entryFontHeight;

        if(item.title.find("click to go back") != std::string::npos)
            entrySize += 0.5 * _entryFontHeight;
        else
            entrySize += 2.5 * _entryFontHeight;


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

