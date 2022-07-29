//------------------------------------------------------------------
// fileView.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "fileView.h"
#include "fileViewEntry.h"
#include "fileModel.h"

#include <string>
#include <vector>

using std::vector;

FileView::FileView(const irect &contentRect, const vector<FileItem> &files, int page) : ListView(contentRect, page)
{
    auto pageHeight = 0;
    auto contentHeight = _contentRect.h - _footerHeight;
    auto entrycount = files.size();

    _entries.reserve(entrycount);

    auto i = 0;
    while (i < entrycount)
    {
        auto entrySize = TextRectHeight(contentRect.w, files.at(i).name.c_str(), 0) + 2.5 * _entryFontHeight;
        if ((pageHeight + entrySize) > contentHeight)
        {
            pageHeight = 0;
            _page++;
        }
        irect rect = iRect(_contentRect.x, _contentRect.y + pageHeight, _contentRect.w, entrySize, 0);

        _entries.emplace_back(std::unique_ptr<FileViewEntry>(new FileViewEntry(_page, rect, files.at(i))));

        i++;
        pageHeight = pageHeight + entrySize;
    }
    draw();
}

