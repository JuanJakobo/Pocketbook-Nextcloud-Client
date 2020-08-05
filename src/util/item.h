//------------------------------------------------------------------
// item.h
//
// Author:           JuanJakobo          
// Date:             04.08.2020
// Description:      Describes an WEBDAV item
//-------------------------------------------------------------------

#ifndef ITEM
#define ITEM

#include "inkview.h"

#include <string>

using namespace std;

enum Itemtype {IFILE,IFOLDER};
class Item
{
    public:

        Item(const string& Path);

        string getPath() const {return path;};
        void setPath(const string& Path) {path=Path;};

        string getTitle() const {return title;};

        Itemtype getType() const {return type;};

    private:
        string  path;
        Itemtype type;   
        string  title;
};
#endif