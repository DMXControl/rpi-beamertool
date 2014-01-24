#!/bin/bash

#check for root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root"
    exit 1
fi

#check for existing install
if [ -f /opt/rpi-beamertool/rpi-beamertool ]
then
    echo "rpi-beamertool is already installed"
    exit 1
fi

#update
aptitude update
aptitude -y upgrade

#install mediainfo
aptitude -y install mediainfo

#download and copy to /opt/rpi-beamertool
cd ~
wget http://rpi-beamertool.blog-srv.net/rpi_beamertool_0.3.2_installer.zip
unzip rpi_beamertool_0.3.2_installer.zip
rm rpi_beamertool_0.3.2_installer.zip
mv ~/rpi_beamertool_0.3.2 /opt/rpi-beamertool

cd /opt
chmod +x /opt/rpi-beamertool/rpi-beamertool
chmod +x /opt/rpi-beamertool/rpi-bt-install.sh
chmod +x /opt/rpi-beamertool/rpi-bt-uninstall.sh
chmod +x /opt/rpi-beamertool/rpi-bt-update.sh
chmod +x /opt/rpi-beamertool/rpi-bt-start.sh
chmod +x /opt/rpi-beamertool/rpi-bt-restart.sh
chmod 666 /opt/rpi-beamertool/config.txt
chmod 777 /opt/rpi-beamertool/textures
chmod 666 /opt/rpi-beamertool/textures/*.*

#install autostart
sed -i '/^exit 0/i \
#rpi-beamertool autostart\
/opt/rpi-beamertool/rpi-bt-start.sh &' /etc/rc.local

#install samba
install_samba=0
while true; do
    read -p "Install samba [y/n]?" yn
    case $yn in
        [YyJj]* ) install_samba=1; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done
if [ $install_samba == 1 ]; then
    aptitude -y install samba

    # create samba share
    echo "" >> /etc/samba/smb.conf
    echo "[rpi-beamertool]" >> /etc/samba/smb.conf
    echo "   read only = no" >> /etc/samba/smb.conf
    echo "   locking = no" >> /etc/samba/smb.conf
    echo "   path = /opt/rpi-beamertool" >> /etc/samba/smb.conf
    echo "   guest ok = yes" >> /etc/samba/smb.conf
    echo "" >> /etc/samba/smb.conf
fi

#install webinterface
install_web=0
while true; do
    read -p "Install webinterface [y/n]?" yn
    case $yn in
        [YyJj]* ) install_web=1; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done
if [ $install_web == 1 ]; then
    aptitude -y install apache2 php5 

    # enable systemfunctions for webinterface
    echo "" >> /etc/sudoers
    echo "www-data ALL=(ALL) NOPASSWD: /sbin/shutdown -r now" >> /etc/sudoers
    echo "www-data ALL=(ALL) NOPASSWD: /sbin/shutdown -h now" >> /etc/sudoers
    echo "www-data ALL=(ALL) NOPASSWD: /opt/rpi-beamertool/rpi-bt-restart.sh" >> /etc/sudoers

    # create webinterface
    rm -f /var/www/index.*
    mv /opt/rpi-beamertool/index.php /var/www
fi

#change hostname
echo "current hostname is:"
cat /etc/hostname
change_hostname=0
while true; do
    read -p "Change hostname to \"rpi-beamertool\" [y/n]?" yn
    case $yn in
        [YyJj]* ) change_hostname=1; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done
if [ $change_hostname == 1 ]; then
    echo "rpi-beamertool" > /etc/hostname
fi

#reboot
do_reboot=0
while true; do
    read -p "Reboot now [y/n]?" yn
    case $yn in
        [YyJj]* ) do_reboot=1; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes or no.";;
    esac
done
if [ $do_reboot == 1 ]; then
    reboot
fi

#delete install file
#cd $(dirname $0)
#rm rpi-bt-install.sh
