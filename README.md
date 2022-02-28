# Pocketbook Nextcloud Client
A basic client to access an existing nextcloud instance via Pocketbook. 

<img src="/screenshots/loginScreen.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/loginScreenURL.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/startScreen.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/menu.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/folderDialog.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/fileDialog.bmp" width="15%" height="15%">

## Features
* Display and navigate nextcloud file structure
* Download files to local storage system
* Sync folders to local storage system (long click on folders to open context menu for sync)
* Remove files from local storage system
* Show local files that are saved but no longer available in the cloud
* Open epub, pdf, text, html, word and mobi with default ebook reader application
* Set Start folder to custom path
* Offline Modus
* Navigaten via keys and touch

## Installation
Download and unzip the file from releases and place the nextcloud.app into the "applications" folder of your pocketbook. Once you disconnect the Pocketbook from the PC, the application should be visibile in the application launcher.

### Tested on
* Pocketbook Touch HD3 (PB623) 
* Pocketbook Touch Lux 5 (PB628)
* Pocketbook Touch HD2 (PB631)
* Pocketbook Aqua 2 (PB641)
* PocketBook InkPad 3 Pro (PB740)

## Usage
To login type the servername (e.g. https://domainname) or the WebDAV URL (e.g. htts://domainname/remote.php/dav/files/UUID) (You can look up the WebDAV URL in the files app->seetings.), Username and Password. You then will be redirected to the root file folder of your nextcloud instance.
To download a file, click on it. A synced file can be either opened, synced or removed.

## How to build

First you need to install the basic build tools for linux.

Then you have to download the Pocketbook SDK (https://github.com/pocketbook/SDK_6.3.0/tree/5.19).

In the CMakeLists.txt of this project you have to set the root of the TOOLCHAIN_PATH to the location where you saved the SDK. 
This could be for example:

`SET (TOOLCHAIN_PATH "../../SDK/SDK_6.3.0/SDK-B288")`

Then you have to setup cmake by:

`cmake .`

To build the application run:

`make` 

## Disclamer
Use as your own risk! 
Even though the possibility is really low, the application could harm your device or even break it.
