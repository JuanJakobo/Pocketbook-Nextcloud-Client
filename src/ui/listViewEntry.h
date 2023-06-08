#pragma once
//------------------------------------------------------------------
// listViewEntry.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An listViewEntry that handles an item of a listview
//-------------------------------------------------------------------
#include "inkview.h"
#include "model.h"

class ListViewEntry
{
  public:
    /**
     * Creates an ListViewEntry
     *
     * @param Page site of the listView the Entry is shown
     * @param Rect area of the screen the item is positioned
     */
    ListViewEntry(int p_page, const irect &p_position);

    virtual ~ListViewEntry(){};

    const irect &getPosition() const
    {
        return m_position;
    }
    int getPage() const
    {
        return m_page;
    }

    /**
     * draws the listViewEntry to the screen
     *
     * @param entryFont font for the entry itself
     * @param entryFontBold bold font for the header
     * @param fontHeight height of the font
     */
    virtual void draw(const ifont *p_entryFont, const ifont *p_entryFontBold, int p_fontHeight) const = 0;

  protected:
    int m_page;
    irect m_position;

    void drawBottomLine() const;
};
