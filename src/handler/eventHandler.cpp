//------------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "eventHandler.h"
#include "mainMenu.h"
#include "contextMenu.h"
#include "webDAVView.h"
#include "util.h"
#include "log.h"
#include "webDAV.h"
#include "webDAVModel.h"

#include <string>
#include <memory>

using std::string;
using std::vector;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;

EventHandler::EventHandler()
{
    //create an copy of the eventhandler to handle methods that require static functions
    _eventHandlerStatic = std::unique_ptr<EventHandler>(this);

    _loginView = nullptr;
    _webDAVView = nullptr;
    vector<WebDAVItem> fromDB;

    if (iv_access(CONFIG_PATH.c_str(), W_OK) == 0)
    {
        //menubar
        //explanation on first login?
        //TODO here mark folders that are unsynced?
        //compare both datasets, if fromDB etag is different, mark as unsycned
        string tempPath = NEXTCLOUD_ROOT_PATH + Util::accessConfig(CONFIG_PATH, Action::IReadString,"UUID");
        _currentWebDAVItems = _webDAV.getDataStructure(tempPath);
        fromDB = _sqllite.getItemsChildren(tempPath);
    }
    //TODO here or father below?
    _menu = std::unique_ptr<MainMenu>(new MainMenu("Nextcloud"));

    if(_currentWebDAVItems.empty())
    {
        //use from DB
        //this one is always required --> if does not work -> say to the user that it did not work, to sync use
        /*
           vector<Item> Nextcloud::getOfflineStructure(const string &pathUrl)
           {
           if (pathUrl.compare(NEXTCLOUD_ROOT_PATH + getUUID() + "/") == 0)
           {
           Message(ICON_ERROR, "Error", "The root structure is not available offline. Please try again to login.", 2000);
           logout();
           }
           */
        Message(ICON_ERROR, "Error", "Could not login, please try again.", 1200);
        if(fromDB.empty())
        {
            int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Could not login and there is no DB available to restore information. What would you like to do?", "Logout", "Close App", NULL);
            switch (dialogResult)
            {
                case 1:
                    {
                        _webDAV.logout();
                        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu->getContentRect()));
                        FullUpdate();
                    }
                    break;
                case 2:
                default:
                    CloseApp();
                    break;
            }
        }
    }
    else
    {
        _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu->getContentRect(), _currentWebDAVItems,1));
        _sqllite.saveItemsChildren(_currentWebDAVItems);
        FullUpdate();
    }
}

int EventHandler::eventDistributor(const int type, const int par1, const int par2)
{
    if (ISPOINTEREVENT(type))
        return EventHandler::pointerHandler(type, par1, par2);
    else if (ISKEYEVENT(type))
        return EventHandler::keyHandler(type, par1, par2);

    return 1;
}

void EventHandler::mainMenuHandlerStatic(const int index)
{
    _eventHandlerStatic->mainMenuHandler(index);
}

void EventHandler::mainMenuHandler(const int index)
{
    switch (index)
    {
        //TODO  actualize current folder
    case 101:
    {
        break;
    }
    //Logout
    case 102:
    {
        int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Do you want to delete local files?", "Yes", "No", "Cancel");
        switch (dialogResult)
        {
        case 1:
            _webDAV.logout(true);
            break;
        case 3:
            return;
        default:
            _webDAV.logout();
            break;
        }
        _webDAVView.release();
        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu->getContentRect()));
        FullUpdate();
        break;
    }
    //Info
    case 103:
    {
        Message(ICON_INFORMATION, "Information", "Version 0.73 \n For support please open a ticket at https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client/issues", 1200);
        break;
    }
    //Exit
    case 104:
        CloseApp();
        break;
    default:
        break;
    }
}

void EventHandler::contextMenuHandlerStatic(const int index)
{
    _eventHandlerStatic->contextMenuHandler(index);
}
std::unique_ptr<ContextMenu> _contextMenu;

