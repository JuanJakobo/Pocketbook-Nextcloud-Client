//------------------------------------------------------------------
// webDAVViewEntry.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:
//-------------------------------------------------------------------

#ifndef WEBDAVVIEWENTRY
#define WEBDAVVIEWENTRY

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
    WebDAVViewEntry(int page, const irect &position, const WebDAVItem &entry);

    /**
        * draws the WebDAVViewEntry to the screen
        *
        * @param entryFont font for the entry itself
        * @param entryFontBold bold font for the header
        * @param fontHeight height of the font
        */
    void draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight) override;

    WebDAVItem *get() override { return &_entry; };

private:
    WebDAVItem _entry;
};
#endif
