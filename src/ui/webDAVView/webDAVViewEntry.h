#pragma once
//------------------------------------------------------------------
// webDAVViewEntry.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:
//-------------------------------------------------------------------
#include "listViewEntry.h"
#include "webDAVModel.h"

class WebDAVViewEntry : public ListViewEntry
{
  public:
    /**
     * Creates an WebDAVViewEntry
     *
     * @param Page site of the listView the Entry is shown
     * @param Rect area of the screen the item is positioned
     * @param entry entry that shall be drawn
     */
    WebDAVViewEntry(int p_page, const irect &p_position, const WebDAVItem &p_entry);

    /**
     * draws the WebDAVViewEntry to the screen
     *
     * @param entryFont font for the entry itself
     * @param entryFontBold bold font for the header
     * @param fontHeight height of the font
     */
    void draw(const ifont *p_entryFont, const ifont *p_entryFontBold, int p_fontHeight) const override;

    WebDAVItem &get()
    {
        return m_entry;
    };

  private:
    WebDAVItem m_entry;
};
