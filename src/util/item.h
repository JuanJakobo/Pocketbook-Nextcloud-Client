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

        Item(const string& xmlItem);

        void setPath(const string& Path) {path=Path;};
        string getPath() const {return path;};

        Itemtype getType() const {return type;};

        void setTitle(const string& Title) {title=Title;};
        string getTitle() const {return title;};

        bool isDownloaded() const {return downloaded;};

        string getLastEditDate() const {return lastEdithDate;};

        string getSize() const {return size;};

        string getFiletype() const {return fileType;};

        /**
        * downloads a file from WEBDAV and saves it 
        *         
        * @return true - sucessfull, false - error
        */
        string isClicked();

    private:
        string  path;
        Itemtype type;   
        string  title;
        bool downloaded;
        string lastEdithDate;
        string size;
        string fileType;


};
#endif