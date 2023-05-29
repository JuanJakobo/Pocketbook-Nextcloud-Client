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

namespace
{
enum class PocketbookButtons
{
    Back,
    Forward,
    Menu
};
} // namespace

namespace fs = std::experimental::filesystem;

std::unique_ptr<EventHandler> EventHandler::m_eventHandlerStatic;

EventHandler::EventHandler()
{
    // create an copy of the eventhandler to handle methods that require static
    // functions
    m_eventHandlerStatic = std::unique_ptr<EventHandler>(this);

    m_fileHandler = std::make_shared<FileHandler>();
    m_menu = std::make_unique<MainMenu>(APPLICATION_NAME);

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
        auto currentWebDAVItems{m_webDAV.getDataStructure(path)};

        if (currentWebDAVItems.empty())
        {
            currentWebDAVItems = m_sqllite.getItemsChildren(path);
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
                m_webDAV.logout(true);
                m_loginView = std::make_unique<LoginView>(m_menu->getContentRect());
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
        m_loginView = std::make_unique<LoginView>(m_menu->getContentRect());
    }
    m_menu->draw();
}

int EventHandler::eventDistributor(int p_type, int p_par1, int p_par2)
{
    if (ISPOINTEREVENT(p_type))
    {
        return EventHandler::pointerHandler(p_type, p_par1, p_par2);
    }
    else if (ISKEYEVENT(p_type))
    {
        return EventHandler::keyHandler(p_type, p_par1, p_par2);
    }
    return 1;
}

void EventHandler::mainMenuHandlerStatic(int p_index)
{
    auto mainMenuOption{static_cast<MainMenuOption>(p_index)};
    m_eventHandlerStatic->mainMenuHandler(mainMenuOption);
}

