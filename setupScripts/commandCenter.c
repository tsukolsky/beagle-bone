/*******************************************************************************\
| commandCenter.c
| Author: Todd Sukolsky
| Initial Build: 4/10/2013
| Last Revised: 4/10/2013
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
void *gpsThread(void *args);
bool pShutdown(void);
bool pGAVRrequest(void);
bool pUSBupdate();

int pidPinSetup, pidShutdown, pidGAVRrequest;

int main(){
	FILE *fGAVRrequestInt, *fShutdownInt, *fUSBinput;
	int GAVRrequestInt, ShutdownInt, ids=0;
	bool success=false,running=true;
	pthread_attr_t attr;
	
	//Open GPIO files for reading.
	fGAVRrequestInt = fopen("/sys/class/gpio/gpio34/value");
	fShutdownInt = fopen("/sys/class/gpio/gpio66/value"); 
	fUSBinput = fopen("/");

	//Call pin setup
	success=pPinSetup(void);
	if (!success){error("Unable to execute pin setup.\n");exit(10);}

	//Create a new thread for the GPS daemon.
	pthread_t newThread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_create(&newThread,&attr,gpsThread,&ids);

	while (running){
		char buffer[4];
		fread(buffer,1,1,fGAVRrequestInt);
		GAVRrequestInt=atoi(buffer);
		bzero(buffer,4);
		fread(buffer,1,1,fShutdownInt);
		ShutdownInt=atoi(buffer);

		if (USBinserted && !GAVRrequestInt && !ShutdownInt){
			printf("Alerting GAVR of USB inserted");
			success=pUSBupdate();
		}
		//If there was an interrupt for the GAVR, open that protocol
		if (GAVRrequestInt && !ShutdownInt){
			printf("Got interrupt for GAVR request , going to new process.\n");
			success=pGAVRrequest();
		}
		if (ShutdownInt){
			//Kill all processes, save things.
			success=pShutdown(void);
			printf("Shutdown process finished, halting...");
			system("halt"); //shuts system down.
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
bool pUSBupdate(){}
/****************************************************************************/
bool pGAVRrequest(void){
	pidCommGAVR=fork();
	if (pidCommGAVR<0){
		error("Error starting \"ReceiveGAVR\" process.\n");
		return false;
	} else if (pidReceiveGAVR==0){//child 
		system("/home/root/Documents/beagle-bone.git/CommScripts/ReceiveGAVR");
	} else { //Parent
		waitpid(pidReceiveGAVR,0);
	}
	return true;
}
/****************************************************************************/
void *gpsThread(void *args){
	//IN a new thread, start that ./myGpsPipe wihc does everything
	system("/home/root/Documents/beagle-bone.git/NMEA/myGpsPipe");
	pthread_exit(NULL);
}//end gpsThread.
/****************************************************************************/
bool pShutdown(void){
	//Save things...
	pidShutdown=fork();
	if (pidShutdown==0){//child
		system("/home/root/Documents/beagle-bone.git/setupScripts/shutdownProtocol");
	} else if (pidShutdown<0){
		error("Error starting shutdown process.\n");
		return false;
	} else {
		waitpid(pidShutdown,0);
	}
	return true;
}
/****************************************************************************/
void pPinSetup(void){
	//Turn this into the correct process (pinSetup)
	pidPinSetup = fork();
	if (pidPinSetup==0){			//Child
		system("/home/root/Documents/beagle-bone.git/setupScripts/pinSetup");	//initialize the pins.
	} else if (pidPinSetup<0){
		error("Error starting new process.\n");
		return false;
	} else { 			//Parent
		waitpid(pidPinSetup,0);	//Wait for child to finish.
	}
	return true;
}	
/****************************************************************************/
