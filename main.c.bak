#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/hiddev.h>

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
	
	{
		int version;
		int ret_i;
		ret_i = ioctl(fd, HIDIOCGVERSION, &version);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		printf("\tversion: %d\n", version);
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	{
		/*
		struct hiddev_devinfo {
			__u32 bustype;
			__u32 busnum;
			__u32 devnum;
			__u32 ifnum;
			__s16 vendor;
			__s16 product;
			__s16 version;
			__u32 num_applications;
		};
		*/
		int ret_i;
		struct hiddev_devinfo hid_dinfo;
		ret_i = ioctl(fd, HIDIOCGDEVINFO, &hid_dinfo);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		printf("\tstruct hiddev_devinfo {\n");
		printf("\t    __u32 bustype == %u;\n", hid_dinfo.bustype);
		printf("\t    __u32 busnum == %u;\n", hid_dinfo.busnum);
		printf("\t    __u32 devnum == %u;\n", hid_dinfo.devnum);
		printf("\t    __u32 ifnum == %u;\n", hid_dinfo.ifnum);
		printf("\t    __s16 vendor == %d;\n", hid_dinfo.vendor);
		printf("\t    __s16 product == %d;\n", hid_dinfo.product);
		printf("\t    __s16 version == %d;\n", hid_dinfo.version);
		printf("\t    __u32 num_applications == %u;\n", hid_dinfo.num_applications);
		printf("\t};\n");
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	{
		char dev_name_str[256];
		int ret_i;
		memset(dev_name_str, 0, 256);
		ret_i = ioctl(fd, HIDIOCGNAME(255), &dev_name_str);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		printf("\tdev_name_str: %s\n", dev_name_str);
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	uint32_t report_type;
	uint32_t report_id;
	uint32_t num_fields;
	{
		/*
		struct hiddev_report_info {
			__u32 report_type;
			__u32 report_id;
			__u32 num_fields;
		};
		*/
		int ret_i;
		struct hiddev_report_info hid_rinfo;
		//hid_rinfo.report_type = HID_REPORT_TYPE_INPUT;
		//hid_rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
		hid_rinfo.report_type = HID_REPORT_TYPE_FEATURE;
		hid_rinfo.report_id = HID_REPORT_ID_FIRST;
		ret_i = ioctl(fd, HIDIOCGREPORTINFO, &hid_rinfo);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		report_type = hid_rinfo.report_type;
		report_id = hid_rinfo.report_id;
		num_fields = hid_rinfo.num_fields;
		printf("\tstruct hiddev_report_info {\n");
		printf("\t    __u32 report_type == %u;\n", hid_rinfo.report_type);
		printf("\t    __u32 report_id == %u;\n", hid_rinfo.report_id);
		printf("\t    __u32 num_fields == %u;\n", hid_rinfo.num_fields);
		printf("\t};\n");
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	{
		/*
		struct hiddev_field_info {
			__u32 report_type;
			__u32 report_id;
			__u32 field_index;
			__u32 maxusage;
			__u32 flags;
			__u32 physical;         physical usage for this field
			__u32 logical;          logical usage for this field
			__u32 application;      application usage for this field
			__s32 logical_minimum;
			__s32 logical_maximum;
			__s32 physical_minimum;
			__s32 physical_maximum;
			__u32 unit_exponent;
			__u32 unit;
		};
		*/
		int ret_i;
		struct hiddev_field_info hid_finfo;
		hid_finfo.report_type = report_type;
		hid_finfo.report_id   = report_id;
		hid_finfo.field_index = 0;
		printf("report_type            == %d\n", report_type);
		printf("report_id              == %d\n", report_id);
		printf("num_fields             == %d\n", num_fields);
		printf("hid_finfo.field_index  == %d\n", hid_finfo.field_index);
		ret_i = ioctl(fd, HIDIOCGFIELDINFO, &hid_finfo);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		printf("\tstruct hiddev_field_info {\n");
		printf("\t    __u32 report_type == %u;\n", hid_finfo.report_type);
		printf("\t    __u32 report_id == %u;\n", hid_finfo.report_id);
		printf("\t    __u32 field_index == %u;\n", hid_finfo.field_index);
		printf("\t    __u32 maxusage == %u;\n", hid_finfo.maxusage);
		printf("\t    __u32 flags == 0x%X;\n", hid_finfo.flags);
		printf("\t    __u32 physical == %u;\n", hid_finfo.physical);
		printf("\t    __u32 logical == %u;\n", hid_finfo.logical);
		printf("\t    __u32 application == %u;\n", hid_finfo.application);
		printf("\t    __s32 logical_minimum == %d;\n", hid_finfo.logical_minimum);
		printf("\t    __s32 logical_maximum == %d;\n", hid_finfo.logical_maximum);
		printf("\t    __s32 physical_minimum == %d;\n", hid_finfo.physical_minimum);
		printf("\t    __s32 physical_maximum == %d;\n", hid_finfo.physical_maximum);
		printf("\t    __u32 unit_exponent == %u;\n", hid_finfo.unit_exponent);
		printf("\t    __u32 unit == %u;\n", hid_finfo.unit);
		printf("\t};\n");
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	{
		/*
		struct hiddev_usage_ref {
			__u32 report_type;
			__u32 report_id;
			__u32 field_index;
			__u32 usage_index;
			__u32 usage_code;
			__s32 value;
		};
		*/
		int ret_i;
		struct hiddev_usage_ref hid_uinfo;
		//memset(&hid_uinfo, 0, sizeof(hid_uinfo));
		hid_uinfo.report_type = report_type;
		hid_uinfo.report_id   = report_id;
		hid_uinfo.field_index = 0;
		hid_uinfo.usage_index = 0;
		ret_i = ioctl(fd, HIDIOCGUCODE, &hid_uinfo);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		ret_i = ioctl(fd, HIDIOCGUSAGE, &hid_uinfo);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		printf("\tstruct hiddev_usage_ref {\n");
		printf("\t    __u32 report_type == %u;\n", hid_uinfo.report_type);
		printf("\t    __u32 report_id == %u;\n", hid_uinfo.report_id);
		printf("\t    __u32 field_index == %u;\n", hid_uinfo.field_index);
		printf("\t    __u32 usage_index == %u;\n", hid_uinfo.usage_index);
		printf("\t    __u32 usage_code == 0x%X;\n", hid_uinfo.usage_code);
		printf("\t    __s32 value == %d;\n", hid_uinfo.value);
		printf("\t};\n");
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	{
		/*
		struct hiddev_usage_ref {
			__u32 report_type;
			__u32 report_id;
			__u32 field_index;
			__u32 usage_index;
			__u32 usage_code;
			__s32 value;
		};
		*/
		int ret_i;
		struct hiddev_usage_ref hid_uinfo;
		//memset(&hid_uinfo, 0, sizeof(hid_uinfo));
		hid_uinfo.report_type = HID_REPORT_TYPE_OUTPUT;
		hid_uinfo.report_id   = 2;
		hid_uinfo.field_index = 0;
		hid_uinfo.usage_index = 0;
		//hid_uinfo.usage_code  = 0;
		hid_uinfo.usage_code  = 0xFF000002;
		hid_uinfo.value       = 0;
		ret_i = ioctl(fd, HIDIOCSUSAGE, &hid_uinfo);
		if (ret_i == -1) {
			printf("Error: ioctl(), File: \"%s\", Line: %d, Error: (%d) \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));
			return 2;
		}
		printf("\tstruct hiddev_usage_ref {\n");
		printf("\t    __u32 report_type == %u;\n", hid_uinfo.report_type);
		printf("\t    __u32 report_id == %u;\n", hid_uinfo.report_id);
		printf("\t    __u32 field_index == %u;\n", hid_uinfo.field_index);
		printf("\t    __u32 usage_index == %u;\n", hid_uinfo.usage_index);
		printf("\t    __u32 usage_code == 0x%X;\n", hid_uinfo.usage_code);
		printf("\t    __s32 value == %d;\n", hid_uinfo.value);
		printf("\t};\n");
	}
	
	printf("[TRACE> File: \"%s\", Line Number: %d]\n", __FILE__, __LINE__);
	
	/*
	{
		struct hiddev_event {
			unsigned hid;
			signed int value;
		};
		struct hiddev_event hidevt;
		ssize_t ret_ss;
		ret_i = ioctl(fd, HIDIOCGVERSION, &version);
		ret_ss = read(fd, &hidevt, sizeof(hidevt));
		if (ret_ss == -1) {
			printf("Error: ioctl(): %d: %s\n", errno, strerror(errno));
			return 2;
		}
		printf("read(): %ld\n", (signed long int)ret_i);
		printf("struct hiddev_event {\n");
		printf("    unsigned hid == %u;\n", hidevt.hid);
		printf("    signed int value == %d;\n", hidevt.value);
		printf("};\n");
	}
	*/
	
	/*
	union {
		char     str[257];
		uint8_t  bc1;
		uint16_t bc2;
		uint32_t bc4;
	} ret_data;
	unsigned char cmd[2];
	ssize_t ret_ss;
	
	cmd[0] = 0x00;
	cmd[1] = 0x01;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	printf("split\n");
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	close(fd);
	return 5;
	
	cmd[0] = 0x00;
	cmd[1] = 0x02;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x03;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x04;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x05;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x06;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x07;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	
	cmd[0] = 0x00;
	cmd[1] = 0x08;
	ret_ss = write(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: write %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 3;
	}
	ret_ss = read(fd, cmd, 2);
	if (ret_ss != 2) {
		printf("Error: read %d, errno %d, \"%s\"\n", (int)ret_ss, errno, strerror(errno));
		return 4;
	}
	memset(&ret_data.str, 0, 257);
	read(fd, &ret_data, cmd[1]);
	ret_data.str[cmd[1]] = 0;
	printf("id: %d, length: %d\n", (int)cmd[0], (int)cmd[1]);
	printf("bc1: %u\n", ret_data.bc1);
	printf("bc2: %u\n", ret_data.bc2);
	printf("bc4: %u\n", ret_data.bc4);
	printf("string: \"%s\"\n", ret_data.str);
	*/
	
	close(fd);
	
	printf("[END]\n");
	return 0;
}


