//-----------------------------------------------------------------
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
#include "fileBrowser.h"
#include "fileView.h"
#include "fileModel.h"

#include <experimental/filesystem>
#include <string>
#include <memory>
#include <algorithm>

using std::string;
using std::vector;

namespace fs = std::experimental::filesystem;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;

EventHandler::EventHandler()
{
    //create an copy of the eventhandler to handle methods that require static functions
    _eventHandlerStatic = std::unique_ptr<EventHandler>(this);

    _menu = std::unique_ptr<MainMenu>(new MainMenu("Nextcloud"));
    if (iv_access(CONFIG_PATH.c_str(), W_OK) == 0)
    {
        //for backwards compatibilty
        if (Util::accessConfig<string>(Action::IReadString, "storageLocation",{}).compare("error") == 0)
                Util::accessConfig<string>(Action::IWriteString, "storageLocation", "/mnt/ext1/nextcloud");

        if (iv_access(Util::accessConfig<string>(Action::IReadString, "storageLocation",{}).c_str(), W_OK) != 0)
            iv_mkdir(Util::accessConfig<string>(Action::IReadString, "storageLocation",{}).c_str(), 0777);

        std::vector<WebDAVItem> currentWebDAVItems;
        string path = NEXTCLOUD_ROOT_PATH + Util::accessConfig<string>(Action::IReadString,"UUID",{}) + '/';

        currentWebDAVItems = _webDAV.getDataStructure(path);
        _menu = std::unique_ptr<MainMenu>(new MainMenu("Nextcloud"));

        if (currentWebDAVItems.empty())
            currentWebDAVItems = _sqllite.getItemsChildren(path);
        else
            updateItems(currentWebDAVItems);

        if (currentWebDAVItems.empty())
        {
            int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Could not login and there is no DB available to restore information. What would you like to do?", "Logout", "Close App", NULL);
            switch (dialogResult)
            {
                case 1:
                    {
                        _webDAV.logout(true);
                        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu->getContentRect()));
                    }
                    break;
                case 2:
                default:
                    CloseApp();
                    break;
            }
        }
        else
        {
            drawWebDAVItems(currentWebDAVItems);
        }
    }
    else
    {
        _menu = std::unique_ptr<MainMenu>(new MainMenu("Nextcloud"));
        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu->getContentRect()));
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
        //Actualize the current folder
        case 101:
            {
                OpenProgressbar(1, "Actualizing current folder", ("Actualizing path" + _currentPath).c_str(), 0, NULL);
                string childrenPath = _currentPath;
                childrenPath = childrenPath.substr(NEXTCLOUD_ROOT_PATH.length(), childrenPath.length());
                std::string path = NEXTCLOUD_ROOT_PATH;
                std::vector<WebDAVItem> currentWebDAVItems;
                size_t found = 0;
                int i = 0;
                while((found = childrenPath.find("/"),found) != std::string::npos)
                {
                    path += childrenPath.substr(0, found+1);
                    childrenPath = childrenPath.substr(found+1,childrenPath.length());
                    auto state = _sqllite.getState(path);
                    Log::writeInfoLog("cur path " + path);
                    if (i < 1 || state == FileState::IOUTSYNCED || state == FileState::ICLOUD)
                    {
                        UpdateProgressbar(("Upgrading " + path).c_str(), 0);
                        currentWebDAVItems = _webDAV.getDataStructure(path);
                    }
                    else
                    {
                        break;
                    }

                    if (currentWebDAVItems.empty())
                    {
                        Log::writeErrorLog("Could not sync " + path + " via actualize.");
                        Message(ICON_WARNING, "Warning", "Could not sync the file structure.", 2000);
                        HideHourglass();
                        break;
                    }
                    else
                    {
                        updateItems(currentWebDAVItems);
                    }
                    i++;
                }

                Log::writeInfoLog("stopped at " + path );
                currentWebDAVItems = _sqllite.getItemsChildren(_currentPath);

                for(auto &item : currentWebDAVItems)
                {
                    Log::writeInfoLog(item.path);
                    if (item.type == Itemtype::IFOLDER && item.state == FileState::IOUTSYNCED)
                    {
                        UpdateProgressbar(("Upgrading " + item.path).c_str(), 0);
                        vector<WebDAVItem> tempWebDAVItems = _webDAV.getDataStructure(item.path);
                        updateItems(tempWebDAVItems);
                    }

                }
                currentWebDAVItems = _sqllite.getItemsChildren(_currentPath);

                CloseProgressbar();
                if (!currentWebDAVItems.empty())
                    drawWebDAVItems(currentWebDAVItems);
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
                _webDAVView.reset();
                _loginView = std::unique_ptr<LoginView>(new LoginView(_menu->getContentRect()));
                break;
            }
            //Sort by
        case 103:
            {
                int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "By what do you want to sort?", "Filename", "Last modified", "Cancel");
                switch (dialogResult)
                {
                    case 1:
                        Util::accessConfig<int>(Action::IWriteInt, "sortBy", 1);
                        break;
                    case 2:
                        Util::accessConfig<int>(Action::IWriteInt, "sortBy", 2);
                        break;
                    default:
                        return;
                }
                Message(ICON_INFORMATION, "Info", "Reload page to see new order method in effect.", 2000);
                break;
            }
            //Select folder
        case 104:
            {

                _currentPath = _currentPath + ((_currentPath.back() != '/') ? "/nextcloud" : "nextcloud");

                if (iv_mkdir(_currentPath.c_str(), 0777) != 0)
                {
                    Log::writeErrorLog("choosen part " + _currentPath + " could not be created as permission are not sufficient.");
                    Message(ICON_ERROR, "Error", "The permissions are not sufficient.", 1000);
                }
                else
                {
                    Util::accessConfig<string>(Action::IWriteString, "storageLocation", _currentPath);
                    std::vector<WebDAVItem> currentWebDAVItems = _webDAV.getDataStructure(NEXTCLOUD_ROOT_PATH + Util::accessConfig<string>(Action::IReadString,"UUID",{}) + '/');
                    if (currentWebDAVItems.empty())
                    {
                        Message(ICON_ERROR, "Error", "Failed to get items. Please try again.", 1000);
                    }
                    else
                    {
                        _fileView.reset();
                        updateItems(currentWebDAVItems);
                        drawWebDAVItems(currentWebDAVItems);
                    }
                }
                break;
            }
            //Info
        case 105:
            {
                Message(ICON_INFORMATION, "Info", "Version 1.01 \n For support please open a ticket at https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client/issues", 1200);
                break;
            }
            //Exit
        case 106:
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

