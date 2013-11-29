#!/bin/bash
# binary cpt

VUM=./cpusb
PIPE=/tmp/pipeload

$VUM -p pipe -w WRITE &

i=0
while [ $i -lt 256 ]
do 
   printf "\b\b\b%3d" $i
   echo $i > $PIPE
   i=$(($i+1))
   sleep .5
done

echo 0 > $PIPE
sleep 1
pkill $VUM
