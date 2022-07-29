//------------------------------------------------------------------
// fileViewEntry.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:
//-------------------------------------------------------------------

#ifndef FILEVIEWENTRY
#define FILEVIEWENTRY

#include "listViewEntry.h"
#include "fileModel.h"

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
    FileViewEntry(int page, const irect &position, const FileItem &entry);

    /**
        * draws the FileViewEntry to the screen
        *
        * @param entryFont font for the entry itself
        * @param entryFontBold bold font for the header
        * @param fontHeight height of the font
        */
    void draw(const ifont *entryFont, const ifont *entryFontBold, int fontHeight) override;

    FileItem &get() { return _entry; };

private:
    FileItem _entry;
};
#endif
