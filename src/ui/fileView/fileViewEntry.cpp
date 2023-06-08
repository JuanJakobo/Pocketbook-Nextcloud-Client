//------------------------------------------------------------------
// fileViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "fileViewEntry.h"

#include <string>

#include "fileModel.h"

FileViewEntry::FileViewEntry(int p_page, const irect &p_position, const FileItem &p_entry)
    : ListViewEntry(p_page, p_position), m_entry(p_entry)
{
}

void FileViewEntry::draw([[maybe_unused]] const ifont *p_entryFont, const ifont *p_entryFontBold,
                         int p_fontHeight) const
{
    SetFont(p_entryFontBold, BLACK);
    const auto heightOfTitle{TextRectHeight(m_position.w, m_entry.name.c_str(), 0)};
    DrawTextRect(m_position.x, m_position.y, m_position.w, heightOfTitle, m_entry.name.c_str(), ALIGN_LEFT);

    DrawTextRect(m_position.x, m_position.y, m_position.w, heightOfTitle, m_entry.name.c_str(), ALIGN_LEFT);

    DrawTextRect(m_position.x, m_position.y + heightOfTitle + p_fontHeight, m_position.w, p_fontHeight,
                 m_entry.path.c_str(), ALIGN_LEFT);
    auto type{m_entry.type == Type::FFOLDER ? "Folder" : "File"};
    DrawTextRect(m_position.x, m_position.y + heightOfTitle + p_fontHeight, m_position.w, p_fontHeight, type,
                 ALIGN_RIGHT);

    drawBottomLine();
}
