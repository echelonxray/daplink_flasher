#include "../main.h"
#include "flash.h"
#include "../errors.h"
#include "../chips.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define IMAGE_TYPE_BIN  0x1
#define IMAGE_TYPE_IHEX 0x2

typedef struct {
	const char* path;
	unsigned char* data;
	uint32_t size;
	uint32_t load_address;
	short int type;
} ImageParam;

extern RawParameters raw_params;

signed int mode_flash(DAP_Connection* dap_con) {
	// TODO: Check and validate parameters

	uint32_t load_offset = strtol(raw_params.image_offset, NULL, 0);
	if (strcmp(raw_params.image_format, "binary")) {
		PRINT_ERR("Error: unrecognized format: \"%s\"", raw_params.image_format);
		return ERROR_MALFORMED_INPUT;
	}

	dprintf(STDOUT, "raw_params.image_path: \"%s\"\n", raw_params.image_path);
	dprintf(STDOUT, "raw_params.image_format: \"%s\"\n", raw_params.image_format);
	dprintf(STDOUT, "raw_params.image_offset: \"%s\"\n", raw_params.image_offset);
	dprintf(STDOUT, "load_offset: \"0x%08X\"\n", load_offset);
	dprintf(STDOUT, "usbvid: \"0x%04X\"\n", dap_con->usbvid);
	dprintf(STDOUT, "usbpid: \"0x%04X\"\n", dap_con->usbpid);

	signed int fd;
	fd = open(raw_params.image_path, O_RDONLY);
	if (fd < 0) {
		PRINT_ERR("open(\"%s\", O_RDONLY) failed.", raw_params.image_path);
		return ERROR_FILEIO;
	}

	struct stat statbuf;
	signed int retval;
	retval = fstat(fd, &statbuf);
	if (retval < 0) {
		PRINT_ERR("fstat() failed.");
		close(fd);
		return ERROR_FILEIO;
	}

	void* image_data = malloc(statbuf.st_size);
	if (image_data == NULL) {
		PRINT_ERR(ERRSTR_MEMALLOC);
		close(fd);
		return ERROR_MEMALLOC;
	}

	ssize_t image_size;
	image_size = read(fd, image_data, statbuf.st_size);
	if (image_size != statbuf.st_size) {
		PRINT_ERR("read() failed.");
		free(image_data);
		close(fd);
		return ERROR_FILEIO;
	}

	close(fd);

	retval = link_find_and_connect(dap_con, dap_con->usbvid, dap_con->usbpid);
	if (retval) {
		free(image_data);
		RELAY_RETURN(retval);
	}

	retval = oper_init(dap_con);
	if (retval) {
		free(image_data);
		link_disconnect(dap_con);
		RELAY_RETURN(retval);
	}

	retval = chip_conn_init(dap_con);
	if (retval) {
		free(image_data);
		link_disconnect(dap_con);
		RELAY_RETURN(retval);
	}

	retval = chip_write_to_flash(dap_con, load_offset, image_data, image_size, 0);
	if (retval) {
		free(image_data);
		link_disconnect(dap_con);
		RELAY_RETURN(retval);
	}

	free(image_data);

	retval = chip_conn_destroy(dap_con);
	if (retval) {
		link_disconnect(dap_con);
		RELAY_RETURN(retval);
	}

	retval = oper_destroy(dap_con);
	if (retval) {
		link_disconnect(dap_con);
		RELAY_RETURN(retval);
	}

	retval = link_disconnect(dap_con);
	if (retval) {
		RELAY_RETURN(retval);
	}

	return 0;
}
