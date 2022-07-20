//------------------------------------------------------------------
// webDAVView.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "webDAVView.h"
#include "webDAVModel.h"

#include <string>
#include <vector>

using std::vector;

WebDAVView::WebDAVView(const irect &contentRect, const vector<WebDAVItem> &items, int page) : ListView(contentRect, page)
{
    auto pageHeight = 0;
    auto contentHeight = _contentRect.h - _footerHeight;
    auto entrycount = items.size();

    _entries.reserve(entrycount);

    auto i = 0;
    while (i < entrycount)
    {
        auto entrySize = TextRectHeight(contentRect.w, items.at(i).title.c_str(), 0);

        if(items.at(i).type == IFILE)
            entrySize += _entryFontHeight;

        if(items.at(i).title.find("click to go back") != std::string::npos)
            entrySize += 0.5 * _entryFontHeight;
        else
            entrySize += 2.5 * _entryFontHeight;


        if ((pageHeight + entrySize) > contentHeight)
        {
            pageHeight = 0;
            _page++;
        }
        irect rect = iRect(_contentRect.x, _contentRect.y + pageHeight, _contentRect.w, entrySize, 0);

        _entries.emplace_back(std::unique_ptr<WebDAVViewEntry>(new WebDAVViewEntry(_page, rect, items.at(i))));

        i++;
        pageHeight = pageHeight + entrySize;
    }
    draw();
}

