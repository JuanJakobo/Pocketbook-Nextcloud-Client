//------------------------------------------------------------------
// item.cpp
//
// Author:           JuanJakobo          
// Date:             04.08.2020
//  
//-------------------------------------------------------------------

#include "item.h"
#include "inkview.h"

using namespace std;

Item::Item(const string& Path): path(Path)
{
    title = path;
    if(path.back()=='/')
    {
        type=IFOLDER;
        title = title.substr(0,path.length()-1);
    }
    else
    {
        type=IFILE;
    }    
    title = title.substr(title.find_last_of("/")+1,path.length());
}