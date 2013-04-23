#!/bin/bash

$setupPath=/home/root/Documents/beagle-bone.git/setupScripts
$gpsPath=/home/root/Documents/beagle-bone.git/NMEA
$usbPath=/home/root/Documents/
##Call pin setup, start GPS pipe
./$setupPath/pinSetup.sh
./$gpsPath/myGpsPipe.py
./$setupPath/mountUSB.sh

##Start watchdog scripts.
./$setupPath/waitForGAVR
./$setupPath/waitForShutdown

echo "Finished setup" > /dev/kmsg
exit

