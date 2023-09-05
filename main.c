#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/hiddev.h>

int main(int argc, char *argv[]) {
	printf("[START]\n");
	
	if (argc < 2) {
		printf("Error: Invalid parameters.\n");
		return 1;
	}
	
	int fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		printf("Error: open(): %d: %s\n", errno, strerror(errno));
		return 2;
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	// TODO
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	close(fd);
	
	printf("[END]\n");
	return 0;
}