void EventHandler::contextMenuHandler(const int index)
{
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
        if (_webDAVView->getCurrentEntry().state != FileState::ILOCAL)
        {
            startDownload();
        }
        else
        {
            //TODO upload to cloud
            //Dialog file is offline, do you want to sync it to the cloud?
            Message(ICON_ERROR, "Error", "The File is local and upload to cloud is currently not supported.", 2000);
            _webDAVView->invertCurrentEntryColor();
        }

        break;
    }
    //remove
    case 103:
    {
        if (_webDAVView->getCurrentEntry().state != FileState::ICLOUD)
        {
            if (_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
            {
                fs::remove_all(_webDAVView->getCurrentEntry().localPath);
            }
            else
            {
                fs::remove(_webDAVView->getCurrentEntry().localPath);
            }
            vector<WebDAVItem> currentWebDAVItems = _sqllite.getItemsChildren(_currentPath);
            updateItems(currentWebDAVItems);
            drawWebDAVItems(currentWebDAVItems);
        }
        else
        {
            Message(ICON_ERROR, "Error", "File is not available locally.", 1000);
            _webDAVView->invertCurrentEntryColor();
        }
        break;
    }
    default:
    {
        _webDAVView->invertCurrentEntryColor();
        break;
    }
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
                _contextMenu.createMenu(par2, _webDAVView->getCurrentEntry().state, EventHandler::contextMenuHandlerStatic);
            }
        }
    }
    else if (type == EVT_POINTERUP)
    {
        if (IsInRect(par1, par2, &_menu->getMenuButtonRect()) == 1)
        {
            return _menu->createMenu((_fileView != nullptr), (_webDAVView != nullptr), EventHandler::mainMenuHandlerStatic);
        }
        else if (_webDAVView != nullptr)
        {
            if (_webDAVView->checkIfEntryClicked(par1, par2))
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

                std::vector<WebDAVItem> currentWebDAVItems = _webDAV.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword(), _loginView->getIgnoreCert());;
                if (currentWebDAVItems.empty())
                {
                    Message(ICON_ERROR, "Error", "Login failed.", 1000);
                    HideHourglass();
                }
                else
                {
                    int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Do you want to choose your own storage path or use the default one. \n (/mnt/ext1/nextcloud/)", "Choose my own path", "Choose standard path", NULL);
                    switch (dialogResult)
                    {
                        case 1:
                            {
                                auto path = "/mnt/ext1";
                                vector<FileItem> currentFolder = FileBrowser::getFileStructure(path,false,true);
                                _currentPath = path;
                                _loginView.reset();
                                FillAreaRect(&_menu->getContentRect(), WHITE);
                                _fileView = std::unique_ptr<FileView>(new FileView(_menu->getContentRect(), currentFolder,1));
                            }
                            break;
                        case 2:
                        default:
                            if (iv_access(Util::accessConfig<string>(Action::IReadString, "storageLocation",{}).c_str(), W_OK) != 0)
                                iv_mkdir(Util::accessConfig<string>(Action::IReadString, "storageLocation",{}).c_str(), 0777);
                            updateItems(currentWebDAVItems);
                            drawWebDAVItems(currentWebDAVItems);
                            break;
                    }
                }
                return 0;
            }
        }
        else if (_fileView != nullptr)
        {
            if (_fileView->checkIfEntryClicked(par1, par2))
            {
                _fileView->invertCurrentEntryColor();

                if (_fileView->getCurrentEntry().type == Type::FFOLDER)
                {
                    _currentPath = _fileView->getCurrentEntry().path;
                    vector<FileItem> currentFolder = FileBrowser::getFileStructure(_currentPath,false,true);
                    _fileView.reset(new FileView(_menu->getContentRect(), currentFolder,1));
                }
            }

            return 0;
        }

    }
    return 1;
}

