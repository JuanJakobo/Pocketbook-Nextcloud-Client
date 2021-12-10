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

## Known Errors
### Let's Encrypt root CA isn't working properly
Due to the Expiration of DST Root CA X3 in September 2021 (https://letsencrypt.org/docs/dst-root-ca-x3-expiration-september-2021/) and the fact that the OpenSSL Version of the PB is rather old (1.0.2) (https://www.openssl.org/blog/blog/2021/09/13/LetsEncryptRootCertExpire/) some adjustments by the user have to be made.
#### 1. Copy an new pem file to your PB
Download an export from the newest CA certifictes from Mozilla https://curl.se/docs/caextract.html "cacert.pem" and place it inside the applications folder of your Pocketbook.

#### 2. Update your Server
In some cases you furthermore have to update certbot on your server.(https://github.com/electron/electron/issues/31212#issuecomment-933721249)

##### Update the certbot to the last version using snap (certbot instructions)
```
$ sudo snap install core
$ sudo snap refresh core
$ sudo apt-get remove certbot
$ sudo snap install --classic certbot
$ sudo ln -s /snap/bin/certbot /usr/bin/certbot
```
 ##### Renew & replace the certificate
```
certbot certonly --apache -d www.yourdomain.com --preferred-chain "ISRG Root X1"
service apache2 reload
```
To verify if the changes were applied successfully please run
```
openssl s_client -showcerts -connect <HOSTNAME>:443 | grep Root
```

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
