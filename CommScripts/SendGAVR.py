#!/usr/bin/python

###############################################################
## SendGAVR.py
## Author: Todd Sukolsky
## Copyright of Todd Sukolsky and Re.Cycle
## Date Created: 4/15/2013
## Last Revised: 4/15/2013
###############################################################
## Description:
##    This module is responisble for communication between the GAVR
## and the BeagleBone. only thing communicated is trip data. USB insert
## is handled by a GPIO pin.
###############################################################
## Revisions:	
##	4/15:  Moved code from commGAVR and ReceiveGAVR to accomodate
##			changes
###############################################################
## Changes to be made:
##     
###############################################################

import time
import sys
import os
import serial
import optparse
import glob
import subprocess


##OPTPARSER INFO
sendTrips=False

use = "Usage: %prog [options] <arguments>. "
parser = optparse.OptionParser(usage=use)
parser.add_option('-t',action="store_true",dest="trips",default=False,help='Send trips to GAVR')

(options, args) = parser.parse_args()

#Assume only one is going to be done.
if options.trips is True:
	sendTrips=True
else
	print "Nothing to do, type --help"
	exit(0)


##Useful Variables
serialPort='/dev/ttyO4'
baudRate=9600
USBpath='/dev/USB'
VirtualPath='/ReCycle/Trips/'
gpsPath='/ReCycle/gps/'

#### Send String Routine ####
def sendString(STRING):
	for char in STRING:
		#print "Writing " + char
		serPort.write(char)
		time.sleep(250.0/1000.0)    #25 is good, 10 is too fast, misses it sometimes...

#### Get String Routine ####
def getString(waitTime):
	time.sleep(waitTime)
	string=''
	string+=serPort.read(serPort.inWaiting())
	#print string
	return string

## Set port functions, then open
serPort = serial.Serial(
        port=serialPort,
        baudrate=baudRate,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
)

try:
	serPort.open()
except: 
	print "Error opening port" + serialPort
	exit()

### Start functionality here ###
#setup bools and variables
communicating=True
whichTrip=0
response=''
state=0
successful=True			#only needed in state 4, doesn't need to be assigned in states 0->3, reset in state 5
#Get the trip files "1.txt", "2.txt", etc...
path=USBpath+VirtualPath+"*.txt"
tripFiles=glob.glob(path)			#tripFiles now has all the file names

##Start state machine.
while communicating:
	if (state=0):
		#Send Interrupt to GAVR
		os.system("echo high > /sys/class/gpio/gpio44/direction")
		time.sleep(25.0/1000.0)
		os.system("echo low > /sys/class/gpio/gpio44/direction")
		state=1
	elif (state=1):
		#wait for 'A.'
		response=getString(200.0/1000.0)
		state=2
	elif (state=2):
		#If we got the 'A.', we got the right thing. Send trip data. OTherwise go to state 5 for Error.
		if (response=='A.'):
			state=3
		else:
			state=5
	elif (state=3):		
		##Sending Trips to him. Read the Start Days, line 4, and Start Year line 5
		for aFile in tripFiles:
			##open the file
			FILE=open(aFile,'r')
			lines=FILE.readlines()
			startDays='T'+lines[4][2:].rstrip().strip()			#take of first two characters,Strip new line, then white space. Add '.'
			startYears='T'+lines[5][2:].rstrip().strip()		#Same as ^^
			sendString(startDays)						#Send the string, wait for response
			response=getString(200.0/1000.0)
			if response=='A.':
				##One down, now send years
				sendString(startYears)
				response=getString(200.0/1000.0)
				if (response!='A.'):
					##iF there was an error, go to state 5 and wait. Otherwise, go to next file
					sendString("E.")
					state=5
					successful=False
					break
			else:
				##Error on first send, go to state 5 and wait, then try again
				sendString("E.")
				state=5
				successful=False
				break
			#Close the file
			FILE.close()	
		##Came out of for loop from eiether all files read or break, if successful, it was all files. Done
		if successful:
			state=4
	elif (state=4):
		##Successful transmission. Send 'D.' so GAVR knows that's it.
		sendString("D.")
		communicating=False
	elif (state=5):
		##Error. Wait for 10 seconds then try aggain. Reset state and successful
		time.sleep(10)
		state=0
		successful=True			##reset to allow for new communication
	else:
		communicating=False
		

exit()
		
		
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
