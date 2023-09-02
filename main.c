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
	printf("[Stage: 0]\n");
	
	if (argc < 2) {
		return 1;
	}
	
	int fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		printf("Error: open(): %d: %s\n", errno, strerror(errno));
		return 2;
	}
	
	/*
	printf("[Stage: 1]\n");
	
	{
		int version;
		int ret_i;
		ret_i = ioctl(fd, HIDIOCGVERSION, &version);
		if (ret_i == -1) {
			printf("Error: ioctl(): %d: %s\n", errno, strerror(errno));
			return 2;
		}
		printf("\tversion: %d\n", version);
	}
	
	printf("[Stage: 2]\n");
	*/
	
	{
		/*
		struct hiddev_devinfo {
			__u32 bustype;
			__u32 busnum;
			__u32 devnum;
			__u32 ifnum;
			__s16 vendor;
			__s16 product;
			__s16 version;
			__u32 num_applications;
		};
		*/
		/*
		int ret_i;
		struct hiddev_devinfo hid_dinfo;
		ret_i = ioctl(fd, HIDIOCGVERSION, &hid_dinfo);
		if (ret_i == -1) {
			printf("Error: ioctl(): %d: %s\n", errno, strerror(errno));
			return 2;
		}
		printf("\tstruct hiddev_devinfo {\n");
		printf("\t    __u32 bustype == %u;\n", hid_dinfo.bustype);
		printf("\t    __u32 busnum == %u;\n", hid_dinfo.busnum);
		printf("\t    __u32 devnum == %u;\n", hid_dinfo.devnum);
		printf("\t    __u32 ifnum == %u;\n", hid_dinfo.ifnum);
		printf("\t    __s16 vendor == %d;\n", hid_dinfo.vendor);
		printf("\t    __s16 product == %d;\n", hid_dinfo.product);
		printf("\t    __s16 version == %d;\n", hid_dinfo.version);
		printf("\t    __u32 num_applications == %u;\n", hid_dinfo.num_applications);
		printf("\t};\n");
		*/
	}
	
	printf("[Stage: 3]\n");
	
	/*
	{
		char dev_name_str[256];
		int ret_i;
		ret_i = ioctl(fd, HIDIOCGNAME(256), &dev_name_str);
		if (ret_i == -1) {
			printf("Error: ioctl(): %d: %s\n", errno, strerror(errno));
			return 2;
		}
		printf("\tdev_name_str: %s\n", dev_name_str);
	}
	*/
	
	printf("[Stage: 4]\n");
	
	/*
	{
		struct hiddev_event {
			unsigned hid;
			signed int value;
		};
		struct hiddev_event hidevt;
		ssize_t ret_ss;
		ret_i = ioctl(fd, HIDIOCGVERSION, &version);
		ret_ss = read(fd, &hidevt, sizeof(hidevt));
		if (ret_ss == -1) {
			printf("Error: ioctl(): %d: %s\n", errno, strerror(errno));
			return 2;
		}
		printf("read(): %ld\n", (signed long int)ret_i);
		printf("struct hiddev_event {\n");
		printf("    unsigned hid == %u;\n", hidevt.hid);
		printf("    signed int value == %d;\n", hidevt.value);
		printf("};\n");
	}
	*/
	
	union {
		char     str[257];
		uint8_t  bc1;
		uint16_t bc2;
		uint32_t bc4;
	} ret_data;
	unsigned char cmd[2];
	ssize_t ret_ss;
	
	cmd[0] = 0x00;
	cmd[1] = 0x01;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	printf("split\n");
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	close(fd);
	return 5;
	
	cmd[0] = 0x00;
	cmd[1] = 0x02;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x03;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x04;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x05;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x06;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x07;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x08;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	close(fd);
	
	printf("[Stage: 5]\n");
	return 0;
}


