//------------------------------------------------------------------
// WebDAVViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "webDAVViewEntry.h"
#include "webDAVModel.h"

#include <string>

WebDAVViewEntry::WebDAVViewEntry(int page, const irect &position, const WebDAVItem &entry) : ListViewEntry(page, position), _entry(entry)
{
}

void WebDAVViewEntry::draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight)
{
    SetFont(entryFontBold, BLACK);
    int heightOfTitle = TextRectHeight(_position.w, _entry.title.c_str(), 0);
    DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.title.c_str(), ALIGN_LEFT);

    SetFont(entryFont, BLACK);

    //DrawTextRect(_position.x, _position.y + heightOfTitle, _position.w, fontHeight, _entry.name.c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, _entry.path.c_str(), ALIGN_LEFT);
    std::string type = "File";
    //if(_entry.type == Itemtype::FOLDER)
        //type = "Folder";
    DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, type.c_str(), ALIGN_RIGHT);

    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}
