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
    if(_entry.title.find("click to go back") != std::string::npos)
        DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.title.c_str(), ALIGN_CENTER);
    else
    {
        DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.title.c_str(), ALIGN_LEFT);

        SetFont(entryFont, BLACK);


        if (_entry.state == FileState::ILOCAL)
        {
            DrawTextRect(_position.x, _position.y + heightOfTitle + 2 * fontHeight, _position.w, fontHeight, "Local", ALIGN_RIGHT);
        }
        else
        {
            if (_entry.type == IFILE)
            {
                DrawTextRect(_position.x, _position.y + heightOfTitle, _position.w, fontHeight, _entry.fileType.c_str(), ALIGN_LEFT);

                switch(_entry.state)
                {
                    case FileState::ISYNCED:
                        DrawTextRect(_position.x, _position.y + heightOfTitle + 2 * fontHeight, _position.w, fontHeight, "Synced", ALIGN_RIGHT);
                        break;
                    case FileState::IOUTSYNCED:
                        DrawTextRect(_position.x, _position.y + heightOfTitle + 2 * fontHeight, _position.w, fontHeight, "Out of sync", ALIGN_RIGHT);
                        break;
                    default:
                        DrawTextRect(_position.x, _position.y + heightOfTitle + 2 * fontHeight, _position.w, fontHeight, "Click to Download", ALIGN_RIGHT);
                }

                DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, _entry.lastEditDate.c_str(), ALIGN_LEFT);
                DrawTextRect(_position.x, _position.y + heightOfTitle + 2 * fontHeight, _position.w, fontHeight, _entry.size.c_str(), ALIGN_LEFT);
            }
            else
            {
                switch(_entry.state)
                {
                    case FileState::ISYNCED:
                        DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, "Structure synced", ALIGN_RIGHT);
                        break;
                    case FileState::IOUTSYNCED:
                        DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, "Structure out of sync", ALIGN_RIGHT);
                        break;
                    case FileState::ICLOUD:
                        DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, "Cloud", ALIGN_RIGHT);
                        break;
                    default:
                        break;
                }
                DrawTextRect(_position.x, _position.y + heightOfTitle, _position.w, fontHeight, _entry.lastEditDate.c_str(), ALIGN_LEFT);
                DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight, _position.w, fontHeight, _entry.size.c_str(), ALIGN_LEFT);
            }

        }

    }
    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}
