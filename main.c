#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
		                               0x81,     // Endpoint (Send)
		                               data,     // Data Buffer
		                               length,   // Length
		                               &len,     // Amount transferred
		                               10000);   // Timeout ms (10 sec)
	if (retval == 0) {
		return len;
	}
	return retval;
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
		//int j = 0;
		//uint8_t path[8];

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
				break;
			}
			/*
			printf("%04x:%04x (bus %d, device %d)",
				desc.idVendor, desc.idProduct,
				libusb_get_bus_number(dev), libusb_get_device_address(dev));

			r = libusb_get_port_numbers(dev, path, sizeof(path));
			if (r > 0) {
				printf(" path: %d", path[0]);
				for (j = 1; j < r; j++)
					printf(".%d", path[j]);
			}
			printf("\n");
			*/
		}
	}

	if (d_handle != NULL) {
		unsigned char data[500];
		int retval;

		memset(data, 0, 64);
		data[0] = 0x00;
		data[1] = 0xFC;

		retval = send_data(d_handle, data, 2);
		if (retval >= 0) {
			printf("Sent %d bytes via control transfer.\n", retval);
			print_bytes(data, retval);
		} else {
			printf("Error: libusb_control_transfer(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
		}

		retval = receive_data(d_handle, data, 64);
		if (retval >= 0) {
			printf("Received %d bytes via control transfer:\n", retval);
			print_bytes(data, retval);
		} else {
			printf("Error: libusb_control_transfer(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
		}
	}

	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);

	/*
	struct usb_bus *busses;
	struct usb_bus *bus;
	struct usb_device *dev;
	int c, i, res;
	char data[337];
	char idata[10];

	libusb_init(NULL);
	usb_find_busses();
	usb_find_devices();
	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idProduct == 0x0204 && dev->descriptor.idVendor == 0x0d28) {
				printf("Product id: %04hx", dev->descriptor.idProduct);
				printf("Vendor id: %04hx \n", dev->descriptor.idVendor);

				usb_dev_handle *l_Handle = usb_open(dev);
				if (NULL == l_Handle) {
					printf( "usb_open(): no handle to device\n" );
				}
				res = usb_claim_interface(l_Handle, 0);
				if(res < -EBUSY) {
					printf("Device interface not available to be claimed! \n");
					exit(0);
				}
				if(res < -ENOMEM) {
					printf("Insufficient Memory! \n");
					exit(0);
				}
				printf( "\nPlease swipe your card\n", res);

				res = usb_interrupt_read(l_Handle, 0x81, data, 337, -1);
				printf( "\nusb_interrupt_read %d \n", res);

				c = -1;
				// I am interested in only 10 characters in this range
				for(i = 1; i < 10; i++) {
					idata[++c] = data[i];
				}
				c = atoi(idata);
				printf("\nMy data : %d\n", c);

				usb_release_interface(l_Handle, 0);
				usb_close(l_Handle);
			}
		}
	}
	//getchar();
	libusb_exit(0);
	*/
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	close(fd);
	
	printf("[END]\n");
	return 0;
}
