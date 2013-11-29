#!/bin/bash
# bash control of cpusb

# usbtool delivered with v-usb
USBTOOL=../firmware/vusb-20100715/examples/usbtool/usbtool

# last values from /proc/stat needed to compute cpu %
lUSER=0
lNICE=0
lSYST=0
lIDLE=0

# compute cpu usage since last call
# output : $PERC
cpuload()
{
   STAT=( $(head -1 /proc/stat) )
   PERC=$( echo " scale=2;tot=(${STAT[1]} - $lUSER + ${STAT[2]} - $lNICE + ${STAT[3]} - $lSYST); ( tot / ( tot + ${STAT[4]} - $lIDLE ) ) * 100 "|bc )
   lUSER=${STAT[1]}
   lNICE=${STAT[2]}
   lSYST=${STAT[3]}
   lIDLE=${STAT[4]}
} # cpuload

#### MAIN ####
cpuload
while [ 1 ]
do
   sleep .5
   cpuload
#    PERC=$(./cpuload -d 0|tail -1| cut -f1)
#    echo "scale=2; $PERC / 12.5" | bc
   # number of leds to light
   LED=$( echo "2^($PERC / 12.5)-1" | bc )
   echo "$PERC % --> $LED"
   $USBTOOL -v 0x16c0 -p 0x05dc control in vendor endpoint 2 $LED 0
done