void EventHandler::openItem()
{
    _webDAVView->invertCurrentEntryColor();
    if (_webDAVView->getCurrentEntry().state == FileState::ICLOUD)
    {
        Message(ICON_ERROR, "Error", "Could not find file.", 1000);
    }
    else if (_webDAVView->getCurrentEntry().fileType.find("application/epub+zip") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("application/pdf") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("application/octet-stream") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("text/plain") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("text/html") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("text/rtf") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("text/markdown") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("application/msword") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("application/x-mobipocket-ebook") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("application/vnd.openxmlformats-officedocument.wordprocessingml.document") != string::npos ||
                    _webDAVView->getCurrentEntry().fileType.find("application/x-fictionbook+xml") != string::npos)
    {
        OpenBook(_webDAVView->getCurrentEntry().localPath.c_str(), "", 0);
    }
    else
    {
        Message(ICON_INFORMATION, "Info", "The filetype is currently not supported.", 1200);
    }
}

void EventHandler::openFolder()
{
    ShowHourglassForce();

    std::vector<WebDAVItem> currentWebDAVItems;

    switch ((_webDAVView->getCurrentEntry().state == FileState::ILOCAL) ? FileState::ILOCAL : _sqllite.getState(_webDAVView->getCurrentEntry().path))
    {
        case FileState::ILOCAL:
            {
                Message(ICON_ERROR, "Error", "Not implemented to look at local folder.", 2000);
                _webDAVView->invertCurrentEntryColor();
                //TODO use FileBrowser
                //_webDAVView.reset();
                //FileBrowser fB = FileBrowser(true);
                //_fileView.reset(new FileView(_menu->getContentRect(),fB.getFileStructure(_webDAVView->getCurrentEntry().path),1));
                break;
            }
        case FileState::IOUTSYNCED:
        case FileState::ICLOUD:
            currentWebDAVItems = _webDAV.getDataStructure(_webDAVView->getCurrentEntry().path);
        case FileState::ISYNCED:
        case FileState::IDOWNLOADED:
            {
                if (!currentWebDAVItems.empty())
                    updateItems(currentWebDAVItems);
                else if (_webDAVView->getCurrentEntry().state != FileState::ICLOUD)
                    currentWebDAVItems = _sqllite.getItemsChildren(_webDAVView->getCurrentEntry().path);

                if (currentWebDAVItems.empty())
                {
                    Message(ICON_ERROR, "Error", "Could not sync the items and there is no offline copy available.", 2000);
                    HideHourglass();
                    _webDAVView->invertCurrentEntryColor();
                }
                else
                {
                    drawWebDAVItems(currentWebDAVItems);
                }
                break;
            }
    }
}

int EventHandler::keyHandler(const int type, const int par1, const int par2)
{
    if (_webDAVView != nullptr)
    {
        if (type == EVT_KEYPRESS)
        {
            switch(par1)
            {
                //menu button
                case 23:
                    _webDAVView->firstPage();
                    break;
                    //left button
                case 24:
                    _webDAVView->prevPage();
                    break;
                    //right button
                case 25:
                    _webDAVView->nextPage();
                    break;
                default:
                    return 1;
            }
            return 0;
        }
    }
    else if (_fileView != nullptr)
    {
        if (type == EVT_KEYPRESS)
        {
            switch(par1)
            {
                //menu button
                case 23:
                    _fileView->firstPage();
                    break;
                    //left button
                case 24:
                    _fileView->prevPage();
                    break;
                    //right button
                case 25:
                    _fileView->nextPage();
                    break;
                default:
                    return 1;
            }
            return 0;
        }
    }

    return 1;
}

