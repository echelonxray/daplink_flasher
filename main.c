#include "main.h"
#include "errors.h"
#include "chips.h"
#include "probes.h"
#include "dapctl.h"
#include "modes/flash.h"
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

int verbosity;
RawParameters raw_params;

void talk_to_dap(libusb_device_handle* d_handle) {
	DAP_Connection dap_con;

	assert(! chips_find(&dap_con, "max32690") );

	dprintf(STDOUT, "START: Init Connection\n");
	assert(! chip_conn_init(&dap_con) );
	dprintf(STDOUT, "END: Init Connection\n");

	uint32_t address = 0x10030000;
	int len = 2;
	uint32_t buffer[len];
	oper_read_memblock32(&dap_con, address, buffer, len);
	for (int i = 0; i < len; i++) {
		printf("Address: 0x%08X, Value: 0x%08X\n", (uint32_t)(address + (sizeof(uint32_t) * i)), buffer[i]);
	}
	/*
	uint32_t tmp;
	tmp = buffer[0];
	for (int i = 0; i < len; i++) {
		if (i == len - 1) {
			buffer[i] = tmp;
		} else {
			buffer[i] = buffer[i + 1];
		}
	}
	*/

	dprintf(STDOUT, "START: Destroy Connection\n");
	assert(! chip_conn_destroy(&dap_con) );
	dprintf(STDOUT, "END: Destroy Connection\n");

	dprintf(STDOUT, "START: Init Connection\n");
	assert(! chip_conn_init(&dap_con) );
	dprintf(STDOUT, "END: Init Connection\n");

	printf("Erase> Address: 0x%08X\n", address);
	assert(! chip_erase_flash_page(&dap_con, address) );
	/*
	buffer[0] = 0x01234567;
	buffer[1] = 0x89ABCDEF;
	buffer[2] = 0xDEADF33D;
	buffer[3] = 0xB16B00B5;
	*/

	unsigned char buffer2[len * sizeof(uint32_t)];
	buffer2[0x0] = 0x01;
	buffer2[0x1] = 0x23;
	buffer2[0x2] = 0x45;
	buffer2[0x3] = 0x67;
	buffer2[0x4] = 0xAA;
	buffer2[0x5] = 0xCC;
	buffer2[0x6] = 0xBB;
	buffer2[0x7] = 0xDD;
	/*
	buffer2[0x8] = 0x01;
	buffer2[0x9] = 0x23;
	buffer2[0xA] = 0x45;
	buffer2[0xB] = 0x67;
	buffer2[0xC] = 0x89;
	buffer2[0xD] = 0xAB;
	buffer2[0xE] = 0xCD;
	buffer2[0xF] = 0xEF;
	*/
	printf("Write> Address: 0x%08X\n", address);
	for (int i = 0; i < len; i++) {
		uint32_t tmp0 = buffer2[i * sizeof(uint32_t) + 0];
		uint32_t tmp1 = buffer2[i * sizeof(uint32_t) + 1];
		uint32_t tmp2 = buffer2[i * sizeof(uint32_t) + 2];
		uint32_t tmp3 = buffer2[i * sizeof(uint32_t) + 3];
		uint32_t tmpi = 0;
		tmpi |= (tmp0 << 24);
		tmpi |= (tmp1 << 16);
		tmpi |= (tmp2 <<  8);
		tmpi |= (tmp3 <<  0);
		printf("\tBytes: %02X, %02X, %02X, %02X | 0x%08Xbe\n", tmp0, tmp1, tmp2, tmp3, tmpi);
	}
	assert(! chip_write_to_flash_page(&dap_con, address, buffer2, len * sizeof(uint32_t)) );

	/*
	uint32_t address = 0x10030000;
	uint32_t buffer[4];
	oper_read_memblock32(&dap_con, 0x10030000, buffer, 4);
	for (int i = 0; i < 4; i++) {
		printf("Address: 0x%08X, Value: 0x%08X\n", address + (4 * i), buffer[i]);
	}
	*/

	dprintf(STDOUT, "START: Destroy Connection\n");
	assert(! chip_conn_destroy(&dap_con) );
	dprintf(STDOUT, "END: Destroy Connection\n");

	return;
}

