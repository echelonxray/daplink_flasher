// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2023 Michael T. Kloos <michael@michaelkloos.com>
 */

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
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/hiddev.h>
#include <libusb-1.0/libusb.h>

int verbosity;
RawParameters raw_params;

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
		if        (strcmp(arg, "-h") == 0 || strcmp(arg, "-?") == 0 || strcmp(arg, "--help") == 0) {
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
	verbosity = VERBOSITY_NORMAL;

	if (argc < 2) {
		PRINT_ERR("Insufficient arguments");
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
	dap_connection.usbvid = 0;
	dap_connection.usbpid = 0;
	dap_connection.sel_addr = 0;

	signed int retval;
	retval = parse_general_params(argc - 2, argv + 2, &dap_connection);
	if (retval) {
		free(raw_params.image_buffer);
		RELAY_RETURN(retval);
	}

	if        (strcmp(argv[1], "flash") == 0) {
		//retval = mode_flash(&dap_connection);
		mode_flash(&dap_connection);
	} else {
		free(raw_params.image_buffer);
		PRINT_ERR("Unknown command mode: \"%s\"", argv[1]);
		return ERROR_MALFORMED_INPUT;
	}

	free(raw_params.image_buffer);

	return 0;
}
