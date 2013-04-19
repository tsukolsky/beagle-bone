#!/bin/bash

$setupPath=/home/root/Documents/setupScripts
$gpsPath=/home/root/Documents/GPS

##Call pin setup, start GPS pipe
./$setupPath/pinSetup.sh
./$gpsPath/myGpsPipe.py
#./$usbPath/waitForUSB

##Start watchdog scripts.
./$setupPath/waitForGAVR
./$setupPath/waitForShutdown

echo "Finished setup" > /dev/kmsg
exit

