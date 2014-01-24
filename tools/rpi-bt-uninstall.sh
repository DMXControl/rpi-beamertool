#!/bin/bash

#check for root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

#unistall autostart
sed -i '/#rpi-beamertool/d' /etc/rc.local
sed -i '/\/\opt\/rpi-beamertool\//d' /etc/rc.local

#delete files
rm -r /opt/rpi-beamertool


echo "The uninstaller only deletes autorun and the rpi-beamertool itself."
echo "The folowing programms are still installed after this uninstaller:"
echo "mediainfo"
echo "samba"
echo "apache2"
echo "php5"
echo ""
echo ""
