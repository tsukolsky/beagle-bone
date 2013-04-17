/*******************************************************************************\
| waitForGAVR.c
| Author: Todd Sukolsky
| Initial Build: 4/16/2013
| Last Revised: 4/16/2013
|================================================================================
| Description: This module is a spin-off of commandCenter.c. It is used to 
|	alert the BeagleBone that the GAVR is sending it something. Blocks on read.
|	Forks to call ReceiveGAVR which will call SendGAVR
|--------------------------------------------------------------------------------
| Revisions:
|================================================================================
| *NOTES:Polling example found at: http://bwgz57.wordpress.com/tag/beaglebone/
\*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include "pollingExtras.h"

void error(const char *msg);
void handleInterrupt(void);

int main(){
	//Declare a file descriptor and open the file we need as read only.
	int fd;
	fd = open("/sys/class/gpio/gpio34/value", O_RDONLY);		//this is the file for GAVR request to come in.
	struct pollfd pfd;
	//Intiailize polling file as the file descriptor	
	pfd.fd = fd;
	pfd.events = POLLPRI;
	pfd.revents = 0;
	//Lead is what the current pin is set to, ready is how many times this guy has been activated.
	int lead, ready;
	while (1) {
		int ready = poll(&pfd, 1, -1);
		printf("ready: %d\n", ready);
		if (pfd.revents != 0 /*&& (lead=get_lead(fd)) == 1*/) {	//if an event happened and the pin is a 1, that means we got an interrupt from GAVR
			printf("\tThis happend: %d\n",pfd.revents);
			lead=get_lead(fd);
		}
		printf("\t\t A: %d\n", lead);
		//handleInterrupt();
	}
	return 0;
}
/****************************
error:
****************************/
void error(const char *msg){
	perror(msg);
}

/****************************
handleInterrupt()
****************************/
void handleInterrupt(){
	int pid2;
	pid2=fork();
	if (pid2<0){
		error("Error starting \"RecieveGAVR\" process.\n");
	} else if (pid2==0){//child process	
		char *args[]={"/home/root/Documents/beagle-bone.git/CommScripts/ReceiveGAVR",0};
		execv(args[0],args);
		error("Unable to exec ReceiveGAVR.");
	} else { //Parent
		waitpid(pid2,NULL,0);
	}//end of fork, go back to blocking
}
