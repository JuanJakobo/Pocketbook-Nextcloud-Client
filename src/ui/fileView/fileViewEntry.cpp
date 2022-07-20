//------------------------------------------------------------------
// fileViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "fileViewEntry.h"
#include "fileModel.h"

#include <string>

FileViewEntry::FileViewEntry(int page, const irect &position, const File &entry) : ListViewEntry(page, position), _entry(entry)
{
}

void FileViewEntry::draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight)
{
    SetFont(entryFontBold, BLACK);
    int heightOfTitle = TextRectHeight(_position.w, _entry.name.c_str(), 0);
    DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.name.c_str(), ALIGN_LEFT);

    SetFont(entryFont, BLACK);

    //DrawTextRect(_position.x, _position.y + heightOfTitle, _position.w, fontHeight, _entry.name.c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, _entry.path.c_str(), ALIGN_LEFT);
    std::string type = "File";
    if(_entry.type == Type::FOLDER)
        type = "Folder";
    DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, type.c_str(), ALIGN_RIGHT);

    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}
