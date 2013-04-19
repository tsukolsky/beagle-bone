#!/bin/sh

#Copyright (c) 2013 Todd Sukolsky
# All rights reserved.
#
#Author: Todd Sukolsky, 2013
#
# /etc/init.d/bar
#   and its symbolic link
# /usr/sbin/firstCall
#

### BEGIN INIT INFO
# Provides: 		firstCall
# Required-Start:	$all
# Required-Stop:
# Default-Start:	1 2 3 
# Default-Stop:		0 4 5 6
# Short-Description: firstCall daemon, provides system intefaces
# Description: This daemon is a service that initializes all
#	things used to work on the ReCycle system.
#	All services must start before this is called.
#
#
### END INIT INFO

#Check for the missing binaries
firstCall_BIN=/usr/bin/firstCall
test -x $firstCall_BIN || { echo "$firstCall_BIN not installed" > /dev/kmsg;
	if [ "$1" = "stop" ]; then exit 0;
	else exit 5; fi; }

#Load the rc.status script for this service
. /etc/rc.status

#Reset the status of this sergice
rc_reset

case "$1" in
	start)
		echo -n "Starting firstCall" > /dev/kmsg
		##Start wtih startproc.
		startproc $firstCall_BIN

		#REmember status and be verbose
		rc_status -v
		;;
	stop)
		echo -n "Shutting down firstCall" > /dev/kmsg
		##Stop deamon

		killproc -TERM $firstCall_BIN

		#Remember status and be verbose
		rc_status -v
		;;
	restart)
		## Stop the service regardless of whether it was 
		## running or not, start it again.
		$0 stop
		$0 start
	
		#Remember status and be quiet
		rc_status
		;;
	reload)
		## Doesn't support reload:
		rc_failed 3
		rc_status -v
		;;
	status)
		echo -n "CHecking for service firstCall" > /dev/kmsg

        ## Check status with checkproc(8), if process is running
        ## checkproc will return with exit status 0.

        # Return value is slightly different for the status command:
        # 0 - service up and running
        # 1 - service dead, but /var/run/  pid  file exists
        # 2 - service dead, but /var/lock/ lock file exists
        # 3 - service not running (unused)
        # 4 - service status unknown :-(
        # 5--199 reserved (5--99 LSB, 100--149 distro, 150--199 appl.)

        # NOTE: checkproc returns LSB compliant status values.
        checkproc $firstCall
        # NOTE: rc_status knows that we called this init script with
        # "status" option and adapts its messages accordingly.
        rc_status -v
        ;;
    *)
        ## If no parameters are given, print which are avaiable.
        echo "Usage: $0 {start|stop|status|restart|reload}" > /dev/kmsg
        exit 1
        ;;
esac


