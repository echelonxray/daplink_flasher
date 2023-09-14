#include "link.h"

#include <stdio.h>
#include <string.h>
//#include <linux/hiddev.h>
#include <libusb-1.0/libusb.h>

void link_print_bytes(unsigned char* data, unsigned int length) {
	unsigned int i = 0;
	while (i < length) {
		printf("  ");

		char text[17];
		memset(text, '.', 16);
		text[16] = 0;

		unsigned int j;
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

		for (unsigned int k = j; k < 16; k++) {
			printf("   ");
		}

		printf("  [%s]\n", text);
	}
	return;
}

signed int link_send_data(void* device_handle, unsigned char* data, unsigned int length) {
	libusb_device_handle* d_handle;
	signed int len;
	signed int retval;

	//printf("TX %d\n", length);
	//link_print_bytes(data, 64);

	d_handle = device_handle;
	retval = libusb_interrupt_transfer(d_handle, // Device Handle
	                                   0x02,     // Endpoint (Send)
	                                   data,     // Data Buffer
	                                   length,   // Length
	                                   &len,     // Amount transferred
	                                   10000);   // Timeout ms (10 sec)
	if (retval != 0) {
		return -1;
	}

	return len;
}

signed int link_receive_data(void* device_handle, unsigned char* data, unsigned int length) {
	libusb_device_handle* d_handle;
	signed int len;
	signed int retval;

	d_handle = device_handle;
	retval = libusb_interrupt_transfer(d_handle, // Device Handle
	                                   0x81,     // Endpoint (Receive)
	                                   data,     // Data Buffer
	                                   length,   // Length
	                                   &len,     // Amount transferred
	                                   10000);   // Timeout ms (10 sec)
	if (retval != 0) {
		return -1;
	}

	//printf("RX\n");
	//link_print_bytes(data, 64);

	return len;
}
