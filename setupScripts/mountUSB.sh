#!/bin/sh

# Description: This bash script searches for a known USB drive and checks to see if it is mounted. If it's not mounted, it will mount it.
# NOTE: You need to run this as root.
# Author: Ben Paolillo
# Date: 2/10/13

# Revision by Todd Sukolsky:4/16/13- Added things from Ben's other scripts to format this in one. Checks for usb, if inserted, raise GPIO and wait for eject. Once eject, lower GPIO and wait for insert.

clear


#Initialize LASTOUTPUT
LASTOUTPUT=''

while true
do
	##Lower GPIO line
	echo 0 > /sys/class/gpio/gpio/value
	echo "GPIO line low"

	# Looking for USB drives using the mount cmd.
	# Assuming the drive will be /dev/sda1 or /dev/sdb1 or /dev/sdc1
	while :
	do
		echo "Checking for USB insert"
		sleep 5
		command='dmesg | tail -f | grep 'sda\|sdb\|sdc' | awk '{print $2}' | cut -d "]" -f 1'
		IOUTPUT=`$command`
		if [[ -z "$IOUTPUT" ] && [ "$IOUTPUT" != "$LASTOUTPUT" ]]
		then
			echo "USB inserted"
			break
		else
			echo "No new USB"
	done
		
	LASTOUTPUT=$IOUTPUT

	MOUNTDIR=/media/USB

	#See which drive partition is was applied under, default is sda1
	if dmesg | grep -q "sdb1"; then
		USBNAME=/dev/sdb1
	elif dmesg | grep -q "sdc1"; then
		USBNAME=/dev/sdc1
	else
		USBNAME=/dev/sda1
	fi

	echo "Mounting $USBNAME to $MOUNTDIR..."
	
	# Creating the dir /media/USB (if it does not exist)
	if [ ! -d $MOUNTDIR ]; then
		mkdir -p $MOUNTDIR
	fi
	
	# Mount the drive
	mount -t vfat -o rw,users $USBNAME $MOUNTDIR
	
	#Raise GPIO line to GAVR
	echo 1 > /sys/class/gpio/gpio/value
	echo "GPIO line high."
	
	#Setup the files if they don't already exist
	if [ ! -d $TEMPDIR/ReCycle ]; then
		mkdir $MOUNTDIR/ReCycle
	fi
	if [ ! -d $MOUNTDIR/ReCycle/Trips ]; then
		mkdir $TEMPDIR/ReCycle/Trips
	fi
	if [ ! -d $MOUNTDIR/ReCycle/gps ]; then
		mkdir $MOUNTDIR/ReCycle/gps
	fi

	#Wait for unmount.
	while :
	do
		echo "Checking for unmount"
		sleep 3
		DOUTPUT=`dmesg | tail -f | grep "disconnect" | awk '{print $2}' | cut -d "]" -f 1`
		if [ -z "$DOUTPUT" ]
		then
			echo "USB still inserted, sleeping"
		else
			echo "USB disconnected"
			break
		fi
	done #Done with blocking wait for unmount.
	
done	#Done with outer loop that continuously checks.
		
exit	
	


