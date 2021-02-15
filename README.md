# Pocketbook Nextcloud Client
A basic client to access an existing nextcloud instance via Pocketbook. 

<img src="/screenshots/loginScreen.bmp" width="25%" height="25%">&nbsp;&nbsp;<img src="/screenshots/loginScreenURL.bmp" width="25%" height="25%">&nbsp;&nbsp;<img src="/screenshots/startScreen.bmp" width="25%" height="25%">&nbsp;&nbsp;<img src="/screenshots/menu.bmp" width="25%" height="25%">&nbsp;&nbsp;<img src="/screenshots/folderDialog.bmp" width="25%" height="25%">&nbsp;&nbsp;<img src="/screenshots/fileDialog.bmp" width="25%" height="25%">

## Features
* Display and navigate nextcloud file structure
* Download files to local storage system
* Sync folders to local storage system
* Remove files from local storage system
* Show local files that are saved but no longer available in the cloud
* Open epub, pdf, text, html, word and mobi with default ebook reader application
* Set Start folder to custom path
* Offline Modus

## Installation
Download and unzip the file from releases and place the nextcloud.app into the "applications" folder of your pocketbook. Once you disconnect the Pocketbook from the PC, the application should be visibile in the application launcher.

### Tested on
* Pocketbook Touch HD3 (PB623) 
* Pocketbook Touch Lux 5
* PocketBook Touch HD2 (PB631)
* Pocketboot Aqua 2 (PB641)

## Usage
To login type the servername (You can look up the WebDAV URL in the files app->seetings. (The nextcloud URL is the part till \"/remote.php...\".), Username and Password. You then will be redirected to the root file folder of your nextcloud instance.
To download a file, click on it. A synced file can be either opened, synced or removed.

### Known issues
* The main menu of the PB shows new books only if it turns into sleep modus. Otherwise the new books can only be opened from the nextcloud applicaton.

## Disclamer
Use as your own risk! 
Even though the possibility is really low, the application could harm your device or even break it.
