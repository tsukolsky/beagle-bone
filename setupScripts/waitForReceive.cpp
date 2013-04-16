/*******************************************************************************\
| waitForReceive.cpp
| Author: Todd Sukolsky
| Initial Build: 4/15/2013
| Last Revised: 4/15/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description: This module is a spin-off of commandCenter.c. It is used to 
|	alert the BeagleBone that the GAVR is sending it something. Blocks on read.
|	Forks to call ReceiveGAVR which will call SendGAVR
|--------------------------------------------------------------------------------
| Revisions: 4/15: Initial build/take.
|================================================================================
| *NOTES:
\*******************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

using namespace std;

int main(){
	//Setup files to wait for. Looking for interrupt from GAVR. Should be configured for "echo > rising" ?
	FILE *fGAVRrequestInt;
	fGAVRrequestInt = fopen("/sys/class/gpio/gpio34/value","r");		
	while(true){
		//Block for waiting
		while (/*Waiting for pin to go high*/);
		int pid2;
		pid2=fork();
		if (pid2<0){
			error("Error starting \"RecieveGAVR\" process.\n");
		} else if (pid2==0){//child process	
			char *args[]={"/home/root/Documents/beagle-bone.git/CommScripts/ReceiveGAVR",0};
			execv(args[0],args);
			error("Unable to exec ReceiveGAVR.");
		} else { //Parent
			waitpid(pidGAVRrequest,0);
			receivingGAVR=false;					//Reset bool to allow for a new receive, if needed.
		}//end of fork, go back to blocking
	}//end while true
	return(1);
}
