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

int pidPinSetup, pidShutdown, pidGAVRrequest;

int main(){
	FILE *fGAVRrequestInt, *fShutdownInt, *fUSBinput;
	int GAVRrequestInt, ShutdownInt, ids=0;
	bool success=false,running=true;
	pthread_attr_t attr;
	
	//Open files for reading.
	fGAVRrequestInt = File.open("/sys/class/gpio/gpioX/value");
	fShutdownInt = File.open("/sys/class/gpio/gpioY/value"); 

	//Call pin setup
	success=pPinSetup(void);
	if (!success){error("Unable to execute pin setup.\n");exit(10);}

	//Create a new thread for the GPS daemon.
	pthread_t newThread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_create(&newThread,&attr,gpsThread,&ids);

	while (running){
		GAVRrequestInt=fGAVRrequestInt.readbyte();
		ShutdownInt=fShutdownInt.readbyte();

		//If there was an interrupt for the GAVR, open that protocol
		if (GAVRrequestInt && !ShutdownInt){
			printf("Got interrupt for GAVR request , going to new process.\n");
			success=pGAVRrequest(void);
		}
		if (ShutdownInt){
			//Kill all processes, save things.
			success=pShutdown(void);
			printf("Shutdown process finished, halting...");
			execvp("halt",(char *)NULL); //shuts system down.
			return (1);
		}
	}//end while(running)
	return (0);
}//end main



void error(const char *msg){
	perror(msg);
	//exit(11);
}


bool pGAVRrequest(void){
	pidCommGAVR=fork();
	if (pidCommGAVR<0){
		error("Error starting \"ReceiveGAVR\" process.\n");
		return false;
	} else if (pidReceiveGAVR==0){//child 
		execvp("./home/root/Documents/beagle-bone.git/CommScripts/ReceiveGAVR");
	} else { //Parent
		waitpid(pidReceiveGAVR,0);
	}
	return true;
}

void *gpsThread(void *args){
	//IN a new thread, start that ./myGpsPipe wihc does everything
	execvp("./home/root/Documents/beagle-bone.git/NMEA/myGpsPipe",(char *)NULL);
	pthread_exit(NULL);
}//end gpsThread.

bool pShutdown(void){
	//Save things...
	pidShutdown=fork();
	if (pidShutdown==0){//child
		execvp(".//home/root/Documents/beagle-bone.git/shutdownProtocol",(char *)NULL);
	} else if (pidShutdown<0){
		error("Error starting shutdown process.\n");
		return false;
	} else {
		waitpid(pidShutdown,0);
	}
	return true;
}

void pPinSetup(void){
	//Turn this into the correct process (pinSetup)
	pidPinSetup = fork();
	if (pidPinSetup==0){			//Child
		execvp(".//home/root/Documents/beagle-bone.git/initialSetupScripts/pinSetup",(char *)NULL);	//initialize the pins.
	} else if (pidPinSetup<0){
		error("Error starting new process.\n");
		return false;
	} else { 			//Parent
		waitpid(pidPinSetup,0);	//Wait for child to finish.
	}
	return true;
}	
