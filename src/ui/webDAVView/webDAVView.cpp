//------------------------------------------------------------------
// webDAVView.cpp
//
// Author:           JuanJakobo
// Date:             08.09.2021
//
//-------------------------------------------------------------------

#include "webDAVView.h"

#include <algorithm>

#include "util.h"
#include "webDAV.h"
#include "webDAVModel.h"

namespace
{
constexpr auto PAGE_BEGIN{0};
constexpr auto FONT_SIZE_MANIPULATOR{2.5};
} // namespace

using std::vector;

WebDAVView::WebDAVView(const irect &p_contentRect, std::vector<WebDAVItem> &p_itemsUnfiltered, int p_page)
    : ListView(p_contentRect, p_page)
{
    auto pageHeight{PAGE_BEGIN};
    auto const contentHeight{p_contentRect.h - m_footerHeight};

    vector<WebDAVItem> items;
    std::copy_if(p_itemsUnfiltered.begin(), p_itemsUnfiltered.end(), std::back_inserter(items),
                 [](WebDAVItem p_item) { return p_item.hide != HideState::IHIDE; });

    auto const entrycount{items.size()};

    m_entries.reserve(entrycount);

    // resize item 1
    auto header{items.at(0).path};
    header = header.substr(0, header.find_last_of("/"));
    header = header.substr(0, header.find_last_of("/") + 1);
    items.at(0).path = header;
    items.at(0).title += "\nclick to go back";
    items.at(0).lastEditDate.tm_year = 2200;

    std::vector<WebDAVItem>::iterator begin;

    auto const rootPath{WebDAV::getRootPath(false)};
    auto const parentRootPath{rootPath.substr(0, rootPath.substr(0, rootPath.length() - 1).find_last_of("/") + 1)};

    if (items.at(0).path.compare(parentRootPath) == 0)
    {
        items.erase(items.begin());
        begin = items.begin();
    }
    else
    {
        begin = items.begin() + 1;
    }

    auto const sortBy{Util::getConfig<int>("sortBy", -1) == 2};

    sort(begin, items.end(), [&sortBy](WebDAVItem &p_w1, WebDAVItem &p_w2) -> bool {
        if (sortBy)
        {
            // sort by lastmodified
            auto const t1{mktime(&p_w1.lastEditDate)};
            auto const t2{mktime(&p_w2.lastEditDate)};
            return difftime(t1, t2) > 0;
        }
        else
        {
            // folders first then files
            if (p_w1.type == Itemtype::IFILE && p_w2.type == Itemtype::IFOLDER)
                return false;
            else if (p_w1.type == Itemtype::IFOLDER && p_w2.type == Itemtype::IFILE)
                return true;
            else
                return p_w1.title < p_w2.title;
        }
    });

    for (const auto item : items)
    {
        auto entrySize{TextRectHeight(p_contentRect.w, item.title.c_str(), 0)};

        if (item.type == Itemtype::IFILE)
        {
            entrySize += m_entryFontHeight;
        }

        entrySize += FONT_SIZE_MANIPULATOR * m_entryFontHeight;

        if ((pageHeight + entrySize) > contentHeight)
        {
            pageHeight = PAGE_BEGIN;
            m_page++;
        }

        auto const rect{iRect(p_contentRect.x, p_contentRect.y + pageHeight, p_contentRect.w, entrySize, ALIGN_CENTER)};

        m_entries.emplace_back(std::make_shared<WebDAVViewEntry>(m_page, rect, item));

        pageHeight = pageHeight + entrySize;
    }
    draw();
}
