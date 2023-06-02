//------------------------------------------------------------------
// fileView.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "fileView.h"

#include <vector>

#include "fileModel.h"
#include "fileViewEntry.h"

namespace
{
constexpr auto PAGE_BEGIN{0};
constexpr auto FONT_SIZE_MANIPULATOR{2.5};
} // namespace

FileView::FileView(const irect &p_contentRect, const std::vector<FileItem> &p_files, uint8_t p_page)
    : ListView(p_contentRect, p_page)
{
    auto pageHeight{PAGE_BEGIN};
    auto const contentHeight{p_contentRect.h - m_footerHeight};
    auto const entrycount{p_files.size()};

    m_entries.reserve(entrycount);

    for (const auto file : p_files)
    {
        auto const entrySize{TextRectHeight(p_contentRect.w, file.name.c_str(), 0) +
                             FONT_SIZE_MANIPULATOR * m_entryFontHeight};
        if ((pageHeight + entrySize) > contentHeight)
        {
            pageHeight = PAGE_BEGIN;
            m_page++;
        }
        auto const rect =
            iRect(p_contentRect.x, p_contentRect.y + pageHeight, p_contentRect.w, entrySize, ALIGN_CENTER);

        m_entries.emplace_back(std::make_shared<FileViewEntry>(m_page, rect, file));

        pageHeight = pageHeight + entrySize;
    }
    draw();
}
