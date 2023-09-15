#include "dap.h"
#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/hiddev.h>
#include <libusb-1.0/libusb.h>

#define DEBUG_PORT  0x0
#define ACCESS_PORT 0x1
#define MODE_WRITE  0x0
#define MODE_READ   0x2

void talk_to_dap(libusb_device_handle* d_handle) {
	DAP_Connection* dap_con;

	assert(! oper_init(&dap_con, d_handle) );
	printf("Init\n\n");

	uint32_t buffer[12];
	uint32_t address = 0x10030000;
	//uint32_t address = 0x40029400;
	/*
	for (address = 0x10030000; address < 0x10030010; address += 0x4) {
		assert(! oper_read_mem32(dap_con, address, buffer) );
		uint32_t tmp;
		oper_read_reg(dap_con, OPER_REG_ACCESS_TAR, &tmp);
		printf("Read32 from 0x%08X: 0x%08X; Next addr: 0x%08X\n", address, buffer[0], tmp);
	}
	*/
	printf("[START] Stage 1\n");
	assert(! oper_read_memblock32(dap_con, address, buffer, 12) );
	for (unsigned int i = 0; i < 12; i++) {
		printf("\tValue: 0x%08X\n", buffer[i]);
	}
	printf("[END] Stage 1\n\n");

	printf("[START] Stage 2\n");
	assert(! oper_erase_flash_page(dap_con, 0x10030000, 0x4000, 0x40029400) );
	printf("[END] Stage 2\n\n");

	printf("[START] Stage 3\n");
	assert(! oper_write_to_flash_page(dap_con, 0x10030010, 0x40029400,
	                                  0x01010101,0x23232323, 0x45454545, 0x67676767) );
	printf("[END] Stage 3\n\n");

	printf("[START] Stage 4\n");
	assert(! oper_read_memblock32(dap_con, address, buffer, 12) );
	for (unsigned int i = 0; i < 12; i++) {
		printf("\tValue: 0x%08X\n", buffer[i]);
	}
	printf("[END] Stage 4\n\n");

	//assert(! oper_read_memblock32(dap_con, address, buffer, 8) );
	//for (unsigned int i = 0; i < 8; i++) {
	//	printf("Value: 0x%08X\n", buffer[i]);
	//}
	/*
	{
		assert(! oper_read_mem32(dap_con, address, buffer) );
		printf("Read32 from 0x%08X: 0x%08X\n", address, buffer[0]);
		assert(! oper_write_mem32(dap_con, address, 0x01234567) );
		printf("Writ32 from 0x%08X: 0x%08X\n", address, 0x01234567);
		assert(! oper_read_mem32(dap_con, address, buffer) );
		printf("Read32 from 0x%08X: 0x%08X\n", address, buffer[0]);
	}
	*/

	assert(! oper_destroy(dap_con) );
	printf("Destroy\n");

	return;
}

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

	libusb_device **devs;
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0) {
		return r;
	}

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0){
		libusb_exit(NULL);
		return (int)cnt;
	}
	libusb_device_handle* d_handle = NULL;
	{
		libusb_device *dev;
		int i = 0;

		while ((dev = devs[i++]) != NULL) {
			struct libusb_device_descriptor desc;
			int r = libusb_get_device_descriptor(dev, &desc);
			if (r < 0) {
				fprintf(stderr, "failed to get device descriptor");
				continue;
			}

			if (desc.idVendor == 0x0D28 && desc.idProduct == 0x0204) {
				int retval;
				retval = libusb_open(dev, &d_handle);
				if (retval < 0) {
					printf("Error: libusb_open(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
					exit(3);
				}
#ifdef __linux__
				retval = libusb_detach_kernel_driver(d_handle, 3); // TODO: Reattach kernel driver
				if (retval < 0) {
					printf("Error: libusb_detach_kernel_driver(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
					//exit(3);
				}
#endif
				retval = libusb_claim_interface(d_handle, 3);
				if (retval < 0) {
					printf("Error: libusb_claim_interface(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
					exit(3);
				}

				talk_to_dap(d_handle);

				break;
			}
		}
	}

	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	close(fd);
	
	printf("[END]\n");
	return 0;
}
