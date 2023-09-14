#include "dap.h"
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
	DAP_Connection dap_con;
	dap_con.device_handle = d_handle;

	// Connect
	{
		signed int retval;
		retval = dap_connect(&dap_con, DAP_CONNECT_PORT_MODE_SWD);
		assert(retval == 0);
		printf("Connect.\n");
	}

	// SWJ Sequence
	{
		signed int retval;
		unsigned char sequence[] = {
			0x00,
		};
		retval = dap_swj_sequence(&dap_con, 0x08, sequence, sizeof(sequence) / sizeof(*sequence));
		assert(retval == 0);
		printf("SWJ Sequence.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x0 | DEBUG_PORT | MODE_READ,
		};
		uint32_t transfer_buffer[] = {
			0x00000000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Read ID register: 0x%08X.\n", transfer_buffer[0]);
	}

	// Transfer - Enable AP register access and set SELECT to 0
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x4 | DEBUG_PORT | MODE_WRITE,
			0x8 | DEBUG_PORT | MODE_WRITE,
		};
		uint32_t transfer_buffer[] = {
			0x50000000,
			0x00000000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Enable AHB-AP register access and clear the SELECT register.\n");
	}

	// Transfer - Configure Debug SELECT register for access to the Access ID register
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x8 | DEBUG_PORT | MODE_WRITE,
		};
		uint32_t transfer_buffer[] = {
			0x000000F0,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Configure Debug SELECT register for access to the Access ID register.\n");
	}

	// Transfer - Read Access ID register
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0xC | ACCESS_PORT | MODE_READ,
		};
		uint32_t transfer_buffer[] = {
			0x00000000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Read Access ID register: 0x%08X.\n", transfer_buffer[0]);
	}

	// Transfer - Clear Debug SELECT register for access to the normal Access registers
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x8 | DEBUG_PORT | MODE_WRITE,
		};
		uint32_t transfer_buffer[] = {
			0x00000000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Clear Debug SELECT register for access to the normal Access registers.\n");
	}

	// Transfer - Set memory access mode to 32-bit.  Set memory address to 4-byte auto increment.  Set memory address to 0x10030000.
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x0 | ACCESS_PORT | MODE_WRITE,
			0x4 | ACCESS_PORT | MODE_WRITE,
		};
		uint32_t transfer_buffer[] = {
			0x00000022,
			0x10030000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Set memory access mode to 32-bit.  Set memory address to 4-byte auto increment.  Set memory address to 0x10030000.\n");
	}

	// Transfer - Read a 32-bit value from memory
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0xC | ACCESS_PORT | MODE_READ,
		};
		uint32_t transfer_buffer[] = {
			0x00000000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Read a 32-bit value from memory: 0x%08X.\n", transfer_buffer[0]);
	}

	// Disconnect
	{
		signed int retval;
		retval = dap_disconnect(&dap_con);
		assert(retval == 0);
		printf("Disconnect.\n");
	}

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
