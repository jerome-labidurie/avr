cpusb
-----

a small USB peripheral showing a vumeter on 8 leds

licence
-------
GNU GPLv2 : http://www.gnu.org/licenses/gpl-2.0.html

firmware
--------
ATtiny45 firmware

-- test_leds.c
simply test the 74hc4094 driver and led connections. No USB
make TARGET=test_leds
make upload

-- main.c
full firmware
make extract_vusb # only once
make fuse         # program lfuse for 16MHz
make
make upload

host-libusb
-----------
Host tools to control the usb device via libusb 1.0

-- cpusb.c
Gather data through plugins on a regular basis.Then display it on the usb device.
Plugins are dynamic libs (.so) and might be in the same directory.
The .c file must be named p_<plugin_name>.c

plugins interface should be :
/* initialization function, not mandatory */
void init(void);
/* gather value, return a percentage (0-100), mandatory */
float value(void);
/* cleaning function, not mandatory */
void quit (void);

See delivered plugins for examples:
- p_cpu.c: get cpus activity based on /proc/stat
- p_pulse.c: PulseAudio vumeter for default source (see DEVICE note at the beginning)
- p_pipe.c: gather data from a named pipe. So can be interfaced with any shell script

usage: cpusb [-d N] [-vh] -p plugin
   -d : wait N millisec between each display (default:500)
   -p : load plugin (p_plugin.so)
   -v : increase verbosity

examples:
$ cpusb -d 100 -p pulse
$ cpusb -p pipe
$ echo 68.3 > /tmp/pipeload


-- 50-vusb.rules 
Rule for udev. The device will belong to group plugdev.
Copy to /etc/udev/rules.d/ 
reload udev rules :
$ sudo udevadm control --reload-rules

-- bincpt.sh
Shell script to do a binary counter
Use cpusb in WRITE mode with plugin pipe

-- cpusb.sh
Simple shell script to gather cpu activity and send it to usb device.
Based on usbtool from libusb. (see below)

usbtool
-------
usage of vusb-20100715/examples/usbtool

./usbtool control in vendor endpoint 1 1 0
while [ 1 ];do i=0;while [ $i -lt 256 ];do ./usbtool -v 0x16c0 -p 0x05dc control in vendor endpoint 1 $i 0;i=$(($i+1));sleep 1;done;done
