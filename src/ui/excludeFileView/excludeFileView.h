//------------------------------------------------------------------
// excludeFileView.h
//
// Author:           RPJoshL
// Date:             03.10.2022
//
//-------------------------------------------------------------------

#include <string>

#ifndef EXCLUDEFILEVIEW
#define EXCLUDEFILEVIEW

using std::string;

#include "inkview.h"

#include <memory>
#include <string>

enum class ExcludeFileKeyboardTarget {
  IEXTENSIONS,
  IREGEX,
  IFOLDERREGEX,
  ISTARTFOLDER,
};

const int EXCLUDE_FILE_KEYBOARD_STRING_LENGHT = 90;

class ExcludeFileView {
public:
  /**
   * Draws the excludeFileView including an textfield with Extension list and
   * regex, checkbox for inverting the "excluding" and the save and cancel
   * button inside the contentRect
   *
   * @param contentRect area where the excludeFileView shall be drawn
   */
  ExcludeFileView(const irect &contentRect);
  ~ExcludeFileView();

  /**
   * Checks which part of the view is shown and reacts accordingly
   *
   * @param x x-coordinate
   * @param y y-coordinate
   * @return int if event has been handled. Returns 2 if save has been clicked
   * and all items are set, or -1 when cancel was fired
   */
  int excludeClicked(int x, int y);

  std::string getExtensionList() { return _extensionList; };
  std::string getRegex() { return _regex; };
  std::string getFolderRegex() { return _folderRegex; };
  std::string getStartFolder() { return _startFolder; };
  int getInvertMatch() { return _invertMatch; };

private:
  static ExcludeFileView *_excludeFileViewStatic;
  int _fontHeight;
  ifont *_font;
  const irect _contentRect;
  irect _saveButton;
  irect _cancelButton;
  irect _extensionListButton;
  irect _regexButton;
  irect _invertMatchButton;
  irect _folderRegexButton;
  irect _startFolderButton;
  ExcludeFileKeyboardTarget _target;
  std::string _extensionList;
  std::string _regex;
  std::string _folderRegex;
  std::string _startFolder;
  bool _invertMatch = false;
  std::string _temp;

  /**
   * Functions needed to call C function, handles the keyboard
   *
   * @param  text text that has been typed in by the user
   */
  static void keyboardHandlerStatic(char *text);

  /**
   * Called by the static method and saves and writes the input from the user to
   * the screen
   *
   * @param text text that has been typed in by the user
   */
  void keyboardHandler(char *text);
};
#endif