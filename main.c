#include "main.h"
#include "dap.h"
#include "operations.h"

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

#define IMAGE_TYPE_BIN  0x1
#define IMAGE_TYPE_IHEX 0x2

#define ERRH_MEMALLOC { dprintf(STDERR, "Error: Memory Allocation Failure.  File: \"%s\", Line: %d\n", __FILE__, __LINE__); exit(1); }
#define HELP_REM "Use command flags \"--help\", \"-h\", or \"-?\" to show help and command usage information.\n"
#define HELP_MSG "TODO: Help Message\n"

void talk_to_dap(libusb_device_handle* d_handle) {
	DAP_Connection* dap_con;

	dprintf(STDOUT, "START: Init Connection\n");
	assert(! oper_init(&dap_con, d_handle) );
	dprintf(STDOUT, "END: Init Connection\n");

	dprintf(STDOUT, "\n");
	dprintf(STDOUT, "START: Destroy Connection\n");
	assert(! oper_destroy(dap_con) );
	dprintf(STDOUT, "END: Destroy Connection\n");

	return;
}

typedef struct {
	const char* path;
	unsigned char* data;
	uint32_t size;
	uint32_t load_address;
	short int type;
} ImageParam;

int main(int argc, const char* argv[]) {
	printf("[START]\n");

	size_t imagecount = 0;
	ImageParam* imagelist = NULL;

	// Parse Input
	for (int i = 1; i < argc; i++) {
		const char* param = argv[i];

		// Input image?
		if (!strcmp(param, "--image") || !strcmp(param, "-i")) {
			// Parse image path
			i++;
			if (i >= argc) {
				dprintf(STDERR, "Error: Missing image path after --image/-i command flag.\n" HELP_REM);
				exit(1);
			}
			param = argv[i];
			imagecount++;
			imagelist = reallocarray(imagelist, imagecount, sizeof(*imagelist));
			if (imagelist == NULL) {
				ERRH_MEMALLOC;
			}
			imagelist[imagecount - 1].path = param;

			// Parse image type
			i++;
			if (i >= argc) {
				dprintf(STDERR, "Error: Missing image type after image path.\n" HELP_REM);
				exit(1);
			}
			param = argv[i];
			if (!strcmp(param, "bin")) {
				imagelist[imagecount - 1].type = IMAGE_TYPE_BIN;
			} else {
				dprintf(STDERR, "Error: Unrecognized image type: \"%s\".\n" HELP_REM, param);
				exit(1);
			}

			// Parse image load location
			i++;
			if (i >= argc) {
				dprintf(STDERR, "Error: Missing image load location.\n" HELP_REM);
				exit(1);
			}
			param = argv[i];
			unsigned long long tmp_load_location;
			uint32_t load_location;
			char* endptr = "";
			tmp_load_location = strtoull(param, &endptr, 0);
			if (tmp_load_location == ULLONG_MAX || *endptr != '\0') {
				dprintf(STDERR, "Error: Invalid image load location: \"%s\".\n" HELP_REM, param);
				exit(1);
			}
			load_location = tmp_load_location;
			imagelist[imagecount - 1].load_address = load_location;

			continue;
		} else if (!strcmp(param, "--help") || !strcmp(param, "-h") || !strcmp(param, "-?")) {
			free(imagelist);
			dprintf(STDERR, HELP_MSG);
			exit(0);
		}

		dprintf(STDERR, "Error: Unrecognized command flag \"%s\".\n" HELP_REM, param);
		exit(1);
	}

	// Validate parameters
	// Validate and Load images
	{
		if (imagecount == 0) {
			dprintf(STDERR, "Error: No images specified.\n" HELP_REM);
			exit(1);
		}
		for (size_t i = 0; i < imagecount; i++) {
			int fd;
			fd = open(imagelist[i].path, O_RDONLY);
			if (fd == -1) {
				dprintf(STDERR, "Error: Failed to open() image: \"%s\".\n" HELP_REM, imagelist[i].path);
				exit(1);
			}

			struct stat statbuf;
			int retval;
			retval = fstat(fd, &statbuf);
			if (retval == -1) {
				dprintf(STDERR, "Error: Failed to fstat() image: \"%s\", fd: %d.\n" HELP_REM, imagelist[i].path, fd);
				exit(1);
			}

			unsigned char* data = NULL;
			data = malloc(statbuf.st_size);
			if (data == NULL && statbuf.st_size != 0) {
				ERRH_MEMALLOC;
			}

			ssize_t read_ret;
			read_ret = read(fd, data, statbuf.st_size);
			if (read_ret == -1) {
				dprintf(STDERR, "Error: Failed to read() image: \"%s\", fd: %d, errno: %d \"%s\".\n" HELP_REM,
				                imagelist[i].path, fd, errno, strerror(errno));
				exit(1);
			}
			if (read_ret != statbuf.st_size) {
				dprintf(STDERR, "Error: read() did not return image size.  image: \"%s\", fd: %d, image size: %lu, read() returned: %lu.\n" HELP_REM,
				                imagelist[i].path, fd, (unsigned long)statbuf.st_size, (unsigned long)read_ret);
				exit(1);
			}

			imagelist[i].data = data;
			imagelist[i].size = statbuf.st_size;

			retval = close(fd);
			if (retval == -1) {
				dprintf(STDERR, "Error: Failed to close() image: \"%s\", fd: %d, errno: %d \"%s\".\n" HELP_REM,
				                imagelist[i].path, fd, errno, strerror(errno));
				exit(1);
			}

			// Is this image within the bounds of programmable flash?
			{
				// MAX32690 Flash bounds
				//   mem0: 0x1000_0000 - 0x102F_FFFF (Exactly   3 Mebibytes)
				//   mem1: 0x1030_0000 - 0x1033_FFFF (Exactly 256 Kibibytes)
				uint32_t start_addr;
				uint32_t end_addr;
				start_addr = imagelist[i].load_address;
				end_addr = imagelist[i].load_address + imagelist[i].size;
				if (start_addr < 0x10000000 || start_addr >= 0x10340000 ||
				    end_addr > 0x10340000 ||
				    end_addr < start_addr) {
					dprintf(STDERR, "Error: image \"%s\" exceeds bounds of flash memory.\n"
					                "\tFlash memory0> Start (Inclusive): 0x10000000, End (Non-Inclusive): 0x10300000, Length: 3145728.\n"
					                "\tFlash memory1> Start (Inclusive): 0x10300000, End (Non-Inclusive): 0x10340000, Length: 262144.\n"
					                "\t        Image> Start (Inclusive): 0x%08X, End (Non-Inclusive): 0x%08X, Length: %u.\n" HELP_REM,
					                imagelist[i].path,
					                (unsigned int)start_addr, (unsigned int)end_addr, (unsigned int)(end_addr - start_addr));
					exit(1);
				}
			}

			// Does this image overlap and conflict with any other images?
			{
				uint32_t curr_start_addr;
				uint32_t curr_end_addr;
				curr_start_addr = imagelist[i].load_address;
				curr_end_addr = imagelist[i].load_address + imagelist[i].size;
				size_t j = i;
				while (j > 0) {
					j--;
					uint32_t prev_start_addr;
					uint32_t prev_end_addr;
					prev_start_addr = imagelist[j].load_address;
					prev_end_addr = imagelist[j].load_address + imagelist[j].size;
					if ((prev_start_addr >= curr_start_addr && prev_start_addr <  curr_end_addr) ||
					    (prev_end_addr   >  curr_start_addr && prev_end_addr   <= curr_end_addr)) {
						// TODO: Error: Image load location block overlap
						dprintf(STDERR, "Error: Image A \"%s\" overlaps with Image B \"%s\".\n"
						                "\tImage A> Start (Inclusive): 0x%08X, End (Non-Inclusive): 0x%08X, Length: %u.\n"
						                "\tImage B> Start (Inclusive): 0x%08X, End (Non-Inclusive): 0x%08X, Length: %u.\n" HELP_REM,
						                imagelist[i].path, imagelist[j].path,
						                (unsigned int)curr_start_addr, (unsigned int)curr_end_addr, (unsigned int)(curr_end_addr - curr_start_addr),
						                (unsigned int)prev_start_addr, (unsigned int)prev_end_addr, (unsigned int)(prev_end_addr - prev_start_addr));
						exit(1);
					}
				}
			}
		}
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

	for (size_t i = 0; i < imagecount; i++) {
		free(imagelist[i].data);
	}
	free(imagelist);
	
	printf("[END]\n");
	return 0;
}
