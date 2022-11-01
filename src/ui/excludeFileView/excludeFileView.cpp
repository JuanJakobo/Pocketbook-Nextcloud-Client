//------------------------------------------------------------------
// excludeFileView.cpp
//
// Author:           RPJoshL
// Date:             03.10.2022
//
//-------------------------------------------------------------------

#include "excludeFileView.h"
#include "log.h"
#include "util.h"

#include <regex>

using std::string;

ExcludeFileView *ExcludeFileView::_excludeFileViewStatic;
ExcludeFileView::ExcludeFileView(const irect &contentRect)
    : _contentRect(contentRect) {
  _excludeFileViewStatic = this;

  _extensionList = Util::getConfig<string>("ex_extensionList", "");
  _regex = Util::getConfig<string>("ex_pattern", "");
  _folderRegex = Util::getConfig<string>("ex_folderPattern", "");
  _startFolder = Util::getConfig<string>("ex_relativeRootPath", "");
  _invertMatch = Util::getConfig<int>("ex_invertMatch", 0);

  int contentHeight = contentRect.h / 2;
  int contentWidth = _contentRect.w * 0.9;
  int checkBoxWidth = _contentRect.w * 0.1;

  int beginY = 0.4 * contentHeight;
  int beginX = (_contentRect.w - contentWidth) / 2;

  int contents = contentHeight / 7;

  _fontHeight = contents / 2;

  _font = OpenFont("LiberationMono", _fontHeight, FONT_STD);
  SetFont(_font, BLACK);
  FillAreaRect(&_contentRect, WHITE);

  _extensionListButton =
      iRect(beginX, beginY, contentWidth, contents, ALIGN_CENTER);
  DrawTextRect(_extensionListButton.x,
               _extensionListButton.y - _fontHeight - _fontHeight / 2,
               _extensionListButton.w, _extensionListButton.h,
               "Extensions:", ALIGN_LEFT);
  DrawString(_extensionListButton.x, _extensionListButton.y,
             _extensionList.c_str());
  DrawRect(_extensionListButton.x - 1, _extensionListButton.y - 1,
           _extensionListButton.w + 2, _extensionListButton.h + 2, BLACK);

  _regexButton = iRect(beginX, beginY + 2 * contents, contentWidth, contents,
                       ALIGN_CENTER);
  DrawTextRect(_regexButton.x, _regexButton.y - _fontHeight - _fontHeight / 3,
               _regexButton.w, _regexButton.h, "File-Regex:", ALIGN_LEFT);
  DrawString(_regexButton.x, _regexButton.y, _regex.c_str());
  DrawRect(_regexButton.x - 1, _regexButton.y - 1, _regexButton.w + 2,
           _regexButton.h + 2, BLACK);

  _folderRegexButton = iRect(beginX, beginY + 4 * contents, contentWidth,
                             contents, ALIGN_CENTER);
  DrawTextRect(_folderRegexButton.x,
               _folderRegexButton.y - _fontHeight - _fontHeight / 3,
               _folderRegexButton.w, _folderRegexButton.h,
               "Folder-Regex:", ALIGN_LEFT);
  DrawString(_folderRegexButton.x, _folderRegexButton.y, _folderRegex.c_str());
  DrawRect(_folderRegexButton.x - 1, _folderRegexButton.y - 1,
           _folderRegexButton.w + 2, _folderRegexButton.h + 2, BLACK);

  _startFolderButton = iRect(beginX, beginY + 6 * contents, contentWidth,
                             contents, ALIGN_CENTER);
  DrawTextRect(_startFolderButton.x,
               _startFolderButton.y - _fontHeight - _fontHeight / 3,
               _startFolderButton.w, _startFolderButton.h,
               "Start folder:", ALIGN_LEFT);
  DrawString(_startFolderButton.x, _startFolderButton.y, _startFolder.c_str());
  DrawRect(_startFolderButton.x - 1, _startFolderButton.y - 1,
           _startFolderButton.w + 2, _startFolderButton.h + 2, BLACK);

  _invertMatchButton =
      iRect(_contentRect.w - 2 * checkBoxWidth, beginY + 8 * contents,
            checkBoxWidth, contents, ALIGN_CENTER);
  DrawTextRect(beginX, _invertMatchButton.y, contentWidth, _invertMatchButton.h,
               "Only include matching:", ALIGN_LEFT);
  DrawRect(_invertMatchButton.x - 1, _invertMatchButton.y - 1,
           _invertMatchButton.w + 2, _invertMatchButton.h + 2, BLACK);
  if (_invertMatch) {
    FillArea(_invertMatchButton.x - 1, _invertMatchButton.y - 1,
             _invertMatchButton.w + 2, _invertMatchButton.h + 2, BLACK);
  }

  _saveButton = iRect(beginX, beginY + 10 * contents, contentWidth / 2 - 20,
                      contents, ALIGN_CENTER);
  FillAreaRect(&_saveButton, BLACK);
  SetFont(_font, WHITE);
  DrawTextRect2(&_saveButton, "Save");
  PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);

  _cancelButton = iRect(beginX + contentWidth / 2, beginY + 10 * contents,
                        contentWidth / 2 - 20, contents, ALIGN_CENTER);
  FillAreaRect(&_cancelButton, BLACK);
  SetFont(_font, WHITE);
  DrawTextRect2(&_cancelButton, "Cancel");
  PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
}

ExcludeFileView::~ExcludeFileView() {
  CloseFont(_font);
  // Pointer cannot be freeded... It would crash when optining the window >~ 2
  // times (as well as an shared_ptr)
}

