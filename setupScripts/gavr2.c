#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>

#include "pollingExtras.h"

#define A 0
int main(){
	int fd;
	fd = open("/sys/class/gpio/gpio34/value", O_RDONLY);
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLPRI;
	pfd.revents = 0;
	int lead;
	while (1) {
		int ready = poll(&pfd, 1, -1);
		printf("ready: %d\n", ready);
		if (pfd.revents != 0) {
			printf("\t Lead A\n");
			//dump_event(fd[A], &pfd[A]);
			lead = get_lead(fd);
		}
		printf("\t\t A: %d\n", lead);
	}
	return 0;
}

