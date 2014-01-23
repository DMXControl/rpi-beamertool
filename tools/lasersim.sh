#!/bin/bash

# Download Win32DiskImager: http://sourceforge.net/projects/win32diskimager
# Download Raspbian “wheezy” Image: http://www.raspberrypi.org/downloads
# Write Image to SD-Card
#
# Login via SSH (e.g. Putty) http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html
# username: pi
# password: raspberry
#
# run following commands:
# sudo raspi-config
# -> Expand Filesystem
# -> Advanced Options -> Memory Split -> 256
#
# run following commands:
# wget http://beamertool.ath.cx/lasersim.sh
# sudo bash lasersim.sh
#
# after running this script the raspberry pi will reboot and autostart lasersim
# in browser you should be able to open http://lasersim
# in windows network you should see  lasersim -> edit \\lasersim\lasersim\config.txt

# update system
aptitude update
aptitude -y upgrade
aptitude -y install samba apache2 php5 

# create samba share
echo "" >> /etc/samba/smb.conf
echo "[lasersim]" >> /etc/samba/smb.conf
echo "   read only = no" >> /etc/samba/smb.conf
echo "   locking = no" >> /etc/samba/smb.conf
echo "   path = /opt/lasersim" >> /etc/samba/smb.conf
echo "   guest ok = yes" >> /etc/samba/smb.conf
echo "" >> /etc/samba/smb.conf

# enable systemfunctions for webinterface
echo "" >> /etc/sudoers
echo "www-data ALL=(ALL) NOPASSWD: /sbin/shutdown -r now" >> /etc/sudoers
echo "www-data ALL=(ALL) NOPASSWD: /sbin/shutdown -h now" >> /etc/sudoers
echo "www-data ALL=(ALL) NOPASSWD: /usr/local/bin/lasersim" >> /etc/sudoers


cd /opt
wget http://beamertool.ath.cx/lasersim.tar.gz
tar xvfz lasersim.tar.gz

# create webinterface
rm -f /var/www/index.*
mv lasersim/index.php /var/www
mv lasersim/lasersim /usr/local/bin
chmod +x /usr/local/bin/lasersim

# set filesystem access rights
chmod 666 lasersim/config.txt
chmod 777 lasersim/textures
chmod 666 lasersim/textures/*.*

# autostart
sed "s/1:2345:respawn:\/sbin\/getty --noclear 38400 tty1/1:2345:respawn:\/bin\/login -f pi tty1 \<\/dev\/tty1\>dev\/tty1 2\>\&1/g" /etc/inittab > /etc/inittab.2
mv /etc/inittab.2 /etc/inittab

echo "clear" >> /home/pi/.bashrc
echo "PS1='${debian_chroot:+($debian_chroot)}'" >> /home/pi/.bashrc
echo "sudo lasersim --silent" >> /home/pi/.bashrc


# change hostname
echo "lasersim" > /etc/hostname

# reboot
reboot