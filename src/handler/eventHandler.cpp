//-----------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "eventHandler.h"

#include <algorithm>
#include <experimental/filesystem>
#include <memory>
#include <string>

#include "contextMenu.h"
#include "fileBrowser.h"
#include "fileHandler.h"
#include "fileModel.h"
#include "fileView.h"
#include "inkview.h"
#include "log.h"
#include "mainMenu.h"
#include "util.h"
#include "webDAV.h"
#include "webDAVModel.h"
#include "webDAVView.h"

using std::string;
using std::vector;

namespace fs = std::experimental::filesystem;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;

EventHandler::EventHandler()
{
    // create an copy of the eventhandler to handle methods that require static
    // functions
    _eventHandlerStatic = std::unique_ptr<EventHandler>(this);

    _fileHandler = std::make_shared<FileHandler>();
    _menu = std::make_unique<MainMenu>(APPLICATION_NAME);

    if (iv_access(CONFIG_FILE_LOCATION.c_str(), W_OK) == 0)
    {
        // for backwards compatibilty
        // all to consts
        if (Util::getConfig<string>(CONF_STORAGE_LOCATION, "error").compare("error") == 0)
        {
            Util::writeConfig<string>(CONF_STORAGE_LOCATION, DEFAULT_STORAGE_LOCATION);
        }

        if (iv_access(Util::getConfig<string>(CONF_STORAGE_LOCATION).c_str(), W_OK) != 0)
        {
            iv_mkdir(Util::getConfig<string>(CONF_STORAGE_LOCATION).c_str(), 0777);
        }

        auto path{WebDAV::getRootPath(true)};
        auto currentWebDAVItems{_webDAV.getDataStructure(path)};

        if (currentWebDAVItems.empty())
        {
            currentWebDAVItems = _sqllite.getItemsChildren(path);
        }
        else
        {
            updateItems(currentWebDAVItems);
        }

        if (currentWebDAVItems.empty())
        {
            auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION,
                                            "Could not login and there is no DB available to "
                                            "restore information. What would you like to do?",
                                            TEXT_DIALOG_LOGOUT, TEXT_DIALOG_CLOSE_APP, NULL)};
            switch (dialogResult)
            {
            case 1:
                _webDAV.logout(true);
                _loginView = std::make_unique<LoginView>(_menu->getContentRect());
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
        _loginView = std::make_unique<LoginView>(_menu->getContentRect());
    }
    _menu->draw();
}

int EventHandler::eventDistributor(int type, int par1, int par2)
{
    if (ISPOINTEREVENT(type))
        return EventHandler::pointerHandler(type, par1, par2);
    else if (ISKEYEVENT(type))
        return EventHandler::keyHandler(type, par1, par2);

    return 1;
}

void EventHandler::mainMenuHandlerStatic(int p_index)
{
    auto mainMenuOption{static_cast<MainMenuOption>(p_index)};
    _eventHandlerStatic->mainMenuHandler(mainMenuOption);
}