void EventHandler::mainMenuHandler(MainMenuOption p_mainMenuOption)
{
    switch (p_mainMenuOption)
    {
    case MainMenuOption::ActualizeCurrentFolder: {
        OpenProgressbar(1, "Actualizing current folder", ("Actualizing path" + m_currentPath).c_str(), 0, NULL);
        auto childrenPath{m_currentPath};
        childrenPath = childrenPath.substr(NEXTCLOUD_ROOT_PATH.length(), childrenPath.length());
        std::string path{NEXTCLOUD_ROOT_PATH};
        std::vector<WebDAVItem> currentWebDAVItems;
        size_t found{0};
        auto counter{0};
        while ((found = childrenPath.find("/"), found) != std::string::npos)
        {
            path += childrenPath.substr(0, found + 1);
            childrenPath = childrenPath.substr(found + 1, childrenPath.length());
            auto state{m_sqllite.getState(path)};
            Log::writeInfoLog("current path " + path);
            if (counter < 1 || state == FileState::IOUTSYNCED || state == FileState::ICLOUD)
            {
                UpdateProgressbar(("Upgrading " + path).c_str(), 0);
                currentWebDAVItems = m_webDAV.getDataStructure(path);
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
            counter++;
        }

        Log::writeInfoLog("stopped at " + path);
        currentWebDAVItems = m_sqllite.getItemsChildren(m_currentPath);

        for (const auto &item : currentWebDAVItems)
        {
            Log::writeInfoLog(item.path);
            if (item.type == Itemtype::IFOLDER && item.state == FileState::IOUTSYNCED)
            {
                UpdateProgressbar(("Upgrading " + item.path).c_str(), 0);
                auto tempWebDAVItems{m_webDAV.getDataStructure(item.path)};
                updateItems(tempWebDAVItems);
            }
        }
        currentWebDAVItems = m_sqllite.getItemsChildren(m_currentPath);

        CloseProgressbar();
        drawWebDAVItems(currentWebDAVItems);
        break;
    }
    case MainMenuOption::Logout: {
        const auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION, "Do you want to delete local files?",
                                              TEXT_DIALOG_YES, TEXT_DIALOG_NO, TEXT_DIALOG_CANCEL)};
        switch (dialogResult)
        {
        case 1:
            m_webDAV.logout(true);
            break;
        case 2:
            m_webDAV.logout();
            break;
        case 3:
        default:
            return;
        }
        m_webDAVView.reset(nullptr);
        m_loginView = std::make_unique<LoginView>(m_menu->getContentRect());
        break;
    }
    case MainMenuOption::SortBy: {
        const auto dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION, "By what do you want to sort?",
                                              "Filename", "Last modified", TEXT_DIALOG_CANCEL)};
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
        if (m_fileView != nullptr)
        {
            m_currentPath = m_fileView->getCurrentEntry().path;
            m_fileView.reset();
        }
        else
        {
            m_currentPath = {};
        }
        m_webDAVView.reset(nullptr);
        FillAreaRect(&m_menu->getContentRect(), WHITE);
        m_excludeFileView = std::make_unique<ExcludeFileView>(m_menu->getContentRect());
        break;
    }
    case MainMenuOption::ChooseFolder: {
        m_currentPath = m_currentPath + ((m_currentPath.back() != '/') ? "/nextcloud" : "nextcloud");

        if (iv_mkdir(m_currentPath.c_str(), 0777) != 0)
        {
            Log::writeErrorLog("choosen part " + m_currentPath +
                               " could not be created as permission are not sufficient.");
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "The permissions are not sufficient.", TIMEOUT_MESSAGE);
        }
        else
        {
            Util::writeConfig<string>(CONF_STORAGE_LOCATION, m_currentPath);
            auto currentWebDAVItems{m_webDAV.getDataStructure(WebDAV::getRootPath(true))};
            if (currentWebDAVItems.empty())
            {
                Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Failed to get items. Please try again.", TIMEOUT_MESSAGE);
            }
            else
            {
                m_fileView.reset(nullptr);
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
    m_eventHandlerStatic->contextMenuHandler(contextMenuOption);
}

void EventHandler::contextMenuHandler(ContextMenuOption p_contextMenuOption)
{
    switch (p_contextMenuOption)
    {
    case ContextMenuOption::Open:
        if (m_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
        {
            openFolder();
        }
        else
        {
            openItem();
        }
        break;
    case ContextMenuOption::Sync:
        if (m_webDAVView->getCurrentEntry().state != FileState::ILOCAL)
        {
            startDownload();
        }
        else
        {
            // TODO upload to cloud
            // Dialog file is offline, do you want to sync it to the cloud?
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "The File is local and upload to cloud is currently not supported.",
                    TIMEOUT_MESSAGE);
            m_webDAVView->invertCurrentEntryColor();
        }
        break;
    case ContextMenuOption::Remove:
        if (m_webDAVView->getCurrentEntry().state != FileState::ICLOUD)
        {
            if (m_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
            {
                fs::remove_all(m_webDAVView->getCurrentEntry().localPath);
            }
            else
            {
                fs::remove(m_webDAVView->getCurrentEntry().localPath);
            }
            auto currentWebDAVItems{m_sqllite.getItemsChildren(m_currentPath)};
            updateItems(currentWebDAVItems);
            drawWebDAVItems(currentWebDAVItems);
        }
        else
        {
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "File is not available locally.", TIMEOUT_MESSAGE);
            m_webDAVView->invertCurrentEntryColor();
        }
        break;
    default:
        m_webDAVView->invertCurrentEntryColor();
        break;
    }
}

int EventHandler::pointerHandler(int p_type, int p_point_x, int p_point_y)
{
    // long press to open up context menu
    if (p_type == EVT_POINTERLONG && m_webDAVView != nullptr)
    {
        m_webDAVView->checkIfEntryClicked(p_point_x, p_point_y);
        m_webDAVView->invertCurrentEntryColor();
        if (m_webDAVView->getCurrentEntry().title.compare("...") != 0)
        {
            m_contextMenu.createMenu(p_point_y, m_webDAVView->getCurrentEntry().state,
                                     EventHandler::contextMenuHandlerStatic);
        }
    }
    else if (p_type == EVT_POINTERUP)
    {
        if (IsInRect(p_point_x, p_point_y, &m_menu->getMenuButtonRect()) == 1)
        {
            m_menu->open((m_fileView != nullptr), (m_webDAVView != nullptr), EventHandler::mainMenuHandlerStatic);
        }
        else if (m_webDAVView != nullptr)
        {
            if (m_webDAVView->checkIfEntryClicked(p_point_x, p_point_y))
            {
                m_webDAVView->invertCurrentEntryColor();
                if (m_webDAVView->getCurrentEntry().type == Itemtype::IFOLDER)
                {
                    openFolder();
                }
                else
                {
                    if (m_webDAVView->getCurrentEntry().state != FileState::ICLOUD)
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
        else if (m_excludeFileView != nullptr)
        {
            auto const click{m_excludeFileView->excludeClicked(p_point_x, p_point_y)};
            if (click == 3)
            {
                Util::writeConfig<string>(CONF_EXTENSION_LIST, m_excludeFileView->getExtensionList());
                Util::writeConfig<string>(CONF_EXTENSION_PATTERN, m_excludeFileView->getRegex());
                Util::writeConfig<string>(CONF_EXTENSION_FOLDER_PATTERN, m_excludeFileView->getFolderRegex());
                Util::writeConfig<string>(CONF_EXTENSION_RELATIVE_ROOT_PATH, m_excludeFileView->getStartFolder());
                Util::writeConfig<int>(CONF_EXTENSION_INVERT_MATCH, m_excludeFileView->getInvertMatch());

                m_sqllite.resetHideState();
                if (m_excludeFileView->getStartFolder() != "")
                {
                    m_sqllite.deleteItemsNotBeginsWith(WebDAV::getRootPath(true));
                }
                redrawItems();
            }
            else if (click == -1)
            {
                redrawItems();
            }
        }
        // if loginView is shown
        else if (m_loginView != nullptr)
        {
            if (m_loginView->logginClicked(p_point_x, p_point_y) == 2)
            {
                ShowHourglassForce();

                auto currentWebDAVItems{m_webDAV.login(m_loginView->getURL(), m_loginView->getUsername(),
                                                       m_loginView->getPassword(), m_loginView->getIgnoreCert())};
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
                        m_currentPath = FLASHDIR;
                        FillAreaRect(&m_menu->getContentRect(), WHITE);
                        m_fileView = std::make_unique<FileView>(m_menu->getContentRect(), currentFolder, 1);
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
                    m_loginView.reset(nullptr);
                }
                return 0;
            }
        }
        else if (m_fileView != nullptr)
        {
            if (m_fileView->checkIfEntryClicked(p_point_x, p_point_y))
            {
                m_fileView->invertCurrentEntryColor();

                if (m_fileView->getCurrentEntry().type == Type::FFOLDER)
                {
                    m_currentPath = m_fileView->getCurrentEntry().path;
                    const auto currentFolder{FileBrowser::getFileStructure(m_currentPath, false, true)};
                    m_fileView.reset(new FileView(m_menu->getContentRect(), currentFolder, 1));
                }
            }

            return 0;
        }
    }
    return 1;
}

void EventHandler::openItem()
{
    m_webDAVView->invertCurrentEntryColor();
    if (m_webDAVView->getCurrentEntry().state == FileState::ICLOUD)
    {
        Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Could not find file.", TIMEOUT_MESSAGE);
    }
    else if (m_webDAVView->getCurrentEntry().fileType.find("application/epub+zip") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("application/pdf") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("application/octet-stream") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("text/plain") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("text/html") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("text/rtf") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("text/markdown") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("application/msword") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("application/x-mobipocket-ebook") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("application/"
                                                           "vnd.openxmlformats-officedocument.wordprocessingml."
                                                           "document") != string::npos ||
             m_webDAVView->getCurrentEntry().fileType.find("application/x-fictionbook+xml") != string::npos)
    {
        OpenBook(m_webDAVView->getCurrentEntry().localPath.c_str(), "", 0);
    }
    else
    {
        Message(ICON_INFORMATION, TEXT_MESSAGE_INFO, "The filetype is currently not supported.", TIMEOUT_MESSAGE);
    }
}

void EventHandler::openFolder()
{
    std::vector<WebDAVItem> currentWebDAVItems;

    const auto current_state{(m_webDAVView->getCurrentEntry().state == FileState::ILOCAL)
                                 ? FileState::ILOCAL
                                 : m_sqllite.getState(m_webDAVView->getCurrentEntry().path)};
    switch(current_state)
    {
    case FileState::ILOCAL: {
        Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Not implemented to look at local folder.", TIMEOUT_MESSAGE);
        m_webDAVView->invertCurrentEntryColor();
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
        currentWebDAVItems = m_webDAV.getDataStructure(m_webDAVView->getCurrentEntry().path);
    case FileState::ISYNCED:
    case FileState::IDOWNLOADED: {
        if (currentWebDAVItems.empty() && m_webDAVView->getCurrentEntry().state != FileState::ICLOUD)
            currentWebDAVItems = m_sqllite.getItemsChildren(m_webDAVView->getCurrentEntry().path);
        updateItems(currentWebDAVItems);

        if (currentWebDAVItems.empty())
        {
            Message(ICON_ERROR, TEXT_MESSAGE_ERROR, "Could not sync the items and there is no offline copy available.",
                    TIMEOUT_MESSAGE);
            HideHourglass();
            m_webDAVView->invertCurrentEntryColor();
        }
        else
        {
            drawWebDAVItems(currentWebDAVItems);
        }
        break;
    }
    }
}

int EventHandler::keyHandler(int p_type, int p_clicked_button, int par2)
{
    auto const clicked_button{static_cast<PocketbookButtons>(p_clicked_button)};
    if (m_webDAVView != nullptr)
    {
        if (p_type == EVT_KEYPRESS)
        {
            switch (clicked_button)
            {
            case PocketbookButtons::Menu:
                m_webDAVView->firstPage();
                break;
            case PocketbookButtons::Back:
                m_webDAVView->prevPage();
                break;
            case PocketbookButtons::Forward:
                m_webDAVView->nextPage();
                break;
            default:
                return 1;
            }
            return 0;
        }
    }
    else if (m_fileView != nullptr)
    {
        if (p_type == EVT_KEYPRESS)
        {
            switch (clicked_button)
            {
            case PocketbookButtons::Menu:
                m_fileView->firstPage();
                break;
            case PocketbookButtons::Back:
                m_fileView->prevPage();
                break;
            case PocketbookButtons::Forward:
                m_fileView->nextPage();
                break;
            default:
                return 1;
            }
            return 0;
        }
    }

    return 1;
}

void EventHandler::getLocalFileStructure(std::vector<WebDAVItem> &p_tempItems)
{
    std::string const localPath{p_tempItems.at(0).localPath + '/'};
    if (iv_access(localPath.c_str(), W_OK) == 0)
    {
        const auto currentFolder{FileBrowser::getFileStructure(localPath, true, false)};
        const auto storageLocationLength{m_fileHandler->getStorageLocation().length()};
        for (const FileItem &local : currentFolder)
        {
            auto p = find_if(p_tempItems.begin() + 1, p_tempItems.end(),
                             [&](const WebDAVItem &item) { return item.localPath.compare(local.path) == 0; });
            if (p == p_tempItems.end())
            {
                WebDAVItem temp;
                temp.localPath = local.path;
                temp.state = FileState::ILOCAL;
                temp.title = temp.localPath.substr(temp.localPath.find_last_of('/') + 1, temp.localPath.length());
                // Log::writeInfoLog(std::to_string(fs::file_size(entry)));
                temp.lastEditDate = local.lastEditDate;

                std::string directoryPath = temp.localPath;
                if (directoryPath.length() > storageLocationLength)
                {
                    directoryPath = directoryPath.substr(storageLocationLength + 1);
                }
                if (local.type == Type::FFOLDER)
                {
                    if (m_fileHandler->excludeFolder(directoryPath + "/"))
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
                    if (m_fileHandler->excludeFolder(directoryPath) || m_fileHandler->excludeFile(temp.title))
                    {
                        continue;
                    }
                }
                p_tempItems.push_back(temp);
            }
        }
    }
}

void EventHandler::downloadFolder(std::vector<WebDAVItem> &p_items, int p_itemID)
{
    auto &currentItem{p_items.at(p_itemID)};
    // Don't sync hidden files
    if (currentItem.hide == HideState::IHIDE)
    {
        return;
    }

    // BanSleep(2000);
    std::string path{currentItem.path};

    if (currentItem.type == Itemtype::IFOLDER)
    {
        std::vector<WebDAVItem> tempItems;
        switch (currentItem.state)
        {
        case FileState::IOUTSYNCED:
        case FileState::ICLOUD: {
            UpdateProgressbar(("Syncing folder" + path).c_str(), 0);
            iv_mkdir(currentItem.localPath.c_str(), 0777);
            tempItems = m_webDAV.getDataStructure(path);
            currentItem.state = FileState::IDOWNLOADED;
            m_sqllite.updateState(currentItem.path, currentItem.state);
            updateItems(tempItems);
            break;
        }
        case FileState::ISYNCED: {
            tempItems = m_sqllite.getItemsChildren(path);
            iv_mkdir(currentItem.localPath.c_str(), 0777);
            currentItem.state = FileState::IDOWNLOADED;
            m_sqllite.updateState(currentItem.path, currentItem.state);
            break;
        }
        case FileState::ILOCAL: {
            if (currentItem.localPath.length() > 3 &&
                currentItem.localPath.substr(currentItem.localPath.length() - 3).compare("sdr") == 0)
                Log::writeInfoLog("Ignoring koreader file " + currentItem.localPath);
            else
            {
                CloseProgressbar();

                auto const dialogResult{DialogSynchro(ICON_QUESTION, TEXT_MESSAGE_ACTION,
                                                      ("The folder " + currentItem.localPath +
                                                       " has been removed from the cloud. Do you want to delete it?")
                                                          .c_str(),
                                                      TEXT_DIALOG_YES, TEXT_DIALOG_NO, TEXT_DIALOG_CANCEL)};
                if (dialogResult == 1)
                {
                    fs::remove_all(currentItem.localPath);
                }
                else
                {
                    tempItems.push_back(currentItem);
                }

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
            {
                downloadFolder(tempItems, i);
            }
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
        switch (currentItem.state)
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
            Log::writeInfoLog("started download of " + currentItem.path + " to " + currentItem.localPath);
            if (m_webDAV.get(currentItem))
            {
                currentItem.state = FileState::ISYNCED;
                m_sqllite.updateState(currentItem.path, currentItem.state);
            }
            break;
        }
        // TODO unify
        case FileState::ILOCAL: {
            CloseProgressbar();
            const auto dialogResult{DialogSynchro(
                ICON_QUESTION, TEXT_MESSAGE_ACTION,
                ("The file " + currentItem.localPath + " has been removed from the cloud. Do you want to delete it?")
                    .c_str(),
                TEXT_DIALOG_YES, TEXT_DIALOG_NO, TEXT_DIALOG_CANCEL)};
            if (dialogResult == 1)
                fs::remove(currentItem.localPath);
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

    if (m_webDAVView->getCurrentEntry().type == Itemtype::IFILE)
    {
        Log::writeInfoLog("Started download of " + m_webDAVView->getCurrentEntry().path + " to " +
                          m_webDAVView->getCurrentEntry().localPath);
        if (m_webDAV.get(m_webDAVView->getCurrentEntry()))
        {
            m_webDAVView->getCurrentEntry().state = FileState::ISYNCED;
            m_sqllite.updateState(m_webDAVView->getCurrentEntry().path, m_webDAVView->getCurrentEntry().state);
        }
    }
    else
    {
        auto currentItems{m_sqllite.getItemsChildren(m_webDAVView->getCurrentEntry().path)};
        downloadFolder(currentItems, 0);
        m_webDAVView->getCurrentEntry().state = FileState::IDOWNLOADED;
        m_sqllite.updateState(m_webDAVView->getCurrentEntry().path, m_webDAVView->getCurrentEntry().state);
        UpdateProgressbar("Download completed", 100);
    }

    // TODO implement
    // Util::updatePBLibrary(15);
    CloseProgressbar();
    m_webDAVView->reDrawCurrentEntry();
}

bool EventHandler::checkIfIsDownloaded(std::vector<WebDAVItem> &p_items, int p_itemID)
{
    auto &currentItem{p_items.at(p_itemID)};

    if (iv_access(currentItem.localPath.c_str(), W_OK) != 0)
    {
        currentItem.state = currentItem.type == Itemtype::IFOLDER ? currentItem.state = FileState::ISYNCED
                                                                  : currentItem.state = FileState::IOUTSYNCED;
        m_sqllite.updateState(currentItem.path, currentItem.state);
        return false;
    }

    if (currentItem.type == Itemtype::IFOLDER)
    {
        if (currentItem.state != FileState::IDOWNLOADED)
            return false;
        auto tempItems{m_sqllite.getItemsChildren(currentItem.path)};
        // first item of the vector is the root path itself
        for (auto i = 1; i < tempItems.size(); i++)
        {
            if (!checkIfIsDownloaded(tempItems, i))
            {
                currentItem.state = FileState::ISYNCED;
                m_sqllite.updateState(currentItem.path, currentItem.state);
                return false;
            }
        }
    }
    return true;
}

void EventHandler::updateItems(std::vector<WebDAVItem> &p_items)
{
    for (auto &item : p_items)
    {
        // returns ICloud if is not found
        item.state = m_sqllite.getState(item.path);

        if (item.type == Itemtype::IFILE)
        {
            if (iv_access(item.localPath.c_str(), W_OK) != 0)
                item.state = FileState::ICLOUD;
            else
            {
                item.state = FileState::ISYNCED;
                if (m_sqllite.getEtag(item.path).compare(item.etag) != 0)
                    item.state = FileState::IOUTSYNCED;
            }
        }
        else
        {
            if (m_sqllite.getEtag(item.path).compare(item.etag) != 0)
                item.state = (item.state == FileState::ISYNCED || item.state == FileState::IDOWNLOADED)
                                 ? FileState::IOUTSYNCED
                                 : FileState::ICLOUD;
            if (item.state == FileState::IDOWNLOADED)
            {
                auto currentItems = m_sqllite.getItemsChildren(item.path);
                if (!checkIfIsDownloaded(currentItems, 0))
                    item.state = FileState::ISYNCED;
            }

            if (iv_access(item.localPath.c_str(), W_OK) != 0)
                iv_mkdir(item.localPath.c_str(), 0777);
        }
    }
    if (p_items.at(0).state != FileState::IDOWNLOADED)
        p_items.at(0).state = FileState::ISYNCED;
    m_sqllite.saveItemsChildren(p_items);

    // TODO sync delete when not parentPath existend --> "select * from metadata
    // where parentPath NOT IN (Select DISTINCT(parentPath) from metadata; what
    // happens with the entries below?
}

void EventHandler::drawWebDAVItems(std::vector<WebDAVItem> &p_items)
{
    if (!p_items.empty())
    {
        m_currentPath = p_items.at(0).path;
        getLocalFileStructure(p_items);
        m_webDAVView.reset(new WebDAVView(m_menu->getContentRect(), p_items, 1));
    }
}

void EventHandler::redrawItems()
{
    m_excludeFileView.reset(nullptr);
    ShowHourglassForce();

    FillAreaRect(&m_menu->getContentRect(), WHITE);
    if (m_currentPath.empty())
    {
        auto currentWebDAVItems{m_webDAV.getDataStructure(WebDAV::getRootPath(true))};
        updateItems(currentWebDAVItems);
        drawWebDAVItems(currentWebDAVItems);
    }
    else
    {
        const auto currentFolder{FileBrowser::getFileStructure(m_currentPath, false, true)};
        m_fileView.reset(new FileView(m_menu->getContentRect(), currentFolder, 1));
    }
}
