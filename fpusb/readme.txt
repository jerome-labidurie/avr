fpusb
-----

Périphérique USB pour controler un FilPilote de chauffage

licence
-------
GNU GPLv2 : http://www.gnu.org/licenses/gpl-2.0.html

firmware
--------
ATtiny45 firmware

-- main.c
full firmware
make extract_vusb # only once
make fuse         # program lfuse for 16MHz
make
make upload

host-libusb
-----------
Host tools to control the usb device via libusb 1.0

-- 50-vusb.rules 
Rule for udev. The device will belong to group plugdev.
Copy to /etc/udev/rules.d/ 
reload udev rules :
$ sudo udevadm control --reload-rules

usbtool
-------
usage of vusb-20100715/examples/usbtool

./usbtool control in vendor endpoint 1 1 0
while [ 1 ];do i=0;while [ $i -lt 256 ];do ./usbtool -v 0x16c0 -p 0x05dc control in vendor endpoint 1 $i 0;i=$(($i+1));sleep 1;done;done
