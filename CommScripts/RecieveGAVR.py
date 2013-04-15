#!/usr/bin/python

##Test script for GAVR
###############################################################
## ReceiveGAVR.py
## Author: Todd Sukolsky
## Copyright of Todd Sukolsky and Re.Cycle
## Date Created: 4/8/2013
## Last Revised: 4/15/2013
###############################################################
## Description:
##    This module is responisble for communication between the GAVR
## and the BeagleBone for Trip data and delete data's
###############################################################
## Revisions:	
##	4/8- Initial build.
##  4/15- Added startNewTripGPS, deleteGPStrip, changed name of
##		  deleteTrip to deleteUSBtrip. Now fork/execv functionality
##		  is done using arguments in tuple format. Does IO cahnging and
##		  moving when there is a delete. Can't delete current trip. Can't
## 		  offload the current trip.
###############################################################
## Changes to be made:
## 
###############################################################

import time
import sys
import os
import serial
import optparse
import psutil
import glob

serialPort='/dev/ttyO4'
baudRate=9600
USBpath='/dev/USB'
VirtualPath='/ReCycle/Trips/'
gpsPath='/ReCycle/gps/'
tripLocation=USBpath+VirtualPath
infoFileLocation=tripLocation + '.info'
boneGPSpath='/home/root/Documents/tmp/gps/'

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

def deleteUSBTrip(whichTrip):
	#Should do file IO and move them all down, as well as GPS data.
	rmCommand='rm '+tripLocation+str(whichTrip)+'.txt'
	os.system(rmCommand)
	rmCommand='rm '+USBpath+gpsPath+str(whichTrip)+'.txt'
	os.system(rmCommandd)
	#Move all the trips down
	
	#Replace number of trips in .info file
	
	
def deleteGPSTrip(whichTrip):
	rmCommand='rm '+boneGPSpath+str(whichTrip)+'.txt'
	os.system(rmCommand)
	#Now we need to shift all the files down one. ie, if there were 4 trips and we deleted 1, trip 4 needs to go to 3, 3->2, 2->1
	#See how many trips there are in file before the delete.
	file=boneGPSpath+'.info'
	IF=open(file,'r')
	lines=IF.readlines()
	howManyTrips=int(lines[0].rstrip().strip())		#.rstrip gets rid of \n, strip gets rid of whitespace on left and right
	#Number of trips we need to move down is howManyTrips-whichTrip
	counter=0
	for counter in range(whichTrip+1,howManyTrips+1): #if wt=2, hmt=4, moves 3->2, 4->3
		OLDF=boneGPSpath+str(counter)+'.txt'
		NEWF=boneGPSpath+str(counter-1)+'.txt'
		mvCommand='mv '+OLDF+' '+NEWF
		os.system(mvCommand)
		
	#Now that everything has been shifted down, rewreite how many GPS files are actually in this
	IF.close()
	OF=open(file,'w')
	OF.write(str(howManyTrips-1))
	OF.close()
		
		
def startNewTripGPS(whichTrip):
	##Need to halt myGpsPipe, then move "CURRENT.txt" into the current trip we are working on.
	#First must stop GPS pipe process
	PROCNAME="myGpsPipe"
	for proc in psutil.process_iter():
		if proc.name==PROCNAME:
			proc.kill()
	
	#Process is killed, need to move CURRENT.txt into <trip>.txt. The trip number we are putting to is whichTrip
	mvCommand="mv "+boneGPSpath+'CURRENT.txt'+boneGPSpath+str(whichTrip)+'.txt'
	os.system(mvCommand)
	file=boneGPSpath+'.info'
	OF=open(file,'w')
	OF.write(str(whichTrip))		#Which trip actually represents how many trips are there.
	OF.close()						

	#Restart myGpsPipe script.
	pid=os.fork()
	if pid==0:
		args=['/home/root/Documents/beagle-bone.git/NMEA/myGpsPipe','']
		os.execv(args[0],args)
		

##Function get's called when there is an interrupt from the GAVR. Need to reply with an "A." then get ready to receive
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
##Find out how many trips are in data.

readFile=open(infoFileLocation,'r')
lines=readFile.readlines()
tripNumber=int(lines[0][0])		##Grabs the first character/int on first line which is number of trips that we are looking at.
communicating=True
flagError=False
state=0
response=''
tripFilePath=tripLocation+str(tripNumber+1)+'.txt'		#newTripNumber=tripNumber+1
tripFile=open(tripFilePath,'w')

while communicating:
	if (state==0):
		sendString("A.")
		state=1
	elif (state==1):
		response = getString(200.0/1000.0)
		state=2
	elif (state==2): #Only time this sucker doesn't exit is when we are receiving Trip data.
		if (response=='E.'):
			print "Error receiving on port " + serialPort
			communicating=False
		elif (response=='NT.'):
			sendTrips=True								#Need to send them trips, start the transmission to GAVR script.
			sendString(response)						#Respond with appropriate ack
			communicating=False
		elif (response[0]=='D'):						#User wants to delete a trip on the USB, see which one then do it
			splitResponse=response.split('.')
			sendString(response)
			deleteUSBTrip(int(splitResponse[1:]))			#Call delete trips with the appropriate trip number
			communicating=False
		elif (response=='T.'):							#Sending us trip data.
			state=3
			sendString(response)						#ack back that we know the trip is coming.
		elif (response[0]=='D' and response[1]=='B'):	#Delete a GPS trip. File management needed.
			sendString(response)
			splitResponse=response.split('.')
			deleteGPStrip(int(splitResponse[2:]))
			communicating=False
		elif (response=='ST.'):							#Start a new GPS trip
			sendString(response)
			startNewTripGPS(tripNumber+1)					#send the trip number of the last one which is tripNumber+1
			communicating=False
		else:
			sendString("E.")
			communicating=False
	elif (state==3):
		response=getString(200.0/1000.0)
		state=4
	elif (state==4):
		if (response=='E.'):						#There was an error
			print "Error receiving trip on port " + serialPort
			communicating=False
			flagError=True
		elif (response=='D.'):						#We are done communicating
			communicating=False		
		else:
			sendString(response)
			strToFile=response+'\n'
		 	tripFile.write(strToFile)				#Writes the string into a file.
			state=3							#Go back to state 3 and get the next field
	else:
		print "Error processing state...out of bounds"

##end while communicating
tripFile.close()
readFile.close()
if state==4 and not flagError:
	#Put that there is 1 more file in the USB tirp files. This is used for number of GPS files as well.
	echoCommand="echo "+str(tripNumber+1)+" > " + infoFileLocation
	os.system(echoCommand)			#update the .info file	
	##Put the GPS file onto the USB
	mvCommand="mv /home/root/Documents/tmp/gps/"+str(tripNumber+1)+".txt "+USBpath+gpsPath
	os.system(mvCommand)
	
else:
	removeCommand="rm "+tripLocation+str(tripNumber+1)+".txt"
	os.system(removeCommand)					#if we didn't write anything to the file, make sure it isn't there.

	
	
##Now see if we need to send the trips, or delete something.
if sendTrips:
	pid2=os.fork()
	if pid2==0:		#child
		args=['/home/root/Documents/beagle-bone.git/CommScripts/commGAVR','-t','true','']
		os.execv(args[0],args)
		
		
exit()












