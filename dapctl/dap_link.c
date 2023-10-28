#include "../main.h"
#include "dap_link.h"
#include "../errors.h"
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

/*
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
*/

signed int link_send_data(DAP_Connection* dap_con, unsigned char* data, unsigned int length) {
	libusb_device_handle* d_handle;
	signed int len;
	signed int retval;

	//printf("TX %d\n", length);
	//link_print_bytes(data, 64);

	d_handle = dap_con->device_handle;
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
signed int link_receive_data(DAP_Connection* dap_con, unsigned char* data, unsigned int length) {
	libusb_device_handle* d_handle;
	signed int len;
	signed int retval;

	d_handle = dap_con->device_handle;
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

signed int link_find_and_connect(DAP_Connection* dap_con, uint16_t usb_vid, uint16_t usb_pid) {
	// Init libusb
	signed int retval;
	retval = libusb_init(NULL);
	if (retval != 0) {
		PRINT_ERR("libusb_init() failed with code: %d", retval);
		return ERROR_LIBUSB_SETUP;
	}

	// Get connected devices
	libusb_device** devices;
	ssize_t retval2;
	retval2 = libusb_get_device_list(NULL, &devices);
	if (retval2 < 0){
		libusb_exit(NULL);
		PRINT_ERR("libusb_get_device_list() failed with code: %d", (signed int)retval2);
		return ERROR_LIBUSB_SETUP;
	}

	// Find matching devices
	libusb_device** matching_devices;
	matching_devices = calloc(retval2, sizeof(libusb_device*));
	if (matching_devices == NULL) {
		libusb_free_device_list(devices, 1);
		libusb_exit(NULL);
		PRINT_ERR("Memory allocation failure.  calloc() returned NULL.");
		return ERROR_MEMALLOC;
	}
	for (ssize_t i = 0; i < retval2; i++) {
		matching_devices[i] = NULL;
	}
	ssize_t matching_device_count = 0;
	for (ssize_t i = 0; i < retval2; i++) {
		libusb_device* device;
		device = devices[i];
		if (device == NULL) {
			break;
		}
		struct libusb_device_descriptor desc;
		retval = libusb_get_device_descriptor(device, &desc);
		if (retval != 0) {
			free(matching_devices);
			libusb_free_device_list(devices, 1);
			libusb_exit(NULL);
			PRINT_ERR("libusb_get_device_descriptor() failed with code: %d", retval);
			return ERROR_LIBUSB_SETUP;
		}
		if (desc.idVendor == usb_vid && desc.idProduct == usb_pid) {
			matching_devices[matching_device_count] = device;
			matching_device_count++;
		}
	}
	if (matching_device_count <= 0) {
		free(matching_devices);
		libusb_free_device_list(devices, 1);
		libusb_exit(NULL);
		PRINT_ERR("No USB device with VID:PID matching %04X:%04X was found.", (signed int)usb_vid, (signed int)usb_pid);
		return ERROR_NOMATCHDEV;
	}
	void* tmp_ptr;
	tmp_ptr = reallocarray(matching_devices, matching_device_count, sizeof(libusb_device*));
	if (tmp_ptr == NULL) {
		free(matching_devices);
		libusb_free_device_list(devices, 1);
		libusb_exit(NULL);
		PRINT_ERR("Memory allocation failure.  reallocarray() returned NULL.");
		return ERROR_MEMALLOC;
	}
	matching_devices = tmp_ptr;

	// Select a USB device from the matching list
	libusb_device* the_device;
	if (matching_device_count == 1) {
		the_device = matching_devices[0];
	} else {
		// TODO: Handle multiple matching devices
		free(matching_devices);
		libusb_free_device_list(devices, 1);
		libusb_exit(NULL);
		PRINT_ERR("Multiple matching devices found.  The functionality to handle this is not yet implemented.");
		return ERROR_UNIMPL;
	}
	free(matching_devices);

	// Open the device
	libusb_device_handle* tmp_device_handle;
	retval = libusb_open(the_device, &tmp_device_handle);
	if (retval != 0) {
		libusb_free_device_list(devices, 1);
		libusb_exit(NULL);
		PRINT_ERR("libusb_open() failed with code: %d", retval);
		return ERROR_LIBUSB_SETUP;
		//printf("Error: libusb_open(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
		//exit(3);
	}
	libusb_free_device_list(devices, 1); // Also unrefs the devices

	// Setup the device for IO
	libusb_set_auto_detach_kernel_driver(tmp_device_handle, 1); // Returns either SUCCESS or NOT_SUPPORTED.  Either way is okay to proceed.
	retval = libusb_claim_interface(tmp_device_handle, 3);
	if (retval != 0) {
		libusb_close(tmp_device_handle);
		libusb_exit(NULL);
		PRINT_ERR("libusb_claim_interface() failed with code: %d", retval);
		return ERROR_LIBUSB_SETUP;
		//printf("Error: libusb_claim_interface(): (%d) \"%s\"\n", retval, libusb_strerror(retval));
		//exit(3);
	}

	// Save the device handle
	dap_con->device_handle = tmp_device_handle;

	/*
	//libusb_device_handle* d_handle = NULL;
	{
		libusb_device *dev;
		int i = 0;

		while ((dev = devices[i++]) != NULL) {
			struct libusb_device_descriptor desc;
			int r = libusb_get_device_descriptor(dev, &desc);
			if (r < 0) {
				fprintf(stderr, "failed to get device descriptor");
				continue;
			}

			if (desc.idVendor == usb_vid && desc.idProduct == usb_pid) {
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

				//talk_to_dap(d_handle);

				break;
			}
		}
	}

	libusb_free_device_list(devices, 1); // Also unrefs the devices
	libusb_exit(NULL);
	*/

	return SUCCESS_STATUS;
}
signed int link_disconnect(DAP_Connection* dap_con) {
	libusb_device_handle* d_handle;
	d_handle = dap_con->device_handle;

	signed int func_retval = SUCCESS_STATUS;
	signed int retval;

	retval = libusb_release_interface(d_handle, 3);
	if (retval != 0) {
		PRINT_ERR("libusb_release_interface() failed with code: %d", retval);
		func_retval = ERROR_LIBUSB_CLEAN;
	}
	libusb_close(d_handle);
	libusb_exit(NULL);

	return func_retval;
}
