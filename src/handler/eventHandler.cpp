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

    if (iv_access(NEXTCLOUD_CONFIG_PATH.c_str(), W_OK) == 0)
    {
        //if (_nextcloud.login())
        //this one is always required --> if does not work -> say to the user that it did not work, to sync use
        //menubar
        //explanation on first login?
        //TODO here mark folders that are unsynced?
        //compare both datasets, if fromDB etag is different, mark as unsycned
        WebDAV test = WebDAV();
        _currentWebDAVItems = test.getDataStructure(Util::accessConfig(Action::IReadString,"UUID"));
        //vector<WebDAVItem> fromDB = _sqllite.getItemsChildren(_tempPath);
        if(_currentWebDAVItems.empty())
        {
            Message(ICON_ERROR, "Error", "Could not login, please try again.", 1200);
            _nextcloud.logout();
        }
        else
        {
            _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu.getContentRect(), _currentWebDAVItems,1));
            _sqllite.saveItemsChildren(_currentWebDAVItems);
            FullUpdate();
            //TODO avoid
            return;
        }
    }
    _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));
    FullUpdate();
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
    //offlineModus
    case 101:
    {
        if (_nextcloud.isWorkOffline())
        {
            if (Util::connectToNetwork())
            {
                _nextcloud.switchWorkOffline();
            }
            else
            {
                Message(ICON_WARNING, "Warning", "Could not connect to the internet.", 1200);
            }
        }
        else
        {
            _nextcloud.switchWorkOffline();
        }

        break;
    }
    //Make startfolder
    case 102:
    {
        _nextcloud.setStartFolder(_tempPath);
        Message(ICON_INFORMATION, "Info", ("On the next startup the folder" + _tempPath + " will be shown.").c_str(), 1200);

        break;
    }
    //Logout
    case 103:
    {
        int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Do you want to delete local files?", "Yes", "No", "Cancel");
        switch (dialogResult)
        {
        case 1:
            _nextcloud.logout(true);
            break;
        case 3:
            return;
        default:
            _nextcloud.logout();
            break;
        }
        _webDAVView.release();
        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));
        FullUpdate();
        break;
    }
    //Info
    case 104:
    {
        Message(ICON_INFORMATION, "Information", "Version 0.73 \n For support please open a ticket at https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client/issues", 1200);
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
    //invert color
    switch (index)
    {
    //Open
    case 101:
    {
        if (_nextcloud.getItems().at(_tempItemID).getType() == Itemtype::IFOLDER)
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
        if (_nextcloud.removeItem(_tempItemID))
        {
            updatePBLibrary();
            CloseProgressbar();
            _webDAVView->reDrawCurrentEntry();
        }
        else
        {
            CloseProgressbar();
            Message(ICON_WARNING, "Warning", "Could not delete the file, please try again.", 1200);
        }
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
            if (_tempItemID != -1)
            {
                if (_webDAVView->getCurrentEntry()->title.compare("...") != 0)
                {
                    _contextMenu = std::unique_ptr<ContextMenu>(new ContextMenu());
                    _contextMenu->createMenu(par2, _nextcloud.getItems().at(_tempItemID).getState(), EventHandler::contextMenuHandlerStatic);
                }
            }
        }
    }
    else if (type == EVT_POINTERUP)
    {
        //menu is clicked
        if (IsInRect(par1, par2, _menu.getMenuButtonRect()) == 1)
        {
            return _menu.createMenu(_nextcloud.isLoggedIn(), _nextcloud.isWorkOffline(), EventHandler::mainMenuHandlerStatic);
        }
        //if webDAVView is shown
        else if (_webDAVView != nullptr)
        {
            if(_webDAVView->checkIfEntryClicked(par1, par2))
            {
                _webDAVView->invertCurrentEntryColor();

                if (_webDAVView->getCurrentEntry()->type == Itemtype::IFOLDER)
                {
                    openFolder();
                }
                else
                {
                    if (_webDAVView->getCurrentEntry()->state == FileState::ISYNCED || (_nextcloud.isWorkOffline() && _nextcloud.getItems().at(_tempItemID).getState() == FileState::IOUTSYNCED))
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

                //TODO replace
                if (_nextcloud.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword()))
                {
                    _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu.getContentRect(), _currentWebDAVItems,1));
                    _loginView.reset();

                    FullUpdate();
                }
                else
                {
                    HideHourglass();
                    Log::writeLog("login failed.");
                }
                return 0;
            }
        }
    }
    return 1;
}

void EventHandler::updatePBLibrary()
{
    if (_nextcloud.getItems().at(_tempItemID).getType() == Itemtype::IFOLDER)
    {
        Util::updatePBLibrary(15);
    }
    else
    {
        if (_nextcloud.getItems().at(_tempItemID).isBook())
            Util::updatePBLibrary(5);
    }
}

void EventHandler::startDownload()
{
    OpenProgressbar(1, "Downloading...", "Checking network connection", 0, NULL);
    try
    {
        _nextcloud.download(_tempItemID);
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
    _nextcloud.getItems().at(_tempItemID).open();
}

void EventHandler::openFolder()
{
    FillAreaRect(_menu.getContentRect(), WHITE);
    ////TODO hourglass needed?
    ShowHourglassForce();

    _tempPath = _nextcloud.getItems().at(_tempItemID).getPath();
    if (!_tempPath.empty())
    {
        _nextcloud.setItems(_nextcloud.getDataStructure(_tempPath));
        //if folder is unsynced sync
        WebDAV test = WebDAV();
        vector<WebDAVItem> testitems = test.getDataStructure(_tempPath);
        _sqllite.saveItemsChildren(testitems);

        //if folder is synced, get only from DB
        vector<WebDAVItem> fromDB = _sqllite.getItemsChildren(_tempPath);
        //get etags from DB, if etag for path is unchanged, stays the same, same for foldersjj
    }
    _webDAVView.release();
    _webDAVView = std::unique_ptr<WebDAVView>(new WebDAVView(_menu.getContentRect(), _currentWebDAVItems,1));
    //_sqllite.saveItemsChildren(_nextcloud.getItems());
    //TODO include the header (where am i currently aka)
    //_webDAVView->drawHeader(_tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));
    PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);
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
