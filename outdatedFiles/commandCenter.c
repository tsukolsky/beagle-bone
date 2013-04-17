/*******************************************************************************\
| commandCenter.c
| Author: Todd Sukolsky
| Initial Build: 4/10/2013
| Last Revised: 4/12/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description: This module is responsible for calling all processes that interact with
|	the board and the USB drive on the BeagleBone. To start the GPS is creates 
| 	a new thread; when pinged by the GAVR it will take in the request, then
|	reply appropriately. If a shutdown is imminent, it will run a shutdown script
|	then execute the "halt" command.
|--------------------------------------------------------------------------------
| Revisions:
|	4/10: Initial build. Tried a test script, located in beagle-bone.git/TestScripts/commandTest.c
|	4/11: Working on USB functionality. Changed system calls to "execv" calls. Getting rid of syntax
|	      errors.
|	4/12: Added some bool functionality to eliminate repetitive calls. Might put fopen() in the while
|	      loop instead of before original. Transferring to BeagleBone for testing now.
|================================================================================
| *NOTES: (1) BONE_INT/GAVRO (GAVR request interrupt) is on P8,Pin 5=GPIO1_2
|	  (2) BONE_INT/WAVRO (Shutdown notification) is on P8, pin 6=GPIO1_3
|	  (3) WIO6/BB8_8 (Booted successfully notification for WAVR) is on P8, pin 8=GPIO2_3
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

void error(const char *msg);	
void *gpsThread(void *args);	//not needed, taken care of in initial script and any call of ReceiveGAVR
bool pShutdown(void);		//replaced with "waitingForShutdown.cpp"
bool pGAVRrequest(void);	//replaced with "waitForReceive.cpp"
bool pUSBupdate(void);		//not needed
bool pPinSetup(void);		//not needed, taken care of in initial script.

int pidPinSetup, pidShutdown, pidGAVRrequest,pidUSBupdate;
bool updatedUSB=false,calledShutdown=false,receivingGAVR=false;						//blocking bool to keep from calling update USB script over and over.	

int main(){
	//Declare normal variables.
	FILE *fGAVRrequestInt, *fShutdownInt, *fUSBinput;
	int GAVRrequestInt, ShutdownInt, USBinserted, ids=0;
	bool success=false,running=true;				//whether or not the script call was successful; whether we are running.

	//Declare P-Thread variables
	pthread_t newThread;
	pthread_attr_t attr;
	
	//Open GPIO files for reading.
	fGAVRrequestInt = fopen("/sys/class/gpio/gpio34/value","r");
	fShutdownInt = fopen("/sys/class/gpio/gpio66/value","r"); 
	fUSBinput = fopen("/home/root/Documents/tmp/.usbinfo","r");				//This will change.

	//Call pin setup
	success=pPinSetup();
	if (!success){error("Unable to execute pin setup.\n");exit(10);}

	//Create a new thread for the GPS daemon.
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_create(&newThread,&attr,gpsThread,&ids);
//	pthread_join(newThread):					//Do I need this?

	while (running){
		char buffer[4];						//buffer used for getting "0" or "1" in the files
		fread(buffer,1,1,fGAVRrequestInt);			//See if there is a request from the GAVR
		GAVRrequestInt=atoi(buffer);						
		bzero(buffer,4);					//zero the buffer to allow for next read.
		fread(buffer,1,1,fShutdownInt);				//see if there is a shutdown request
		ShutdownInt=atoi(buffer);				
		bzero(buffer,4);
		fread(buffer,1,1,fUSBinput);
		USBinserted=atoi(buffer);

		if (USBinserted && !GAVRrequestInt && !ShutdownInt && !updatedUSB){
			updatedUSB=true;
			printf("Alerting GAVR of USB inserted");
			//success=pUSBupdate();
		} else if (!USBinserted){updatedUSB=false;}					//this should only be sent low when a USB is detached.
		else;

		//If there was an interrupt for the GAVR, open that protocol
		if (GAVRrequestInt && !ShutdownInt && !receivingGAVR){
			receivingGAVR=true;
			printf("Got interrupt for GAVR request, going to new process.\n");
			//success=pGAVRrequest();
		} else if (!GAVRrequestInt){receivingGAVR=false;}
		else;

		if (ShutdownInt && !calledShutdown){
			calledShutdown=true;
			//Kill all processes, save things.
			success=pShutdown();
			printf("Shutdown process finished, halting...");
			//system("halt"); //shuts system down.
			return (1);
		}
	}//end while(running)
	return (0);
}//end main

/****************************************************************************/
void error(const char *msg){
	perror(msg);
	//exit(11);
}
/****************************************************************************/
bool pUSBupdate(void){
	pidUSBupdate=fork();
	if (pidUSBupdate==0){//child
		char *args[]={"/home/root/Documents/beagle-bone.git/CommScripts/SendGAVR","-s","USB.",0};
		execv(args[0],args);					//calls comm script to GAVR with string "USB."
		error("Unable to exec SendGAVR.");
		return false;
	} else if (pidUSBupdate<0){
		error("Error forking to a new process.");
		return false;
	} else {
		waitpid(pidUSBupdate,NULL,0);
	}
	return true;
}
/****************************************************************************/
bool pGAVRrequest(void){
	pidGAVRrequest=fork();
	if (pidGAVRrequest<0){
		error("Error starting \"ReceiveGAVR\" process.\n");
		return false;
	} else if (pidGAVRrequest==0){//child 
		char *args[]={"/home/root/Documents/beagle-bone.git/CommScripts/ReceiveGAVR",0};
		execv(args[0],args);
		error("Unable to exec ReceiveGAVR.");
		return false;
	} else { //Parent
		waitpid(pidGAVRrequest,0);
		receivingGAVR=false;					//Reset bool to allow for a new receive, if needed.
	}
	return true;
}
/****************************************************************************/
void *gpsThread(void *args){
	//IN a new thread, start that ./myGpsPipe wihc does everything
	char *args2[]={"/home/root/Documents/beagle-bone.git/NMEA/myGpsPipe",0};
	execv(args2[0],args2);
	pthread_exit(NULL);
}//end gpsThread.
/****************************************************************************/
bool pShutdown(void){
	//Save things...
	pidShutdown=fork();
	if (pidShutdown==0){//child
		char *args[]={"/home/root/Documents/beagle-bone.git/setupScripts/shutdownProtocol",0};
		execv(args[0],args);
		error("Unable to exec shutdownProtocol");
		return false;
	} else if (pidShutdown<0){
		error("Error starting shutdown process.\n");
		return false;
	} else {
		waitpid(pidShutdown,0);
	}
	return true;
}
/****************************************************************************/
bool pPinSetup(void){
	//Turn this into the correct process (pinSetup)
	pidPinSetup = fork();
	if (pidPinSetup==0){							//Child
		char *args[]={"/home/root/Documents/beagle-bone.git/setupScripts/pinSetup",0};
		execv(args[0],args);
		error("Unable to exec \"pinSetup\".");				//initialize the pins.
		return false;
	} else if (pidPinSetup<0){
		error("Error starting new process.\n");
		return false;
	} else { 			//Parent
		waitpid(pidPinSetup,0);	//Wait for child to finish.
	}
	return true;
}	
/****************************************************************************/
