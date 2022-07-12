#!/bin/bash

LOGFILE="/mnt/Pocketbook/system/config/nextcloud/logfile.txt"
APPLOCATION="bin/Nextcloud.app"
APPDIR="/mnt/Pocketbook/applications/"

############################################################
# Help                                                     #
############################################################
Help()
{
   # Display Help
   echo "Functions to interact with the PB"
   echo
   echo "Syntax: pb [h|c|l|u]"
   echo "options:"
   echo "-h     Print this Help."
   echo "-c     Copy the app to the appfolder"
   echo "-l     Show a logfile, if it exists"
   echo "-u     Unmount PB"
   echo
}

############################################################
# MOUNT                                                    #
############################################################
Mount()
{
MOUNTPOINT=$(sudo blkid | awk '/PB632/ {print $1;}' | tr -d ':')
if [ -n "$MOUNTPOINT" ]
then
    if mount | grep $MOUNTPOINT > /dev/null; then
        echo "Already mounted."
    else
        sudo mount $MOUNTPOINT /mnt/Pocketbook
    fi
else
    echo "Could not find PB"
fi
}

############################################################
############################################################
# Main program                                             #
############################################################
############################################################
[ $# -eq 0 ] && Help
while getopts ":hclu" o; do
    case $o in
        h) #Display help
            Help
            exit;;
        l)
            Mount
            if [ -f "$LOGFILE" ];
            then
                less $LOGFILE;
                while true; do
                    read -p "Do you want to delete log?" yn
                    case $yn in
                        [Yy]* ) sudo rm -r $LOGFILE; break;;
                        [Nn]* ) exit;;
                        * ) echo "Y/N";;
                    esac
                done
            else
                echo "No log file found."
            fi
            ;;
        c)
            Mount
            if [ -d "$APPDIR" ]
            then
                make
                sudo cp $APPLOCATION $APPDIR
            fi
            ;;
        u)
            if mount | grep /mnt/Pocketbook > /dev/null; then
                sudo umount /mnt/Pocketbook
                echo "Pocketbook umounted"
            else
                echo "/mnt/Pocketbook not mounted."
            fi
            ;;
        *)
            echo "Invalid command. To see available commands add -h."
    esac
done