void EventHandler::mainMenuHandler(MainMenuOption p_mainMenuOption)
{
    switch (p_mainMenuOption)
    {
    case MainMenuOption::ActualizeCurrentFolder: {
        OpenProgressbar(1, "Actualizing current folder", ("Actualizing path" + _currentPath).c_str(), 0, NULL);
        auto childrenPath{_currentPath};
        childrenPath = childrenPath.substr(NEXTCLOUD_ROOT_PATH.length(), childrenPath.length());
        std::string path{NEXTCLOUD_ROOT_PATH};
        std::vector<WebDAVItem> currentWebDAVItems;
        size_t found = 0;
        auto i{0};
        while ((found = childrenPath.find("/"), found) != std::string::npos)
        {
            path += childrenPath.substr(0, found + 1);
            childrenPath = childrenPath.substr(found + 1, childrenPath.length());
            auto state{_sqllite.getState(path)};
            Log::writeInfoLog("current path " + path);
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
                Message(ICON_WARNING, TEXT_MESSAGE_WARNING, "Could not sync the file structure.", TIMEOUT_MESSAGE);
                HideHourglass();
                break;
            }
            else
            {
                updateItems(currentWebDAVItems);
            }
            i++;
        }

        Log::writeInfoLog("stopped at " + path);
        currentWebDAVItems = _sqllite.getItemsChildren(_currentPath);

        for (auto &item : currentWebDAVItems)
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
    case MainMenuOption::Logout: {
        auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION, "Do you want to delete local files?",
                                        TEXT_DIALOG_YES, TEXT_DIALOG_NO, TEXT_DIALOG_CANCEL)};
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
        _loginView = std::make_unique<LoginView>(_menu->getContentRect());
        break;
    }
    case MainMenuOption::SortBy: {
        auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION, "By what do you want to sort?", "Filename",
                                        "Last modified", TEXT_DIALOG_CANCEL)};
        switch (dialogResult)
        {
        case 1:
            Util::writeConfig<int>("sortBy", 1);
            break;
        case 2:
            Util::writeConfig<int>("sortBy", 2);
            break;
        default:
            return;
        }
        Message(ICON_INFORMATION, TEXT_MESSAGE_INFO, "Reload page to see new order method in effect.", TIMEOUT_MESSAGE);
        break;
    }
    case MainMenuOption::ExcludeFiles: {
        if (_fileView != nullptr)
        {
            _currentPath = _fileView->getCurrentEntry().path;
            _fileView.reset();
        }
        else
        {
            _currentPath = "";
        }

        _webDAVView.reset();
        FillAreaRect(&_menu->getContentRect(), WHITE);
        _excludeFileView = std::make_unique<ExcludeFileView>(_menu->getContentRect());
        break;
    }
    case MainMenuOption::ChooseFolder: {
        _currentPath = _currentPath + ((_currentPath.back() != '/') ? "/nextcloud" : "nextcloud");

        if (iv_mkdir(_currentPath.c_str(), 0777) != 0)
        {
            Log::writeErrorLog("choosen part " + _currentPath +
                               " could not be created as permission are not sufficient.");
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "The permissions are not sufficient.", TIMEOUT_MESSAGE);
        }
        else
        {
            Util::writeConfig<string>(CONF_STORAGE_LOCATION, _currentPath);
            auto currentWebDAVItems{_webDAV.getDataStructure(WebDAV::getRootPath(true))};
            if (currentWebDAVItems.empty())
            {
                Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Failed to get items. Please try again.", TIMEOUT_MESSAGE);
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
    case MainMenuOption::Info: {
        string message;
        Message(ICON_INFORMATION, TEXT_MESSAGE_INFO,
                message.append("Version ")
                    .append(PROGRAMVERSION)
                    .append("\nFor support please open a ticket at "
                            "https://github.com/JuanJakobo/"
                            "Pocketbook-Nextcloud-Client/issues")
                    .c_str(),
                TIMEOUT_MESSAGE);
        break;
    }
    case MainMenuOption::Exit:
        CloseApp();
        break;
    default:
        break;
    }
}

void EventHandler::contextMenuHandlerStatic(int p_index)
{
    auto contextMenuOption{static_cast<ContextMenuOption>(p_index)};
    _eventHandlerStatic->contextMenuHandler(contextMenuOption);
}

void EventHandler::contextMenuHandler(ContextMenuOption p_contextMenuOption)
{
    switch (p_contextMenuOption)
    {
    case ContextMenuOption::Open:
        if (_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
        {
            openFolder();
        }
        else
        {
            openItem();
        }
        break;
    case ContextMenuOption::Sync:
        if (_webDAVView->getCurrentEntry().state != FileState::ILOCAL)
        {
            startDownload();
        }
        else
        {
            // TODO upload to cloud
            // Dialog file is offline, do you want to sync it to the cloud?
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "The File is local and upload to cloud is currently not supported.",
                    TIMEOUT_MESSAGE);
            _webDAVView->invertCurrentEntryColor();
        }
        break;
    case ContextMenuOption::Remove:
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
            auto currentWebDAVItems{_sqllite.getItemsChildren(_currentPath)};
            updateItems(currentWebDAVItems);
            drawWebDAVItems(currentWebDAVItems);
        }
        else
        {
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "File is not available locally.", TIMEOUT_MESSAGE);
            _webDAVView->invertCurrentEntryColor();
        }
        break;
    default:
        _webDAVView->invertCurrentEntryColor();
        break;
    }
}

