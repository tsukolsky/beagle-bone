#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>

#define A 0

void dump_value(int fd) {
	char buffer[1024];
	lseek(fd, 0, 0);

	int size = read(fd, buffer, sizeof(buffer));
	buffer[size] = NULL;
	
	printf("\t\t size: %d  buffer: %s\n", size, buffer);
}

void dump_event(int fd, struct pollfd *pfd) {
	short revents = pfd->revents;

	printf("revents: 0x%04X\n", revents);
	if (revents & POLLERR) {
		printf("\t POLLERR  errno: %d\n", errno);

		if (errno == EAGAIN) {
			printf("\t\t EAGAIN\n");
		}
		if (errno == EINTR) {
			printf("\t\t EINTR\n");
		}
		if (errno == EINVAL) {
			printf("\t\t EINVAL\n");
		}
	}

	if (revents & POLLHUP) {
		printf("\t POLLHUP\n");
	}

	if (revents & POLLNVAL) {
		printf("\t POLLINVAL\n");
	}
	
	if (revents & POLLIN) {
		printf("\t POLLIN\n");
		dump_value(fd);
	}
	if (revents & POLLPRI) {
		printf("\t POLLPRI\n");

		dump_value(fd);
	}
	if (revents & POLLOUT) {
		printf("\t POLLOUT\n");
	}
	if (revents & POLLRDNORM) {
		printf("\t POLLNORM\n");
	}
	if (revents & POLLRDBAND) {
		printf("\t POLLRDBAND\n");
	}
	if (revents & POLLWRNORM) {
		printf("\t POLLWRNORM\n");
	}
	if (revents & POLLWRBAND) {
		printf("\t POLLWRBAND\n");
	}
}

int get_lead(int fd) {
	int value;
	lseek(fd, 0, 0);

	char buffer[1024];
	int size = read(fd, buffer, sizeof(buffer));
	if (size != -1) {
		buffer[size] = NULL;
		value = atoi(buffer);
	}
	else {
		value = -1;
	}
	return value;
}

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
