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

void print_bytes(unsigned char* data, size_t length) {
	size_t i = 0;
	while (i < length) {
		printf("  ");

		char text[17];
		memset(text, '.', 16);
		text[16] = 0;

		int j;
		j = 0;
		while (i < length && j < 8) {
			printf("%02X ", data[i]);
			if (data[i] >= ' ' && data[i] <= '~') {
				text[j] = data[i];
			}
			i++;
			j++;
		}
		while (i < length && j < 16) {
			printf(" %02X", data[i]);
			if (data[i] >= ' ' && data[i] <= '~') {
				text[j] = data[i];
			}
			i++;
			j++;
		}

		for (int k = j; k < 16; k++) {
			printf("   ");
		}

		printf("  [%s]\n", text);
	}
	return;
}

ssize_t send_data(libusb_device_handle* d_handle, unsigned char* data, size_t length) {
	int len;
	int retval;
	retval = libusb_interrupt_transfer(d_handle, // Device Handle
		                               0x02,     // Endpoint (Send)
		                               data,     // Data Buffer
		                               length,   // Length
		                               &len,     // Amount transferred
		                               10000);   // Timeout ms (10 sec)
	if (retval == 0) {
		return len;
	}
	return retval;
}

ssize_t receive_data(libusb_device_handle* d_handle, unsigned char* data, size_t length) {
	int len;
	int retval;
	retval = libusb_interrupt_transfer(d_handle, // Device Handle
		                               0x81,     // Endpoint (Receive)
		                               data,     // Data Buffer
		                               length,   // Length
		                               &len,     // Amount transferred
		                               10000);   // Timeout ms (10 sec)
	if (retval == 0) {
		return len;
	}
	return retval;
}

#define DEBUG_PORT  0x0
#define ACCESS_PORT 0x1
#define MODE_WRITE  0x0
#define MODE_READ   0x2

#define VERIFY_COM_RET_S(arg1, arg2, arg3) \
	{ \
		int retval; \
		retval = send_data(arg1, arg2, arg3); \
		assert(retval >= 0); \
		printf("Sent %d bytes via control transfer:\n", retval); \
		print_bytes(data, retval); \
	}
#define VERIFY_COM_RET_R(arg1, arg2, arg3) \
	{ \
		int retval; \
		retval = receive_data(arg1, arg2, arg3); \
		assert(retval >= 0); \
		printf("Received %d bytes via control transfer:\n", retval); \
		print_bytes(data, retval); \
	}

void talk_to_dap(libusb_device_handle* d_handle) {
	unsigned char data[64];

	// Connect
	memset(data, 0, 64);
	data[0] = 0x02;
	data[1] = 0x01;
	VERIFY_COM_RET_S(d_handle, data, 2);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// SWJ Sequence
	memset(data, 0, 64);
	data[0] = 0x12;
	data[1] = 0x08;
	data[2] = 0x00;
	VERIFY_COM_RET_S(d_handle, data, 3);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Transfer
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = 0x0 | DEBUG_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Transfer
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = 0x4 | DEBUG_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Transfer
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

	// Transfer
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x02;
	data[3] = 0x4 | DEBUG_PORT  | MODE_READ;
	data[4] = 0x0 | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 5);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Transfer
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

	// Transfer
	memset(data, 0, 64);
	data[0] = 0x05;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = 0xC | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Transfer
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

	// Transfer
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

	// Transfer
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x01;
	data[ 3] = 0x0 | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 4);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Transfer
	memset(data, 0, 64);
	data[ 0] = 0x05;
	data[ 1] = 0x00;
	data[ 2] = 0x02;
	data[ 3] = 0xC | ACCESS_PORT | MODE_READ;
	data[ 4] = 0x4 | ACCESS_PORT | MODE_READ;
	VERIFY_COM_RET_S(d_handle, data, 5);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);

	// Disconnect
	memset(data, 0, 64);
	data[0] = 0x03;
	VERIFY_COM_RET_S(d_handle, data, 1);
	memset(data, 0, 64);
	VERIFY_COM_RET_R(d_handle, data, 64);
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
