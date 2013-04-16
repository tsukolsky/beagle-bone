/*******************************************************************************\
| waitForShutdown.cpp
| Author: Todd Sukolsky
| Initial Build: 4/15/2013
| Last Revised: 4/15/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description: This module is a spin-off of commandCenter.c. It is used to 
|	alert the BeagleBone that the GAVR is sending it something. Blocks on read.
|	When hit it halts everything.
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
	FILE *fShutdownInt;
	fShutdownInt = fopen("/sys/class/gpio/gpio66/value","r"); 
	while(true){s
		//Block for waiting
		while (/*Waiting for pin to go high*/);
	}//end while true
	return(1);
}