#define CHECK_OVERFLOW() { \
	if (i >= argc) { \
		PRINT_ERR("Missing value after %s", arg); \
		return ERROR_MALFORMED_INPUT; \
	} \
}
char* splitstr(char* str, char separator) {
	while (*str != '\0') {
		if (*str == separator) {
			*str = '\0';
			return str + 1;
		}
		str++;
	}
	return str;
}
signed int parse_general_params(int argc, const char* argv[], DAP_Connection* dap_con) {
	for (int i = 0; i < argc; i++) {
		const char* arg = argv[i];
		if        (strcmp(arg, "-h") == 0 || strcmp(arg, "-?") || strcmp(arg, "--help") == 0) {
			PRINT_USAGE();
		} else if (strcmp(arg, "--") == 0) {
			break;
		} else {
			// Switches that expect another argument
			i++;
			if        (strcmp(arg, "-c") == 0 || strcmp(arg, "--chip") == 0) {
				CHECK_OVERFLOW();
				raw_params.chip = argv[i];
				signed int retval;
				retval = chips_find(dap_con, raw_params.chip);
				if (retval != SUCCESS_STATUS) {
					PRINT_ERR("Unknown chip %s", raw_params.chip);
					return retval;
				}
			} else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--probe") == 0) {
				CHECK_OVERFLOW();
				raw_params.probe = argv[i];
				signed int retval;
				retval = probes_find(dap_con, raw_params.probe);
				if (retval != SUCCESS_STATUS) {
					PRINT_ERR("Unknown probe %s", raw_params.probe);
					return retval;
				}
				// strtol() error checking is not needed here because
				// dap_con->usbvid_str and dap_con->usbpid_str are filled by
				// a program internal source.  They should always be vaild
				// after a successful return of probes_find().
				dap_con->usbvid = strtol(dap_con->usbvid_str, NULL, 0);
				dap_con->usbpid = strtol(dap_con->usbpid_str, NULL, 0);
			} else if (strcmp(arg, "-i") == 0 || strcmp(arg, "--image") == 0) {
				CHECK_OVERFLOW();
				raw_params.image = argv[i];

				void* tmp_alloc;
				tmp_alloc = realloc(raw_params.image_buffer, strlen(raw_params.image) + 1);
				if (tmp_alloc == NULL) {
					free(raw_params.image_buffer);
					PRINT_ERR("Memory allocation failure.  realloc() returned NULL.");
					return ERROR_MEMALLOC;
				}
				raw_params.image_buffer = tmp_alloc;
				strcpy(raw_params.image_buffer, raw_params.image);

				char* buf = raw_params.image_buffer;
				char* new_buf;

				// Find the image path
				new_buf = splitstr(buf, ',');
				if (strlen(buf) > 0) {
					raw_params.image_path = buf;
				} else {
					raw_params.image_path = NULL;
				}
				buf = new_buf;

				// Find the image type
				new_buf = splitstr(buf, ',');
				if (strlen(buf) > 0) {
					raw_params.image_format = buf;
				} else {
					raw_params.image_format = NULL;
				}
				buf = new_buf;

				// Find the image offset
				new_buf = splitstr(buf, ',');
				if (strlen(buf) > 0) {
					raw_params.image_offset = buf;
				} else {
					raw_params.image_offset = NULL;
				}
				buf = new_buf;
			} else if (strcmp(arg, "--vid") == 0) {
				CHECK_OVERFLOW();
				dap_con->usbvid_str = argv[i];
				char* endptr = NULL;
				signed long tmp_usbvid;
				tmp_usbvid = strtol(argv[i], &endptr, 0);
				if (endptr != NULL || tmp_usbvid < 0x0000 || tmp_usbvid > 0xFFFF) {
					PRINT_ERR("Invaild USB Vendor ID (VID): %s", argv[i]);
					return ERROR_MALFORMED_INPUT;
				}
				dap_con->usbvid = tmp_usbvid;
			} else if (strcmp(arg, "--pid") == 0) {
				CHECK_OVERFLOW();
				dap_con->usbpid_str = argv[i];
				char* endptr = NULL;
				signed long tmp_usbpid;
				tmp_usbpid = strtol(argv[i], &endptr, 0);
				if (endptr != NULL || tmp_usbpid < 0x0000 || tmp_usbpid > 0xFFFF) {
					PRINT_ERR("Invaild USB Product ID (PID): %s", argv[i]);
					return ERROR_MALFORMED_INPUT;
				}
				dap_con->usbpid = tmp_usbpid;
			} else {
				// Unknown switch
				PRINT_ERR("Unknown command line switch %s", arg);
				return ERROR_MALFORMED_INPUT;
			}
		}
	}
	return SUCCESS_STATUS;
}

int main(int argc, const char* argv[]) {
	//printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);

	verbosity = VERBOSITY_NORMAL;

	if (argc < 2) {
		PRINT_ERR("Insufficient arguments");
		PRINT_USAGE();
		return ERROR_MALFORMED_INPUT;
	}

	raw_params.image        = NULL;
	raw_params.image_buffer = NULL;
	raw_params.image_path   = NULL;
	raw_params.image_format = NULL;
	raw_params.image_offset = NULL;
	raw_params.chip         = NULL;
	raw_params.probe        = NULL;
	DAP_Connection dap_connection;
	dap_connection.usbvid_str = NULL;
	dap_connection.usbpid_str = NULL;
	parse_general_params(argc - 2, argv + 2, &dap_connection);

	signed int retval;
	if        (strcmp(argv[1], "flash") == 0) {
		retval = mode_flash(&dap_connection);
	} else {
	}

	free(raw_params.image_buffer);

	/*
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
	*/
	
	//printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);

	return 0;
}