int ExcludeFileView::excludeClicked(int x, int y) {
  _temp = "";

  if (IsInRect(x, y, &_extensionListButton)) {
    _target = ExcludeFileKeyboardTarget::IEXTENSIONS;
    if (!_extensionList.empty())
      _temp = _extensionList;
    _temp.resize(EXCLUDE_FILE_KEYBOARD_STRING_LENGHT);
    OpenKeyboard("docx,pdf,sdr", &_temp[0],
                 EXCLUDE_FILE_KEYBOARD_STRING_LENGHT - 1, KBD_NORMAL,
                 &keyboardHandlerStatic);
    return 1;
  } else if (IsInRect(x, y, &_regexButton)) {
    _target = ExcludeFileKeyboardTarget::IREGEX;
    if (!_regex.empty())
      _temp = _regex;
    _temp.resize(EXCLUDE_FILE_KEYBOARD_STRING_LENGHT);
    OpenKeyboard("Pre.*Post\\.pdf", &_temp[0],
                 EXCLUDE_FILE_KEYBOARD_STRING_LENGHT, KBD_NORMAL,
                 &keyboardHandlerStatic);
    return 1;
  } else if (IsInRect(x, y, &_folderRegexButton)) {
    _target = ExcludeFileKeyboardTarget::IFOLDERREGEX;
    if (!_folderRegex.empty())
      _temp = _folderRegex;
    _temp.resize(EXCLUDE_FILE_KEYBOARD_STRING_LENGHT);
    OpenKeyboard("/root/.*/test/", &_temp[0],
                 EXCLUDE_FILE_KEYBOARD_STRING_LENGHT, KBD_NORMAL,
                 &keyboardHandlerStatic);
    return 1;
  } else if (IsInRect(x, y, &_startFolderButton)) {
    _target = ExcludeFileKeyboardTarget::ISTARTFOLDER;
    if (!_startFolder.empty())
      _temp = _startFolder;
    _temp.resize(EXCLUDE_FILE_KEYBOARD_STRING_LENGHT);
    OpenKeyboard("/MyBooks/", &_temp[0], EXCLUDE_FILE_KEYBOARD_STRING_LENGHT,
                 KBD_NORMAL, &keyboardHandlerStatic);
    return 1;
  } else if (IsInRect(x, y, &_invertMatchButton)) {
    _invertMatch = !_invertMatch;
    FillAreaRect(&_invertMatchButton, WHITE);
    if (_invertMatch)
      FillArea(_invertMatchButton.x - 1, _invertMatchButton.y - 1,
               _invertMatchButton.w + 2, _invertMatchButton.h + 2, BLACK);
    else
      DrawRect(_invertMatchButton.x - 1, _invertMatchButton.y - 1,
               _invertMatchButton.w + 2, _invertMatchButton.h + 2, BLACK);

    PartialUpdate(_invertMatchButton.x, _invertMatchButton.y,
                  _invertMatchButton.w, _invertMatchButton.h);

    return 1;
  } else if (IsInRect(x, y, &_saveButton)) {
    if (!_regex.empty()) {
      try {
        std::regex regP = std::regex(_regex);
      } catch (std::regex_error err) {
        Log::writeErrorLog("Unable to parse regex '" + _regex +
                           "': " + err.what());
        Message(ICON_ERROR, "Error", "Unable to parse the regex for the files.",
                1200);
        return 1;
      } catch (...) {
        Message(ICON_ERROR, "Error",
                "Unknown error occured while parsing the regex for the files.",
                1200);
        return 1;
      }
    }

    if (!_folderRegex.empty()) {
      try {
        std::regex regP = std::regex(_folderRegex);
      } catch (std::regex_error err) {
        Log::writeErrorLog("Unable to parse regex for folder '" + _folderRegex +
                           "': " + err.what());
        Message(ICON_ERROR, "Error",
                "Unable to parse the regex for the folders.", 1200);
        return 1;
      } catch (...) {
        Message(
            ICON_ERROR, "Error",
            "Unknown error occured while parsing the regex for the folders.",
            1200);
        return 1;
      }
    }

    FileHandler::update(_regex, _folderRegex, _extensionList, _invertMatch);

    return 3;
  } else if (IsInRect(x, y, &_cancelButton)) {
    return -1;
  }

  return 0;
}

void ExcludeFileView::keyboardHandlerStatic(char *text) {
  if (_excludeFileViewStatic != nullptr) {
    _excludeFileViewStatic->keyboardHandler(text);
  }
}

void ExcludeFileView::keyboardHandler(char *text) {
  if (!text)
    return;

  string s(text);
  // if (s.empty())
  //     return;

  if (_target == ExcludeFileKeyboardTarget::IEXTENSIONS) {
    _extensionList = s.c_str();
    FillAreaRect(&_extensionListButton, WHITE);
    DrawTextRect2(&_extensionListButton, s.c_str());
  } else if (_target == ExcludeFileKeyboardTarget::IREGEX) {
    _regex = s.c_str();
    FillAreaRect(&_regexButton, WHITE);
    DrawTextRect2(&_regexButton, s.c_str());
  } else if (_target == ExcludeFileKeyboardTarget::IFOLDERREGEX) {
    _folderRegex = s.c_str();
    FillAreaRect(&_folderRegexButton, WHITE);
    DrawTextRect2(&_folderRegexButton, s.c_str());
  } else if (_target == ExcludeFileKeyboardTarget::ISTARTFOLDER) {
    _startFolder = s.c_str();
    if (_startFolder.length() > 1 && _startFolder != "/") {
      if (_startFolder.substr(0, 1) != "/") {
        _startFolder = "/" + _startFolder;
      }
      if (_startFolder.substr(_startFolder.length() - 1) != "/") {
        _startFolder = _startFolder + "/";
      }
    }
    FillAreaRect(&_startFolderButton, WHITE);
    DrawTextRect2(&_startFolderButton, _startFolder.c_str());
  }
}
