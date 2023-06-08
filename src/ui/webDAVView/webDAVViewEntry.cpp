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

WebDAVViewEntry::WebDAVViewEntry(int p_page, const irect &p_position, const WebDAVItem &p_entry)
    : ListViewEntry(p_page, p_position), m_entry(p_entry)
{
}

void WebDAVViewEntry::draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight) const
{
    SetFont(entryFontBold, BLACK);
    const auto heightOfTitle{TextRectHeight(m_position.w, m_entry.title.c_str(), 0)};
    DrawTextRect(m_position.x, m_position.y, m_position.w, heightOfTitle, m_entry.title.c_str(), ALIGN_LEFT);
    SetFont(entryFont, BLACK);

    {
        auto height{0};
        std::string text = {};
        if (m_entry.type == Itemtype::IFILE)
        {
            height = fontHeight;
            if (m_entry.state != FileState::ILOCAL)
            {
                DrawTextRect(m_position.x, m_position.y + heightOfTitle + height, m_position.w, fontHeight,
                             m_entry.fileType.c_str(), ALIGN_LEFT);
            }

            switch (m_entry.state)
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
            switch (m_entry.state)
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
        DrawTextRect(m_position.x, m_position.y + heightOfTitle + fontHeight + height, m_position.w, fontHeight,
                     text.c_str(), ALIGN_RIGHT);
        if (m_entry.state != FileState::ILOCAL)
        {
            DrawTextRect(m_position.x, m_position.y + heightOfTitle + fontHeight + height, m_position.w, fontHeight,
                         m_entry.size.c_str(), ALIGN_LEFT);
        }

        time_t now;
        time(&now);
        auto lastEditDate{m_entry.lastEditDate};
        auto seconds = difftime(now, mktime(&lastEditDate) - timezone);
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
                                time = "at " + Util::webDAVTmToString(m_entry.lastEditDate);
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
        DrawTextRect(m_position.x, m_position.y + heightOfTitle, m_position.w, fontHeight, time.c_str(), ALIGN_LEFT);
    }
    drawBottomLine();
}