int EventHandler::pointerHandler(const int type, const int par1, const int par2)
{
    // long press to open up context menu
    if (type == EVT_POINTERLONG && _webDAVView)
    {
        _webDAVView->checkIfEntryClicked(par1, par2);
        _webDAVView->invertCurrentEntryColor();
        if (_webDAVView->getCurrentEntry().title.compare("...") != 0)
        {
            _contextMenu.createMenu(par2, _webDAVView->getCurrentEntry().state, EventHandler::contextMenuHandlerStatic);
        }
    }
    else if (type == EVT_POINTERUP)
    {
        if (IsInRect(par1, par2, &_menu->getMenuButtonRect()) == 1)
        {
            _menu->open((_fileView != nullptr), (_webDAVView != nullptr), EventHandler::mainMenuHandlerStatic);
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
        else if (_excludeFileView != nullptr)
        {
            auto const click{_excludeFileView->excludeClicked(par1, par2)};
            if (click == 3)
            {
                Util::writeConfig<string>(CONF_EXTENSION_LIST, _excludeFileView->getExtensionList());
                Util::writeConfig<string>(CONF_EXTENSION_PATTERN, _excludeFileView->getRegex());
                Util::writeConfig<string>(CONF_EXTENSION_FOLDER_PATTERN, _excludeFileView->getFolderRegex());
                Util::writeConfig<string>(CONF_EXTENSION_RELATIVE_ROOT_PATH, _excludeFileView->getStartFolder());
                Util::writeConfig<int>(CONF_EXTENSION_INVERT_MATCH, _excludeFileView->getInvertMatch());

                _sqllite.resetHideState();
                if (_excludeFileView->getStartFolder() != "")
                {
                    _sqllite.deleteItemsNotBeginsWith(WebDAV::getRootPath(true));
                }

                _excludeFileView.reset();
                ShowHourglassForce();

                FillAreaRect(&_menu->getContentRect(), WHITE);
                if (_currentPath != "")
                {
                    const auto currentFolder{FileBrowser::getFileStructure(_currentPath, false, true)};
                    _fileView.reset(new FileView(_menu->getContentRect(), currentFolder, 1));
                }
                else
                {
                    auto currentWebDAVItems = _webDAV.getDataStructure(WebDAV::getRootPath(true));
                    updateItems(currentWebDAVItems);
                    drawWebDAVItems(currentWebDAVItems);
                }
            }
            else if (click == -1)
            {
                _excludeFileView.reset();
                ShowHourglassForce();

                FillAreaRect(&_menu->getContentRect(), WHITE);
                if (_currentPath != "")
                {
                    const auto currentFolder = FileBrowser::getFileStructure(_currentPath, false, true);
                    _fileView.reset(new FileView(_menu->getContentRect(), currentFolder, 1));
                }
                else
                {
                    auto currentWebDAVItems{_webDAV.getDataStructure(WebDAV::getRootPath(true))};
                    updateItems(currentWebDAVItems);
                    drawWebDAVItems(currentWebDAVItems);
                }
            }
        }
        // if loginView is shown
        else if (_loginView != nullptr)
        {
            if (_loginView->logginClicked(par1, par2) == 2)
            {
                ShowHourglassForce();

                auto currentWebDAVItems{_webDAV.login(_loginView->getURL(), _loginView->getUsername(),
                                                      _loginView->getPassword(), _loginView->getIgnoreCert())};
                if (currentWebDAVItems.empty())
                {
                    Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Login failed.", TIMEOUT_MESSAGE);
                    HideHourglass();
                }
                else
                {
                    const auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION,
                                                          "Do you want to choose your own storage path or "
                                                          "use the default one. \n (/mnt/ext1/nextcloud/)",
                                                          "Choose my own path", "Choose standard path", NULL)};
                    switch (dialogResult)
                    {
                    case 1: {
                        const auto currentFolder{FileBrowser::getFileStructure(FLASHDIR, false, true)};
                        _currentPath = FLASHDIR;
                        FillAreaRect(&_menu->getContentRect(), WHITE);
                        _fileView = std::make_unique<FileView>(_menu->getContentRect(), currentFolder, 1);
                    }
                    break;
                    case 2:
                    default:
                        if (iv_access(Util::getConfig<string>(CONF_STORAGE_LOCATION).c_str(), W_OK) != 0)
                            iv_mkdir(Util::getConfig<string>(CONF_STORAGE_LOCATION).c_str(), 0777);
                        updateItems(currentWebDAVItems);
                        drawWebDAVItems(currentWebDAVItems);
                        break;
                    }
                    _loginView.reset(nullptr);
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
                    const auto currentFolder{FileBrowser::getFileStructure(_currentPath, false, true)};
                    _fileView.reset(new FileView(_menu->getContentRect(), currentFolder, 1));
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
        Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Could not find file.", TIMEOUT_MESSAGE);
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
             _webDAVView->getCurrentEntry().fileType.find("application/"
                                                          "vnd.openxmlformats-officedocument.wordprocessingml."
                                                          "document") != string::npos ||
             _webDAVView->getCurrentEntry().fileType.find("application/x-fictionbook+xml") != string::npos)
    {
        OpenBook(_webDAVView->getCurrentEntry().localPath.c_str(), "", 0);
    }
    else
    {
        Message(ICON_INFORMATION, TEXT_MESSAGE_INFO, "The filetype is currently not supported.", TIMEOUT_MESSAGE);
    }
}

void EventHandler::openFolder()
{
    std::vector<WebDAVItem> currentWebDAVItems;

    switch ((_webDAVView->getCurrentEntry().state == FileState::ILOCAL)
                ? FileState::ILOCAL
                : _sqllite.getState(_webDAVView->getCurrentEntry().path))
    {
    case FileState::ILOCAL: {
        Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Not implemented to look at local folder.", TIMEOUT_MESSAGE);
        _webDAVView->invertCurrentEntryColor();
        // TODO use FileBrowser
        //_webDAVView.reset();
        // FileBrowser fB = FileBrowser(true);
        //_fileView.reset(new
        // FileView(_menu->getContentRect(),fB.getFileStructure(_webDAVView->getCurrentEntry().path),1));
        break;
    }
    case FileState::IOUTSYNCED:
    case FileState::ICLOUD:
        ShowHourglassForce();
        currentWebDAVItems = _webDAV.getDataStructure(_webDAVView->getCurrentEntry().path);
    case FileState::ISYNCED:
    case FileState::IDOWNLOADED: {
        if (currentWebDAVItems.empty() && _webDAVView->getCurrentEntry().state != FileState::ICLOUD)
            currentWebDAVItems = _sqllite.getItemsChildren(_webDAVView->getCurrentEntry().path);
        updateItems(currentWebDAVItems);

        if (currentWebDAVItems.empty())
        {
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Could not sync the items and there is no offline copy available.",
                    TIMEOUT_MESSAGE);
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
            switch (par1)
            {
            // menu button
            case 23:
                _webDAVView->firstPage();
                break;
                // left button
            case 24:
                _webDAVView->prevPage();
                break;
                // right button
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
            switch (par1)
            {
            // menu button
            case 23:
                _fileView->firstPage();
                break;
                // left button
            case 24:
                _fileView->prevPage();
                break;
                // right button
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
        const auto currentFolder{FileBrowser::getFileStructure(localPath, true, false)};
        const auto storageLocationLength{_fileHandler->getStorageLocation().length()};
        for (const FileItem &local : currentFolder)
        {
            auto p = find_if(tempItems.begin() + 1, tempItems.end(),
                             [&](const WebDAVItem &item) { return item.localPath.compare(local.path) == 0; });
            if (p == tempItems.end())
            {
                WebDAVItem temp;
                temp.localPath = local.path;
                temp.state = FileState::ILOCAL;
                temp.title = temp.localPath.substr(temp.localPath.find_last_of('/') + 1, temp.localPath.length());
                // Log::writeInfoLog(std::to_string(fs::file_size(entry)));
                temp.lastEditDate = local.lastEditDate;

                string directoryPath = temp.localPath;
                if (directoryPath.length() > storageLocationLength)
                {
                    directoryPath = directoryPath.substr(storageLocationLength + 1);
                }
                if (local.type == Type::FFOLDER)
                {
                    if (_fileHandler->excludeFolder(directoryPath + "/"))
                    {
                        continue;
                    }
                    // create new dir in cloud
                    temp.type = Itemtype::IFOLDER;
                }
                else
                {
                    // put to cloud
                    temp.type = Itemtype::IFILE;
                    if (directoryPath.length() > temp.title.length())
                    {
                        directoryPath = directoryPath.substr(0, directoryPath.length() - temp.title.length());
                    }
                    if (_fileHandler->excludeFolder(directoryPath) || _fileHandler->excludeFile(temp.title))
                    {
                        continue;
                    }
                }
                tempItems.push_back(temp);
            }
        }
    }
}

void EventHandler::downloadFolder(vector<WebDAVItem> &items, int itemID)
{
    // Don't sync hidden files
    if (items.at(itemID).hide == HideState::IHIDE)
        return;

    // BanSleep(2000);
    string path = items.at(itemID).path;

    if (items.at(itemID).type == Itemtype::IFOLDER)
    {
        vector<WebDAVItem> tempItems;
        switch (items.at(itemID).state)
        {
        case FileState::IOUTSYNCED:
        case FileState::ICLOUD: {
            UpdateProgressbar(("Syncing folder" + path).c_str(), 0);
            iv_mkdir(items.at(itemID).localPath.c_str(), 0777);
            tempItems = _webDAV.getDataStructure(path);
            items.at(itemID).state = FileState::IDOWNLOADED;
            _sqllite.updateState(items.at(itemID).path, items.at(itemID).state);
            updateItems(tempItems);
            break;
        }
        case FileState::ISYNCED: {
            tempItems = _sqllite.getItemsChildren(path);
            iv_mkdir(items.at(itemID).localPath.c_str(), 0777);
            items.at(itemID).state = FileState::IDOWNLOADED;
            _sqllite.updateState(items.at(itemID).path, items.at(itemID).state);
            break;
        }
        case FileState::ILOCAL: {
            if (items.at(itemID).localPath.length() > 3 &&
                items.at(itemID).localPath.substr(items.at(itemID).localPath.length() - 3).compare("sdr") == 0)
                Log::writeInfoLog("Ignoring koreader file " + items.at(itemID).localPath);
            else
            {
                CloseProgressbar();

                int dialogResult = DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION,
                                                 ("The folder " + items.at(itemID).localPath +
                                                  " has been removed from the cloud. Do you want to delete it?")
                                                     .c_str(),
                                                 TEXT_DIALOG_YES, TEXT_DIALOG_NO, TEXT_DIALOG_CANCEL);
                if (dialogResult == 1)
                    fs::remove_all(items.at(itemID).localPath);
                else
                    tempItems.push_back(items.at(itemID));

                // OpenProgressbar(1, "Downloading...", "", 0, NULL);
            }
            break;
        }
        default:
            break;
        }

        if (!tempItems.empty())
        {
            getLocalFileStructure(tempItems);
            // first item of the vector is the root path itself
            for (size_t i = 1; i < tempItems.size(); i++)
                downloadFolder(tempItems, i);
        }

        // compare if file is in DB
        // if file is not in DB, upload
        // if file is in DB but no longer in Cloud, delete (if file local is newer
        // prompt?) if file is newer offline, upload delete in DB if folder is no
        // longer synced
        // TODO remove file parts that are no longer there, check for local path and
        // delete these
    }
    else
    {
        switch (items.at(itemID).state)
        {
        case FileState::IOUTSYNCED:
        case FileState::ICLOUD: {
            // TODO both direction
            // 1. check etag --> if is differnt, cloud has been updated
            // 2. check modification date and file size locally --> if is different,
            // local has been updated
            // 3. if both --> create conflict
            // 4. if first, renew file --> reset etag
            // 5. if second --> upload the local file; test if it has not been
            // update in the cloud
            Log::writeInfoLog("started download of " + items.at(itemID).path + " to " + items.at(itemID).localPath);
            if (_webDAV.get(items.at(itemID)))
            {
                items.at(itemID).state = FileState::ISYNCED;
                _sqllite.updateState(items.at(itemID).path, items.at(itemID).state);
            }
            break;
        }
        case FileState::ILOCAL: {
            CloseProgressbar();
            const auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION,
                                                  ("The file " + items.at(itemID).localPath +
                                                   " has been removed from the cloud. Do you want to delete it?")
                                                      .c_str(),
                                                  TEXT_DIALOG_YES, TEXT_DIALOG_NO, TEXT_DIALOG_CANCEL)};
            if (dialogResult == 1)
                fs::remove(items.at(itemID).localPath);
            break;
            // OpenProgressbar(1, "Downloading...", "", 0, NULL);
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
        Log::writeInfoLog("Started download of " + _webDAVView->getCurrentEntry().path + " to " +
                          _webDAVView->getCurrentEntry().localPath);
        if (_webDAV.get(_webDAVView->getCurrentEntry()))
        {
            _webDAVView->getCurrentEntry().state = FileState::ISYNCED;
            _sqllite.updateState(_webDAVView->getCurrentEntry().path, _webDAVView->getCurrentEntry().state);
        }
    }
    else
    {
        auto currentItems{_sqllite.getItemsChildren(_webDAVView->getCurrentEntry().path)};
        this->downloadFolder(currentItems, 0);
        _webDAVView->getCurrentEntry().state = FileState::IDOWNLOADED;
        _sqllite.updateState(_webDAVView->getCurrentEntry().path, _webDAVView->getCurrentEntry().state);
        UpdateProgressbar("Download completed", 100);
    }

    // TODO implement
    // Util::updatePBLibrary(15);
    CloseProgressbar();
    _webDAVView->reDrawCurrentEntry();
}

bool EventHandler::checkIfIsDownloaded(vector<WebDAVItem> &items, int itemID)
{
    if (iv_access(items.at(itemID).localPath.c_str(), W_OK) != 0)
    {
        items.at(itemID).state = items.at(itemID).type == Itemtype::IFOLDER
                                     ? items.at(itemID).state = FileState::ISYNCED
                                     : items.at(itemID).state = FileState::IOUTSYNCED;
        _sqllite.updateState(items.at(itemID).path, items.at(itemID).state);
        return false;
    }

    if (items.at(itemID).type == Itemtype::IFOLDER)
    {
        if (items.at(itemID).state != FileState::IDOWNLOADED)
            return false;
        auto tempItems{_sqllite.getItemsChildren(items.at(itemID).path)};
        // first item of the vector is the root path itself
        for (auto i = 1; i < tempItems.size(); i++)
        {
            if (!checkIfIsDownloaded(tempItems, i))
            {
                items.at(itemID).state = FileState::ISYNCED;
                _sqllite.updateState(items.at(itemID).path, items.at(itemID).state);
                return false;
            }
        }
    }
    return true;
}

void EventHandler::updateItems(vector<WebDAVItem> &items)
{
    for (auto &item : items)
    {
        // returns ICloud if is not found
        item.state = _sqllite.getState(item.path);

        if (item.type == Itemtype::IFILE)
        {
            if (iv_access(item.localPath.c_str(), W_OK) != 0)
                item.state = FileState::ICLOUD;
            else
            {
                item.state = FileState::ISYNCED;
                if (_sqllite.getEtag(item.path).compare(item.etag) != 0)
                    item.state = FileState::IOUTSYNCED;
            }
        }
        else
        {
            if (_sqllite.getEtag(item.path).compare(item.etag) != 0)
                item.state = (item.state == FileState::ISYNCED || item.state == FileState::IDOWNLOADED)
                                 ? FileState::IOUTSYNCED
                                 : FileState::ICLOUD;
            if (item.state == FileState::IDOWNLOADED)
            {
                vector<WebDAVItem> currentItems = _sqllite.getItemsChildren(item.path);
                if (!checkIfIsDownloaded(currentItems, 0))
                    item.state = FileState::ISYNCED;
            }

            if (iv_access(item.localPath.c_str(), W_OK) != 0)
                iv_mkdir(item.localPath.c_str(), 0777);
        }
    }
    if (items.at(0).state != FileState::IDOWNLOADED)
        items.at(0).state = FileState::ISYNCED;
    _sqllite.saveItemsChildren(items);

    // TODO sync delete when not parentPath existend --> "select * from metadata
    // where parentPath NOT IN (Select DISTINCT(parentPath) from metadata; what
    // happens with the entries below?
}

void EventHandler::drawWebDAVItems(vector<WebDAVItem> &items)
{
    _currentPath = items.at(0).path;
    getLocalFileStructure(items);
    _webDAVView.reset(new WebDAVView(_menu->getContentRect(), items, 1));
}
