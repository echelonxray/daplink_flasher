#include "main.h"
#include "chips.h"
#include "dapctl/dap_cmds.h"
#include "dapctl/dap_oper.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/hiddev.h>
#include <libusb-1.0/libusb.h>

#define DEBUG_PORT  0x0
#define ACCESS_PORT 0x1
#define MODE_WRITE  0x0
#define MODE_READ   0x2

void talk_to_dap(libusb_device_handle* d_handle) {
	DAP_Connection dap_con;
	
	assert(! chips_find(&dap_con, "max32690") );

	dprintf(STDOUT, "START: Init Connection\n");
	assert(! chip_conn_init(&dap_con, d_handle) );
	dprintf(STDOUT, "END: Init Connection\n");

	uint32_t address = 0x10030000;
	uint32_t buffer[4];
	oper_read_memblock32(&dap_con, 0x10030000, buffer, 4);
	for (int i = 0; i < 4; i++) {
		printf("Address: 0x%08X, Value: 0x%08X\n", address + (4 * i), buffer[i]);
	}

	dprintf(STDOUT, "START: Destroy Connection\n");
	assert(! chip_conn_destroy(&dap_con) );
	dprintf(STDOUT, "END: Destroy Connection\n");

	/*
	dprintf(STDOUT, "START: Init Connection\n");
	assert(! chip_conn_init(&dap_con, d_handle) );
	dprintf(STDOUT, "END: Init Connection\n");

	dprintf(STDOUT, "START: Destroy Connection\n");
	assert(! chip_conn_destroy(&dap_con) );
	dprintf(STDOUT, "END: Destroy Connection\n");
	*/

	return;
}

int main(int argc, const char* argv[]) {
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

	return 0;
}