void EventHandler::contextMenuHandler(const int index)
{
    //invert color
    switch (index)
    {
    //Open
    case 101:
    {
        if (_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
        {
            openFolder();
        }
        else
        {
            openItem();
        }

        break;
    }
    //Sync
    case 102:
    {
        startDownload();
        break;
    }
    //remove
    case 103:
    {
        OpenProgressbar(1, "Removing...", "Removing Files.", 0, NULL);
        /*
    Log::writeInfoLog("removing file " + _items.at(itemID).getPath());
    if (!_items.at(itemID).removeFile())
        return false;

    return true;
    */
        /*
        if (_nextcloud.removeItem(_tempItemID))
        {
            updatePBLibrary();
            CloseProgressbar();
            _webDAVView->reDrawCurrentEntry();
        }
        else
        */
        CloseProgressbar();
        Message(ICON_WARNING, "Warning", "Could not delete the file, please try again.", 1200);
        break;
    }
    default:
    {
        _webDAVView->invertCurrentEntryColor();
        break;
    }

        _contextMenu.reset();
    }
}

int EventHandler::pointerHandler(const int type, const int par1, const int par2)
{
    //long press to open up context menu
    if (type == EVT_POINTERLONG)
    {
        if (_webDAVView != nullptr)
        {
            _webDAVView->checkIfEntryClicked(par1, par2);
            _webDAVView->invertCurrentEntryColor();
            if (_webDAVView->getCurrentEntry().title.compare("...") != 0)
            {
                _contextMenu = std::unique_ptr<ContextMenu>(new ContextMenu());
                _contextMenu->createMenu(par2, _webDAVView->getCurrentEntry().state, EventHandler::contextMenuHandlerStatic);
            }
        }
    }
    else if (type == EVT_POINTERUP)
    {
        if (IsInRect(par1, par2, &_menu->getMenuButtonRect()) == 1)
        {
            return _menu->createMenu((_webDAVView != nullptr), EventHandler::mainMenuHandlerStatic);
        }
        else if (_webDAVView != nullptr)
        {
            if(_webDAVView->checkIfEntryClicked(par1, par2))
            {
                _webDAVView->invertCurrentEntryColor();

                if (_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
                {
                    openFolder();
                }
                else
                {
                    if (_webDAVView->getCurrentEntry().state != FileState::ICLOUD)
                    {
                        openItem();
                    }
                    else
                    {
                        startDownload();
                    }
                }
            }

            return 0;
        }
        //if loginView is shown
        else if (_loginView != nullptr)
        {
            if (_loginView->logginClicked(par1, par2) == 2)
            {
                ShowHourglassForce();

                _currentWebDAVItems = _webDAV.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword());
                if(_currentWebDAVItems.empty())
                {
                    HideHourglass();
                    Log::writeErrorLog("login failed.");
                }
                else
                {
                    _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu->getContentRect(), _currentWebDAVItems,1));
                    _loginView.reset();
                    FullUpdate();
                }
                return 0;
            }
        }
    }
    return 1;
}

void EventHandler::updatePBLibrary()
{
    if (_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
    {
        Util::updatePBLibrary(15);
    }
    else
    {
        //if (_nextcloud.getItems().at(_tempItemID).isBook())
        /*
         * is needed twice!
            if (_fileType.find("application/epub+zip") != string::npos ||
                    _fileType.find("application/pdf") != string::npos ||
                    _fileType.find("application/octet-stream") != string::npos ||
                    _fileType.find("text/plain") != string::npos ||
                    _fileType.find("text/html") != string::npos ||
                    _fileType.find("text/rtf") != string::npos ||
                    _fileType.find("application/msword") != string::npos ||
                    _fileType.find("application/x-mobipocket-ebook") != string::npos ||
                    _fileType.find("application/vnd.openxmlformats-officedocument.wordprocessingml.document") != string::npos ||
                    _fileType.find("application/x-fictionbook+xml") != string::npos)
            Util::updatePBLibrary(5);
            */
    }
}

void EventHandler::startDownload()
{
    OpenProgressbar(1, "Downloading...", "Checking network connection", 0, NULL);
    try
    {
        //_nextcloud.download(_tempItemID);
    }
    catch (const std::exception &e)
    {
        Log::writeErrorLog(e.what());
        Message(ICON_ERROR, "Error", "Something has gone wrong. Please check the logs. (/system/config/nextcloud/)", 1200);
    }
    updatePBLibrary();

    CloseProgressbar();
    _webDAVView->reDrawCurrentEntry();
}

void EventHandler::openItem()
{
    _webDAVView->invertCurrentEntryColor();
    /*
    if (_state == FileState::ICLOUD)
    {
        Message(ICON_ERROR, "File not found.", "Could not find file.", 1000);
    }
    else if(isBook())
    {

        OpenBook(_localPath.c_str(), "", 0);
    }
    else
    {
        Message(ICON_INFORMATION, "Warning", "The filetype is currently not supported.", 1200);
    }
    */
    //webDAVView->getCurrentEntry
    //_nextcloud.getItems().at(_tempItemID).open();
}

void EventHandler::openFolder()
{
    ShowHourglassForce();
    //_nextcloud.setItems(_nextcloud.getDataStructure(_tempPath));
    //TODO if folder is unsynced sync
    WebDAV test = WebDAV();
    _currentWebDAVItems =  test.getDataStructure(_webDAVView->getCurrentEntry().path);
    if(_currentWebDAVItems.empty())
    {
        Log::writeErrorLog("items empty");
        HideHourglass();
        _webDAVView->invertCurrentEntryColor();
    }
    else
    {
        Log::writeInfoLog("got new items");
        _sqllite.saveItemsChildren(_currentWebDAVItems);

        //if folder is synced, get only from DB
        //vector<WebDAVItem> fromDB = _sqllite.getItemsChildren(_tempPath);
        //get etags from DB, if etag for path is unchanged, stays the same, same for foldersjj
        FillAreaRect(&_menu->getContentRect(), WHITE);
        _webDAVView.release();
        _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu->getContentRect(), _currentWebDAVItems,1));
        //_sqllite.saveItemsChildren(_nextcloud.getItems());
        //TODO include the header (where am i currently aka)
        //_webDAVView->drawHeader(_tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));
        PartialUpdate(_menu->getContentRect().x, _menu->getContentRect().y, _menu->getContentRect().w, _menu->getContentRect().h);
    }
}

