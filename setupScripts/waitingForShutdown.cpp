/*******************************************************************************\
| waitForShutdown.cpp
| Author: Todd Sukolsky
| Initial Build: 4/15/2013
| Last Revised: 4/16/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description: This module is a spin-off of commandCenter.c. It is used to 
|	alert the BeagleBone that the GAVR is sending it something. Blocks on read.
|	When hit it halts everything.
|--------------------------------------------------------------------------------
| Revisions: 4/15: Initial build/take.
|	     4/16: Added blocking. Should block when there we open the file, as long as it's set to rising > edge
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
#include <fstream>
#include <sys/wait.h>

using namespace std;

int main(){
	//Setup files to wait for. Looking for interrupt from GAVR. Should be configured for "echo > rising" ?
	//FILE *fShutdownInt;
	ifstream shutdownInt;
	//If the edge was set for Rising Edge, will block after open until a new value occurs.
	//fShutdownInt = fopen("/sys/class/gpio/gpio66/value"); 
	shutdownInt.open("/sys/class/gpio/gpio66/value");
	shutdownInt.close();	
	cout << "Got an interrupt\n";
	system("sleep 5");
	//execvp("halt",(char *)NULL);
	return(1);
}