void EventHandler::getLocalFileStructure(std::vector<WebDAVItem> &tempItems)
{
    string localPath = tempItems.at(0).localPath + '/';
    if (iv_access(localPath.c_str(), W_OK) == 0)
    {
        vector<FileItem> currentFolder = FileBrowser::getFileStructure(localPath,true,false);

        for(const FileItem &local : currentFolder)
        {
            auto p = find_if(tempItems.begin()+1, tempItems.end(), [&] (const WebDAVItem &item) {return item.localPath.compare(local.path) == 0;});
            if (p == tempItems.end())
            {
                WebDAVItem temp;
                temp.localPath = local.path;
                temp.state = FileState::ILOCAL;
                temp.title = temp.localPath.substr(temp.localPath.find_last_of('/') + 1, temp.localPath.length());
                //Log::writeInfoLog(std::to_string(fs::file_size(entry)));
                temp.lastEditDate = local.lastEditDate;
                if(local.type == Type::FFOLDER)
                {
                    //create new dir in cloud
                    temp.type = Itemtype::IFOLDER;
                }
                else
                {
                    //put to cloud
                    temp.fileType = "File";
                    temp.type = Itemtype::IFILE;
                }
                tempItems.push_back(temp);
            }

        }
    }
}

void EventHandler::downloadFolder(vector<WebDAVItem> &items, int itemID)
{
    //BanSleep(2000);
    string path = items.at(itemID).path;

    if (items.at(itemID).type == Itemtype::IFOLDER)
    {
        vector<WebDAVItem> tempItems;
        switch(items.at(itemID).state)
        {
            case FileState::IOUTSYNCED:
            case FileState::ICLOUD:
                {
                    UpdateProgressbar(("Syncing folder" + path).c_str(), 0);
                    tempItems = _webDAV.getDataStructure(path);
                    items.at(itemID).state = FileState::IDOWNLOADED;
                    _sqllite.updateState(items.at(itemID).path,items.at(itemID).state);
                    updateItems(tempItems);
                    break;
                }
            case FileState::ISYNCED:
                {
                    tempItems = _sqllite.getItemsChildren(path);
                    items.at(itemID).state = FileState::IDOWNLOADED;
                    _sqllite.updateState(items.at(itemID).path,items.at(itemID).state);
                    break;
                }
            case FileState::ILOCAL:
                {
                    if(items.at(itemID).localPath.length() > 3 && items.at(itemID).localPath.substr(items.at(itemID).localPath.length() - 3).compare("sdr") == 0)
                        Log::writeInfoLog("Ignoring koreader file " + items.at(itemID).localPath);
                    else
                    {
                        CloseProgressbar();

                        int dialogResult = DialogSynchro(ICON_QUESTION, "Action", ("The folder " + items.at(itemID).localPath + " has been removed from the cloud. Do you want to delete it?").c_str(), "Yes", "No", "Cancel");
                        if(dialogResult == 1)
                            fs::remove_all(items.at(itemID).localPath);
                        else
                            tempItems.push_back(items.at(itemID));

                        OpenProgressbar(1, "Downloading...", "", 0, NULL);
                    }
                    break;
                }
            default:
                break;
        }

        if(!tempItems.empty())
        {
            getLocalFileStructure(tempItems);
            //first item of the vector is the root path itself
            for (size_t i = 1; i < tempItems.size(); i++)
                downloadFolder(tempItems, i);
        }

        //compare if file is in DB
        //if file is not in DB, upload
        //if file is in DB but no longer in Cloud, delete (if file local is newer prompt?)
        //if file is newer offline, upload
        //delete in DB if folder is no longer synced
        //TODO remove file parts that are no longer there, check for local path and delete these
    }
    else
    {
        switch(items.at(itemID).state)
        {
            case FileState::IOUTSYNCED:
            case FileState::ICLOUD:
                {
                    //TODO both direction
                    //1. check etag --> if is differnt, cloud has been updated
                    //2. check modification date and file size locally --> if is different, local has been updated
                    //3. if both --> create conflict
                    //4. if first, renew file --> reset etag
                    //5. if second --> upload the local file; test if it has not been update in the cloud
                    Log::writeInfoLog("started download of " + items.at(itemID).path + " to " + items.at(itemID).localPath);
                    if (_webDAV.get(items.at(itemID))) {
                        items.at(itemID).state = FileState::ISYNCED;
                        _sqllite.updateState(items.at(itemID).path,items.at(itemID).state);
                    }
                    break;
                }
            case FileState::ILOCAL:
                {
                    CloseProgressbar();
                    int dialogResult = DialogSynchro(ICON_QUESTION, "Action", ("The file " + items.at(itemID).localPath + " has been removed from the cloud. Do you want to delete it?").c_str(), "Yes", "No", "Cancel");
                    if(dialogResult == 1)
                        fs::remove(items.at(itemID).localPath);
                    break;
                    OpenProgressbar(1, "Downloading...", "", 0, NULL);
                }
            default:
                break;
        }
    }


    return;
}

