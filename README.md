# Pocketbook Nextcloud Client
A basic client to access an existing nextcloud instance via Pocketbook.

<img src="/screenshots/loginScreen.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/chooseStorageLocation.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/startScreen.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/menu.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/folderDialog.bmp" width="15%" height="15%">&nbsp;&nbsp;<img src="/screenshots/fileDialog.bmp" width="15%" height="15%">

## Features
* Display and navigate nextcloud file structure
* Download files to local storage system
* Sync folders to local storage system (long click on folders to open context menu for sync)
* Remove files from local storage system
* Show local files that are saved but no longer available in the cloud
* Open epub, pdf, text, html, word and mobi with default ebook reader application
* Offline Modus
* Navigaten via keys and touch

## Installation

1. **Download and Unzip:**
   - Download the desired release from the repository's releases section.
   - Unzip the downloaded file.

2. **Placement:**
   - Locate the `nextcloud.app` file from the unzipped folder.

3. **Transfer to Pocketbook:**
   - Connect your Pocketbook device to your PC.
   - Navigate to the "applications" folder on your Pocketbook.

4. **Move the App and Add `cacert.pem`:**
   - Transfer the `nextcloud.app` file into the "applications" folder of your Pocketbook device.
   - Additionally, download `cacert.pem` from [here](https://curl.se/ca/cacert.pem) and add it to the "applications" folder to ensure secure communication within the application.

5. **Disconnect and Launch:**
   - Safely disconnect your Pocketbook from the PC.
   - The application should now be visible in the application launcher of your Pocketbook.

### Tested on
* PocketBook Toch HD
* PocketBook Touch HD 2 (PB631)
* PocketBook Touch HD 3 (PB623)
* PocketBook Touch Lux 3 (PB626)
* PocketBook Touch Lux 5 (PB628)
* PocketBook Aqua 2 (PB641)
* PocketBook InkPad 3 Pro (PB740)
* Pocketbook Era (PB700)
* Pocketbook Inkpad Color 2 (PB743)

## Usage

To log in, follow these steps:

1. Enter the server name (e.g., `https://domainname`) or the WebDAV URL (e.g., `https://domainname/remote.php/dav/files/UUID`). You can find the WebDAV URL in the Files app under settings.
   
2. Provide your username and password. If you have 2FA (Two-Factor Authentication) enabled, you need to set up an App-specific password. You can learn more about this process [here](https://docs.nextcloud.com/server/latest/user_manual/en/user_2fa.html#using-client-applications-with-two-factor-authentication).

After logging in, you'll be prompted to select a location to save the Nextcloud files. 

- To download a file, simply click on it.
  
- To sync a folder, click on it until a menu appears. Then, select "sync" from the menu. The folder sync feature will only synchronize files that are "newer" on the server side, ignoring `.sdr` files.

## How to build
Currently there are multiple methods to build the application. It is recommended to use the Dockerimage.

### Development enviroments

#### Dockerimage
The easiest way to build applications as of now to use the Dockerimage. (https://github.com/JuanJakobo/Pocketbook-dev-docker)

#### Setup Toolchain
First you need to install conan and the basic build tools for linux.

Then you have to download the Pocketbook SDK (https://github.com/pocketbook/SDK_6.3.0/tree/5.19).

In the CMakeLists.txt of this project you have to set the root of the TOOLCHAIN_PATH to the location where you saved the SDK.

First you have to create the default profile by:
`conan profile detect`
Furthmore you have to add an conan profile for pocketbook to the profiles directory:

```
[settings]
arch=armv7
build_type=Release
compiler=clang
compiler.cppstd=gnu17
compiler.version=7.0
os=Linux

[buildenv]
CXX=arm-obreey-linux-gnueabi-clang++
CC=arm-obreey-linux-gnueabi-clang
LD=arm-obreey-linux-gnueabi-ld
STRIP=arm-obreey-linux-gnueabi-strip
AR=arm-obreey-linux-gnueabi-ar
AS=arm-obreey-linux-gnueabi-as
NM=arm-obreey-linux-gnueabi-nm
RANLIB=arm-obreey-linux-gnueabi-ranlib
```

### Build the code
On the first run you have to setup conan by:

`conan install . --build=missing -pr=pocketbook`

`cmake --preset conan-release`

After this to rebuild the application you have to run:

`cmake --build build/Release`

## Disclamer
Use as your own risk!
Even though the possibility is really low, the application could harm your device or even break it.
