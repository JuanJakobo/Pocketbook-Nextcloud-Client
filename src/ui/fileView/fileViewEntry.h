#pragma once
//------------------------------------------------------------------
// fileViewEntry.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:
//-------------------------------------------------------------------
#include "fileModel.h"
#include "listViewEntry.h"

class FileViewEntry : public ListViewEntry
{
  public:
    /**
     * Creates an FileViewEntry
     *
     * @param Page site of the listView the Entry is shown
     * @param Rect area of the screen the item is positioned
     * @param entry entry that shall be drawn
     */
    FileViewEntry(int p_page, const irect &p_position, const FileItem &p_entry);

    /**
     * draws the FileViewEntry to the screen
     *
     * @param entryFont font for the entry itself
     * @param entryFontBold bold font for the header
     * @param fontHeight height of the font
     */
    void draw(const ifont *p_entryFont, const ifont *p_entryFontBold, int p_fontHeight) const override;

    const FileItem &get() const
    {
        return m_entry;
    };

  private:
    FileItem m_entry;
};