void EventHandler::startDownload()
{
    OpenProgressbar(1, "Downloading...", "Starting Download.", 0, NULL);

    if (_webDAVView->getCurrentEntry().type == Itemtype::IFILE)
    {
        Log::writeInfoLog("Started download of " + _webDAVView->getCurrentEntry().path + " to " + _webDAVView->getCurrentEntry().localPath);
        if (_webDAV.get(_webDAVView->getCurrentEntry()))
        {
            _webDAVView->getCurrentEntry().state = FileState::ISYNCED;
            _sqllite.updateState(_webDAVView->getCurrentEntry().path,_webDAVView->getCurrentEntry().state);
        }
    }
    else
    {
        vector<WebDAVItem> currentItems = _sqllite.getItemsChildren(_webDAVView->getCurrentEntry().path);
        this->downloadFolder(currentItems, 0);
        _webDAVView->getCurrentEntry().state = FileState::IDOWNLOADED;
        _sqllite.updateState(_webDAVView->getCurrentEntry().path,_webDAVView->getCurrentEntry().state);
        UpdateProgressbar("Download completed", 100);
    }

    //TODO implement
    //Util::updatePBLibrary(15);
    CloseProgressbar();
    _webDAVView->reDrawCurrentEntry();
}

bool EventHandler::checkIfIsDownloaded(vector<WebDAVItem> &items, int itemID)
{
    if (iv_access(items.at(itemID).localPath.c_str(), W_OK) != 0)
    {
        items.at(itemID).state = FileState::IOUTSYNCED;
        _sqllite.updateState(items.at(itemID).path,items.at(itemID).state);
        return false;
    }

    if (items.at(itemID).type == Itemtype::IFOLDER)
    {
        vector<WebDAVItem> tempItems = _sqllite.getItemsChildren(items.at(itemID).path);
        //first item of the vector is the root path itself
        for (size_t i = 1; i < tempItems.size(); i++)
        {
            if(!checkIfIsDownloaded(tempItems, i))
            {
                items.at(itemID).state = FileState::IOUTSYNCED;
                _sqllite.updateState(items.at(itemID).path,items.at(itemID).state);
                return false;
            }
        }
    }
    return true;
}

void EventHandler::updateItems(vector<WebDAVItem> &items)
{
    for(auto &item : items)
    {
        //returns ICloud if is not found
        item.state = _sqllite.getState(item.path);

        if (item.type == Itemtype::IFILE)
        {
            if (iv_access(item.localPath.c_str(), W_OK) != 0)
                item.state = FileState::ICLOUD;
            else
                item.state = FileState::ISYNCED;
        }
        else
        {
            if(item.state == FileState::IDOWNLOADED)
            {
                vector<WebDAVItem> currentItems = _sqllite.getItemsChildren(item.path);
                if(!checkIfIsDownloaded(currentItems,0))
                    item.state = FileState::IOUTSYNCED;
            }

            if (iv_access(item.localPath.c_str(), W_OK) != 0)
                iv_mkdir(item.localPath.c_str(), 0777);
        }
        if (_sqllite.getEtag(item.path).compare(item.etag) != 0)
            item.state = (item.state == FileState::ISYNCED || item.state == FileState::IDOWNLOADED) ? FileState::IOUTSYNCED : FileState::ICLOUD;
    }
    if(items.at(0).state != FileState::IDOWNLOADED)
    {
        items.at(0).state = FileState::ISYNCED;
    }
    _sqllite.saveItemsChildren(items);

    //TODO sync delete when not parentPath existend --> "select * from metadata where parentPath NOT IN (Select
    //DISTINCT(parentPath) from metadata;
    //what happens with the entries below?
}


void EventHandler::drawWebDAVItems(vector<WebDAVItem> &items)
{
    _currentPath = items.at(0).path;
    getLocalFileStructure(items);
    _webDAVView.reset(new WebDAVView(_menu->getContentRect(), items,1));
}
