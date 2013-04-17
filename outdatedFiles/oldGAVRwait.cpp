/*******************************************************************************\
| waitForGAVR.cpp
| Author: Todd Sukolsky
| Initial Build: 4/15/2013
| Last Revised: 4/16/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description: This module is a spin-off of commandCenter.c. It is used to 
|	alert the BeagleBone that the GAVR is sending it something. Blocks on read.
|	Forks to call ReceiveGAVR which will call SendGAVR
|--------------------------------------------------------------------------------
| Revisions: 4/15: Initial build/take.
|	     4/16: Added blocking. WHen opening value file, should block until rising edge
|		   occurs.
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
#include <sys/wait.h>
#include <fstream>

using namespace std;
void error(const char *msg);
void handleInterrupt(void);

int main(){
	//Setup files to wait for. Looking for interrupt from GAVR. Should be configured for "echo > rising" ?
	FILE *fGAVRrequestInt;
	ifstream gavrInterrupt;	
	while(true){
		char buffer[4];
		//fGAVRrequestInt = fopen("/sys/class/gpio/gpio34/value","r");
		//fread(buffer,1,1,fGAVRrequestInt);
		//cout << buffer << endl;
		gavrInterrupt.open("/sys/class/gpio/gpio34/value");				//reads as input since "i"fstream	
		gavrInterrupt.read(buffer,2);
		gavrInterrupt.close();
		cout << "Got interrupt from GAVR.\n" << endl;
		system("sleep 2");
		//handleInterrupt();
		fclose(fGAVRrequestInt);

	}//end while true
	return(1);
}

void error(const char *msg){
	perror(msg);
}

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
