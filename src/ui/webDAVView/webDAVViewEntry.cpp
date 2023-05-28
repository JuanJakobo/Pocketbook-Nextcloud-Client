//------------------------------------------------------------------
// WebDAVViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "webDAVViewEntry.h"

#include <time.h>

#include <string>

#include "util.h"
#include "webDAVModel.h"

WebDAVViewEntry::WebDAVViewEntry(int page, const irect &position, const WebDAVItem &entry)
    : ListViewEntry(page, position), _entry(entry)
{
}

void WebDAVViewEntry::draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight)
{
    SetFont(entryFontBold, BLACK);
    int heightOfTitle = TextRectHeight(_position.w, _entry.title.c_str(), 0);
    if (_entry.title.find("click to go back") != std::string::npos)
        DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.title.c_str(), ALIGN_CENTER);
    else
    {
        DrawTextRect(_position.x, _position.y, _position.w, heightOfTitle, _entry.title.c_str(), ALIGN_LEFT);
        SetFont(entryFont, BLACK);

        {
            int height = 0;
            std::string text = {};
            if (_entry.type == Itemtype::IFILE)
            {
                height = fontHeight;
                if (_entry.state != FileState::ILOCAL)
                    DrawTextRect(_position.x, _position.y + heightOfTitle + height, _position.w, fontHeight,
                                 _entry.fileType.c_str(), ALIGN_LEFT);

                switch (_entry.state)
                {
                case FileState::ISYNCED:
                    text = "Downloaded";
                    break;
                case FileState::IOUTSYNCED:
                    text = "Out of sync";
                    break;
                case FileState::ILOCAL:
                    text = "Local";
                    break;
                default:
                    text = "Click to download";
                }
            }
            else
            {
                switch (_entry.state)
                {
                case FileState::ISYNCED:
                    text = "Structure synced";
                    break;
                case FileState::IOUTSYNCED:
                    text = "Structure of out sync";
                    break;
                case FileState::IDOWNLOADED:
                    text = "Downloaded";
                    break;
                case FileState::ICLOUD:
                    text = "Cloud";
                    break;
                case FileState::ILOCAL:
                    text = "Local";
                }
            }
            DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight + height, _position.w, fontHeight,
                         text.c_str(), ALIGN_RIGHT);
            if (_entry.state != FileState::ILOCAL)
                DrawTextRect(_position.x, _position.y + heightOfTitle + fontHeight + height, _position.w, fontHeight,
                             _entry.size.c_str(), ALIGN_LEFT);

            time_t now;
            time(&now);
            auto seconds = difftime(now, mktime(&_entry.lastEditDate) - timezone);
            int sec;

            std::string time;
            // minutes
            if (seconds > 60)
            {
                seconds = seconds / 60;
                sec = seconds;
                time = std::to_string(sec) + " minutes ago";
                // hours
                if (seconds > 60)
                {
                    seconds = seconds / 60;
                    sec = seconds;
                    time = std::to_string(sec) + " hours ago";
                    // days
                    if (seconds > 24)
                    {
                        seconds = seconds / 24;
                        sec = seconds;
                        time = std::to_string(sec) + " days ago";
                        // month
                        if (seconds > 30)
                        {
                            seconds = seconds / 30;
                            sec = seconds;
                            time = std::to_string(sec) + " months ago";
                            // years
                            if (seconds > 12)
                            {
                                seconds = seconds / 12;
                                sec = seconds;
                                time = std::to_string(sec) + " years ago";
                                if (seconds > 1)
                                {
                                    time = "at " + Util::webDAVTmToString(_entry.lastEditDate);
                                }
                            }
                        }
                    }
                }
            }
            // seconds
            else
            {
                sec = seconds;
                time = std::to_string(sec) + " seconds ago";
            }
            time = "Modified " + time;
            DrawTextRect(_position.x, _position.y + heightOfTitle, _position.w, fontHeight, time.c_str(), ALIGN_LEFT);
        }
    }
    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}
