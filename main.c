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
	//unsigned char data[64];

	DAP_Connection dap_con;
	dap_con.device_handle = d_handle;

	// Connect
	{
		signed int retval;
		retval = dap_connect(&dap_con, DAP_CONNECT_PORT_MODE_SWD);
		assert(retval == 0);
		printf("Connect.\n");
	}
	/*
	memset(data, 0, 64);
	data[0] = 0x02;
	data[1] = 0x01;
	VERIFY_COM_RET_S(d_handle, data, 2);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
	/*
	memset(data, 0, 64);
	data[0] = 0x12;
	data[1] = 0x08;
	data[2] = 0x00;
	VERIFY_COM_RET_S(d_handle, data, 3);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

	// Transfer - Read ID register
	uint32_t error_stat;
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x0 | DEBUG_PORT | MODE_READ,
			//0x4 | DEBUG_PORT | MODE_READ,
		};
		uint32_t transfer_buffer[] = {
			0x00000000,
			//0x00000000,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Read ID register: 0x%08X.\n", transfer_buffer[0]);
		//printf("Transfer - Read ID register: 0x%08X.  Ctrl/Stat register: 0x%08X.\n", transfer_buffer[0], transfer_buffer[1]);
		error_stat = 0;
		if (transfer_buffer[1] & 0x00000080) {
			error_stat |= 0x08;
		}
		if (transfer_buffer[1] & 0x00000020) {
			error_stat |= 0x04;
		}
		if (transfer_buffer[1] & 0x00000010) {
			error_stat |= 0x02;
		}
		if (transfer_buffer[1] & 0x00000002) {
			error_stat |= 0x10;
		}
	}
	/*
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = 0x0 | DEBUG_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

	// Transfer
	/*
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = 0x4 | DEBUG_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

	// Transfer - Enable AP register access and set SELECT to 0
	{
		signed int retval;
		unsigned char transfer_request[] = {
			0x4 | DEBUG_PORT | MODE_WRITE,
			0x8 | DEBUG_PORT | MODE_WRITE,
			//0x0 | DEBUG_PORT | MODE_WRITE,
		};
		uint32_t transfer_buffer[] = {
			0x50000000,
			0x00000000,
			//error_stat,
		};
		retval = dap_transfer(&dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		assert(retval == 0);
		printf("Transfer - Enable AHB-AP register access and clear the SELECT register.\n");
	}
	/*
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x02;
	data[ 3] = 0x4 | DEBUG_PORT | MODE_WRITE;
	data[ 4] = 0x00;
	data[ 5] = 0x00;
	data[ 6] = 0x00;
	data[ 7] = 0x50;
	data[ 8] = 0x8 | DEBUG_PORT | MODE_WRITE;
	data[ 9] = 0x00;
	data[10] = 0x00;
	data[11] = 0x00;
	data[12] = 0x00;
	VERIFY_COM_RET_S(d_handle, data, 13);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

	// Transfer
	/*
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x02;
	data[3] = 0x4 | DEBUG_PORT  | MODE_READ;
	data[4] = 0x0 | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 5);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
	/*
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x01;
	data[ 3] = 0x8 | DEBUG_PORT | MODE_WRITE;
	data[ 4] = 0xF0;
	data[ 5] = 0x00;
	data[ 6] = 0x00;
	data[ 7] = 0x00;
	VERIFY_COM_RET_S(d_handle, data, 8);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
	/*
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = 0xC | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
	/*
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x01;
	data[ 3] = 0x8 | DEBUG_PORT | MODE_WRITE;
	data[ 4] = 0x00;
	data[ 5] = 0x00;
	data[ 6] = 0x00;
	data[ 7] = 0x00;
	VERIFY_COM_RET_S(d_handle, data, 8);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
	/*
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x02;
	data[ 3] = 0x0 | ACCESS_PORT | MODE_WRITE;
	data[ 4] = 0x22;
	data[ 5] = 0x00;
	data[ 6] = 0x00;
	data[ 7] = 0x00;
	data[ 8] = 0x4 | ACCESS_PORT | MODE_WRITE;
	data[ 9] = 0x00;
	data[10] = 0x00;
	data[11] = 0x03;
	data[12] = 0x10;
	VERIFY_COM_RET_S(d_handle, data, 13);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

	// Transfer
	/*
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x01;
	data[ 3] = 0x0 | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

	// Transfer
	/*
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x02;
	data[ 3] = 0xC | ACCESS_PORT | MODE_READ;
	data[ 4] = 0x4 | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 5);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
	/*
	memset(data, 0, 64);
	data[0] = 0x03;
	VERIFY_COM_RET_S(d_handle, data, 1);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
	*/

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