int EventHandler::keyHandler(const int type, const int par1, const int par2)
{
    if (type == EVT_KEYPRESS)
    {
        //menu button
        if (par1 == 23)
        {
            _webDAVView->firstPage();
        }
        else if (_webDAVView != nullptr)
        {
            //left button
            if (par1 == 24)
            {
                _webDAVView->prevPage();
            }
            //right button
            else if (par1 == 25)
            {
                _webDAVView->nextPage();
            }
        }
        else
        {
            return 1;
        }
        return 0;
    }

    return 1;
}


/*
void Nextcloud::getLocalFileStructure(vector<Item> &tempItems, const string &localPath)
{
    //get local files, https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
    DIR *dir;
    class dirent *ent;
    class stat st;

    dir = opendir(localPath.c_str());
    while ((ent = readdir(dir)) != NULL)
    {
        const string fileName = ent->d_name;
        const string fullFileName = localPath + fileName;

        if (fileName[0] == '.')
            continue;

        if (stat(fullFileName.c_str(), &st) == -1)
            continue;

        const bool isDirectory = (st.st_mode & S_IFDIR) != 0;

        bool found = false;
        //looks if files have been modified
        for (unsigned int i = 1; i < tempItems.size(); i++)
        {
            if (tempItems.at(i).getLocalPath().compare(fullFileName) == 0)
            {
                //do via etag and not outsync !
                if (!isDirectory)
                {
                    //check if was changed on the cloud and here and then update...
                    //if etag ist different and last modifcated changed local --> create conflict
                    //compare by etag
                    //get last modification date and compare; if is different upload this

                    std::ifstream in(fullFileName, std::ifstream::binary | std::ifstream::ate);
                    Log::writeInfoLog(tempItems.at(i).getTitle());
                    Log::writeInfoLog(std::to_string(in.tellg()));

                    Log::writeInfoLog(std::to_string(tempItems.at(i).getSize()));
                    if (in.tellg() != tempItems.at(i).getSize())
                    {
                        tempItems.at(i).setState(FileState::IOUTSYNCED);
                    }
                }
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (isDirectory)
            {
                //create new dir in cloud
                tempItems.push_back(Item(fullFileName, FileState::ILOCAL, Itemtype::IFOLDER));
                   Item::Item(const string &localPath, FileState state, Itemtype type) : _localPath(localPath), _state(state), _type(type)
                   {
                   _title = _localPath;
                   _title = _title.substr(_title.find_last_of("/") + 1, _title.length());
                   Util::decodeUrl(_title);
                   }
            }
            else
            {
                //put to coud
                tempItems.push_back(Item(fullFileName, FileState::ILOCAL, Itemtype::IFILE));
            }
        }
    }
    closedir(dir);
}



*/

