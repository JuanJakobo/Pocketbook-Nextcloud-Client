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
#include "fileBrowser.h"
#include "fileView.h"
#include "fileModel.h"

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

    _menu = std::unique_ptr<MainMenu>(new MainMenu("Nextcloud"));
    if (iv_access(CONFIG_PATH.c_str(), W_OK) == 0)
    {
        //for backwards compatibilty
        if (Util::accessConfig(CONFIG_PATH, Action::IReadString, "storageLocation").empty())
                Util::accessConfig(CONFIG_PATH, Action::IWriteString, "storageLocation", "/mnt/ext1/nextcloud");

        if (iv_access(Util::accessConfig(CONFIG_PATH, Action::IReadString, "storageLocation").c_str(), W_OK) != 0)
            iv_mkdir(Util::accessConfig(CONFIG_PATH, Action::IReadString, "storageLocation").c_str(), 0777);
        std::vector<WebDAVItem> currentWebDAVItems;
        string path = NEXTCLOUD_ROOT_PATH + Util::accessConfig(CONFIG_PATH, Action::IReadString,"UUID") + '/';

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
                        Log::writeInfoLog("syncing");
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
                _webDAVView.release();
                _loginView = std::unique_ptr<LoginView>(new LoginView(_menu->getContentRect()));
                break;
            }
            //Select folder
        case 103:
            {

                _currentPath =+ (_currentPath.back() != '/') ? "/nextcloud" : "nextcloud";

                if (iv_mkdir(_currentPath.c_str(), 0777) != 0)
                    Message(ICON_ERROR, "Error", "The permissions are not sufficient.", 1000);
                else
                {
                    Util::accessConfig(CONFIG_PATH, Action::IWriteString, "storageLocation", _currentPath);
                    std::vector<WebDAVItem> currentWebDAVItems = _webDAV.getDataStructure(NEXTCLOUD_ROOT_PATH + Util::accessConfig(CONFIG_PATH, Action::IReadString,"UUID") + '/');
                    if (currentWebDAVItems.empty())
                    {
                        Message(ICON_ERROR, "Error", "Failed to get items. Please try again.", 1000);
                    }
                    else
                    {
                        updateItems(currentWebDAVItems);
                        drawWebDAVItems(currentWebDAVItems);
                    }
                }
                break;
            }
            //Info
        case 104:
            {
                Message(ICON_INFORMATION, "Info", "Version 0.98 \n For support please open a ticket at https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client/issues", 1200);
                break;
            }
            //Exit
        case 105:
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
                Message(ICON_ERROR, "Error", "Currently only files can be deleted.", 2000);
                //string cmd = "rm -rf " + _webDAVView->getCurrentEntry().localPath + '/';
                //if (rmdir((_webDAVView->getCurrentEntry().localPath + '/').c_str()) == 0)
                    //Log::writeInfoLog("okay");
                //if (system(cmd.c_str()) == 0)
                    //Log::writeInfoLog("success");
            }
            else
            {
                remove(_webDAVView->getCurrentEntry().localPath.c_str());
            }
            vector<WebDAVItem> currentWebDAVItems = _sqllite.getItemsChildren(_currentPath);
            updateItems(currentWebDAVItems);
            drawWebDAVItems(currentWebDAVItems);
        }
        else
        {
            Message(ICON_ERROR, "Error", "File is not available.", 1000);
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
                _contextMenu = std::unique_ptr<ContextMenu>(new ContextMenu());
                _contextMenu->createMenu(par2, _webDAVView->getCurrentEntry().state, EventHandler::contextMenuHandlerStatic);
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

                std::vector<WebDAVItem> currentWebDAVItems = _webDAV.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword());
                if (currentWebDAVItems.empty())
                {
                    Message(ICON_ERROR, "Error", "Login failed.", 1000);
                    HideHourglass();
                }
                else
                {
                    int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Do you want to choose your own storage path or use the default one. \n (/mnt/ext1/nextcloud/)", "Choose my own path", "Choose standard path", NULL);
                    auto path = "/mnt/ext1";
                    switch (dialogResult)
                    {
                        case 1:
                            {
                                FileBrowser fileBrowser = FileBrowser(false);
                                vector<FileItem> currentFolder = fileBrowser.getFileStructure(path);
                                _currentPath = path;
                                _loginView.reset();
                                FillAreaRect(&_menu->getContentRect(), WHITE);
                                _fileView = std::unique_ptr<FileView>(new FileView(_menu->getContentRect(), currentFolder,1));
                            }
                            break;
                        default:
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
                    FileBrowser fileBrowser = FileBrowser(false);
                    _currentPath = _fileView->getCurrentEntry().path;
                    vector<FileItem> currentFolder = fileBrowser.getFileStructure(_currentPath);

                    //TODO use other method
                    _fileView.reset();
                    _fileView = std::unique_ptr<FileView>(new FileView(_menu->getContentRect(), currentFolder,1));
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

    switch (_sqllite.getState(_webDAVView->getCurrentEntry().path))
    {
        case FileState::ILOCAL:
            {
                Message(ICON_ERROR, "Error", "Not implemented to look at local folder.", 2000);
                _webDAVView->invertCurrentEntryColor();
                //TODO use FileBrowser
                break; }
        case FileState::IOUTSYNCED:
        case FileState::ICLOUD:
            currentWebDAVItems = _webDAV.getDataStructure(_webDAVView->getCurrentEntry().path);
        case FileState::ISYNCED:
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
        default:
            break;
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


void EventHandler::getLocalFileStructure(vector<WebDAVItem> &items)
{
    //get local files, https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
    DIR *dir;
    class dirent *ent;
    class stat st;

    string localPath = items.at(0).localPath + '/';
    if (localPath.back() != '/')
        localPath = localPath + '/';
    if (iv_access(localPath.c_str(), W_OK) == 0)
    {
        dir = opendir(localPath.c_str());
        while ((ent = readdir(dir)) != NULL)
        {
            const string fileName = ent->d_name;

            if (fileName[0] == '.')
                continue;

            const string fullFileName = localPath + fileName;

            if (stat(fullFileName.c_str(), &st) == -1)
                continue;


            bool found = false;
            for (unsigned int i = 1; i < items.size(); i++)
            {
                if (items.at(i).localPath.compare(fullFileName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                WebDAVItem temp;
                temp.localPath = fullFileName;
                temp.state = FileState::ILOCAL;
                temp.title = fullFileName.substr(fullFileName.find_last_of("/") + 1, fullFileName.length());
                Util::decodeUrl(temp.title);
                if ((st.st_mode & S_IFDIR) != 0)
                {
                    //create new dir in cloud
                    temp.type = Itemtype::IFOLDER;
                }
                else
                {
                    //put to cloud
                    temp.type = Itemtype::IFILE;
                }
                items.push_back(temp);
            }
        }
        closedir(dir);
    }
}

void EventHandler::downloadFolder(vector<WebDAVItem> &items, int itemID)
{
    //BanSleep(2000);
    string path = items.at(itemID).path;
    Log::writeInfoLog("Path to look for " + path);

    if (items.at(itemID).type == Itemtype::IFOLDER)
    {
        vector<WebDAVItem> tempItems;
        if (items.at(itemID).state == FileState::IOUTSYNCED || items.at(itemID).state == FileState::ICLOUD)
        {
            Log::writeInfoLog(path + "outsynced");
            UpdateProgressbar(("Syncing folder" + path).c_str(), 0);
            tempItems = _webDAV.getDataStructure(path);
            updateItems(tempItems);
        }
        else
        {
            Log::writeInfoLog(path + "synced");
            tempItems = _sqllite.getItemsChildren(path);
        }
        //first item of the vector is the root path itself
        for (size_t i = 1; i < tempItems.size(); i++)
        {
            Log::writeInfoLog("Item: " + tempItems.at(i).path);
            downloadFolder(tempItems, i);
        }
        //TODO remove file parts that are no longer there, check for local path and delete these
        //get items from DB, then compare to downloaded, if is in DB but not downloaded, remove

    }
    else
    {
        if (items.at(itemID).state == FileState::IOUTSYNCED || items.at(itemID).state == FileState::ICLOUD)
        {
            Log::writeInfoLog("outsynced");
            //TODO both direction
            //1. check etag --> if is differnt, cloud has been updated
            //2. check modification date and file size locally --> if is different, local has been updated
            //3. if both --> create conflict
            //4. if first, renew file --> reset etag
            //5. if second --> upload the local file; test if it has not been update in the cloud
            Log::writeInfoLog("started download of " + items.at(itemID).path + " to " + items.at(itemID).localPath);
            if (_webDAV.get(items.at(itemID)))
            {
                items.at(itemID).state = FileState::ISYNCED;
                _sqllite.updateState(items.at(itemID).path,FileState::ISYNCED);
            }
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
            _sqllite.updateState(_webDAVView->getCurrentEntry().path,FileState::ISYNCED);
        }
    }
    else
    {
        vector<WebDAVItem> currentItems = _sqllite.getItemsChildren(_webDAVView->getCurrentEntry().path);
        this->downloadFolder(currentItems, 0);
        UpdateProgressbar("Download completed", 100);
        _webDAVView->getCurrentEntry().state = FileState::ISYNCED;
    }

    //TODO implement
    //Util::updatePBLibrary(15);
    CloseProgressbar();
    _webDAVView->reDrawCurrentEntry();
}

void EventHandler::updateItems(vector<WebDAVItem> &items)
{
    for(auto &item : items)
    {
        item.state = _sqllite.getState(item.path);

        if (iv_access(item.localPath.c_str(), W_OK) != 0)
        {
            if (item.type == Itemtype::IFILE)
                item.state = FileState::ICLOUD;
            else
                iv_mkdir(item.localPath.c_str(), 0777);
        }

        if (_sqllite.getEtag(item.path).compare(item.etag) != 0)
        {
            item.state = (item.state == FileState::ISYNCED) ? FileState::IOUTSYNCED : FileState::ICLOUD;
        }
    }
    items.at(0).state =FileState::ISYNCED;
    _sqllite.saveItemsChildren(items);

    //TODO sync delete when not parentPath existend --> "select * from metadata where parentPath NOT IN (Select
    //DISTINCT(parentPath) from metadata;
    //what happens with the entries below?
}


void EventHandler::drawWebDAVItems(vector<WebDAVItem> &items)
{
    _webDAVView.release();
    _currentPath = items.at(0).path;
    getLocalFileStructure(items);
    _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu->getContentRect(), items,1));
}
