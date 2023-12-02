// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2023 Michael T. Kloos <michael@michaelkloos.com>
 */

#include "../main.h"
#include "../dapctl.h"
#include "../errors.h"
#include "../chips.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLC0_BASE 0x40029000
#define FLC1_BASE 0x40029400
#define FLCn_ADDR       0x00
#define FLCn_CLKDIV     0x04
#define FLCn_CTRL       0x08
#define FLCn_INTR       0x24
#define FLCn_DATA_0     0x30
#define FLCn_DATA_1     0x34
#define FLCn_DATA_2     0x38
#define FLCn_DATA_3     0x3C
#define FLCn_ACTRL      0x40
#define FLCn_WELR0      0x80
#define FLCn_RLR0       0x84
#define FLCn_WELR1      0x88
#define FLCn_RLR1       0x8C
#define FLCn_WELR2      0x90
#define FLCn_RLR2       0x94
#define FLCn_WELR3      0x98
#define FLCn_RLR3       0x9C
#define FLCn_WELR4      0xA0
#define FLCn_RLR4       0xA4
#define FLCn_WELR5      0xA8
#define FLCn_RLR5       0xAC

#define GCR_ROOTADDR 0x40000000
#define GCR_SYSCTRL        0x00
#define GCR_RST0           0x04
#define GCR_CLKCTRL        0x08
#define GCR_PM             0x0C
#define GCR_PCLKDIV        0x18
#define GCR_PCLKDIS0       0x24
#define GCR_MEMCTRL        0x28
#define GCR_MEMZ           0x2C
#define GCR_SYSST          0x40
#define GCR_RST1           0x44
#define GCR_PCLKDIS1       0x48
#define GCR_EVENTEN        0x4C
#define GCR_REVISION       0x50
#define GCR_SYSIE          0x54
#define GCR_ECCERR         0x64
#define GCR_ECCCED         0x68
#define GCR_ECCIE          0x6C
#define GCR_ECCADDR        0x70
#define GCR_BTLELDOCTRL    0x74
#define GCR_BTLELDODLY     0x78
#define GCR_GPR0           0x80

static signed int erase_flash_page(DAP_Connection* dap_con, uint32_t address) {
	uint32_t page_size;
	uint32_t controller_address;
	if        (address >= 0x10000000 && address <= 0x102FFFFF) {
		page_size = 0x4000;
		controller_address = 0x40029000;
	} else if (address >= 0x10300000 && address <= 0x1033FFFF) {
		page_size = 0x2000;
		controller_address = 0x40029400;
	} else {
		PRINT_ERR("Attempted to erase flash memory page at invalid address: 0x%08X.", address);
		return ERROR_MALFORMED_INPUT;
	}
	
	dprintf(STDERR, "[STATUS] Erasing page at address 0x%08X of size 0x%04X\n", address, page_size);

	address &= ~(page_size - 1); // Technically not needed.  Align address to page.

	signed int retval;
	retval = oper_write_mem32(dap_con, controller_address + FLCn_INTR, 0x00000000); // Disable interrupts and clear status flags.
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Wait for FLCn_CTRL.pend == 0
	{
		uint32_t tmp0;
		do {
			retval = oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} while (tmp0 & 0x01000000);
	}

	retval = oper_write_mem32(dap_con, controller_address + FLCn_CLKDIV, 0x0000003C); // Reset Clock Divisor.  (Expects 60Mhz SYS_CLK)
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_ADDR,   address);    // Set which page to erase.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20005500); // Unlock flash.  Set erase mode: page.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20005504); // Start erase operation.
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Wait for FLCn_CTRL.pend == 0 && FLCn_CTRL.pge == 0
	{
		uint32_t tmp0;
		do {
			retval = oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} while (tmp0 & 0x01000004);
	}
	// Erase operation should now be complete

	retval = oper_write_mem32(dap_con, controller_address + FLCn_CTRL, 0x00000000); // Lock flash.
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Check exit status
	{
		uint32_t tmp0;
		retval = oper_read_mem32(dap_con, controller_address + FLCn_INTR, &tmp0);
		if (retval) {
			RELAY_RETURN(retval);
		}
		if (tmp0 & 0x00000002) { // Flash access fault?
			PRINT_ERR("Flash Controller reported an Access Fault during the Erase Operation at address 0x%08X", address);
			return ERROR_DEV_CMD_FAILED;
		}
		if (tmp0 & 0x00000001) { // Flash operation complete?
			//dprintf(STDOUT, "Erase Flash Operation Complete.  Page Address: 0x%08X.\n", address);
			return SUCCESS_STATUS;
		}
	}

	// Should not be reachable.
	PRINT_ERR("Reached what should be unreachable code");
	return ERROR_UNSPECIFIED;
}
static signed int _write_to_flash_page(DAP_Connection* dap_con, uint32_t address, uint32_t data_0, uint32_t data_1, uint32_t data_2, uint32_t data_3, unsigned int progress) {
	uint32_t controller_address;
	if        (address >= 0x10000000 && address <= 0x102FFFFF) {
		controller_address = 0x40029000;
	} else if (address >= 0x10300000 && address <= 0x1033FFFF) {
		controller_address = 0x40029400;
	} else {
		PRINT_ERR("Attempted to write to flash memory page at invalid address: 0x%08X.", address);
		return ERROR_MALFORMED_INPUT;
	}

	dprintf(STDERR, "\r[STATUS] Writing 16 bytes at address 0x%08X. %u%%", address, progress);

	//if (address & (0x10 - 1)) {
	//	//dprintf(STDOUT, "Error: oper_write_flash_page(): Address Misaligned: 0x%08X.\n", address);
	//	return -5;
	//}

	signed int retval;
	retval = oper_write_mem32(dap_con, controller_address + FLCn_INTR, 0x00000000); // Disable interrupts and clear status flags.
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Wait for FLCn_CTRL.pend == 0
	{
		uint32_t tmp0;
		do {
			retval = oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} while (tmp0 & 0x01000000);
	}

	retval = oper_write_mem32(dap_con, controller_address + FLCn_CLKDIV, 0x0000003C); // Reset Clock Divisor.  (Expects 60Mhz SYS_CLK)
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_ADDR,   address);    // Set the address to write to.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_DATA_0, data_0);     // Set part 1/4 of the data to write.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_DATA_1, data_1);     // Set part 2/4 of the data to write.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_DATA_2, data_2);     // Set part 3/4 of the data to write.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_DATA_3, data_3);     // Set part 4/4 of the data to write.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20000000); // Unlock flash.
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20000001); // Start write operation.
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Wait for FLCn_CTRL.pend == 0 && FLCn_CTRL.wr == 0
	{
		uint32_t tmp0;
		do {
			retval = oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} while (tmp0 & 0x01000001);
	}
	// Write operation should now be complete

	retval = oper_write_mem32(dap_con, controller_address + FLCn_CTRL, 0x00000000); // Lock flash.
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Check exit status
	{
		uint32_t tmp0;
		retval = oper_read_mem32(dap_con, controller_address + FLCn_INTR, &tmp0);
		if (retval) {
			RELAY_RETURN(retval);
		}
		if (tmp0 & 0x00000002) { // Flash access fault?
			PRINT_ERR("Flash Controller reported an Access Fault during the Write Operation at address 0x%08X", address);
			return ERROR_DEV_CMD_FAILED;
		}
		if (tmp0 & 0x00000001) { // Flash operation complete?
			//dprintf(STDOUT, "Write Flash Operation Complete\n");
			return SUCCESS_STATUS;
		}
	}

	// Should not be reachable.
	PRINT_ERR("Reached what should be unreachable code");
	return ERROR_UNSPECIFIED;
}
static signed int _write_to_flash_page_partial(DAP_Connection* dap_con, uint32_t address, unsigned char* data, size_t data_len, int do_preserve, unsigned int progress) {
	uint32_t aligned_address;
	uint32_t start_skip_count;

	aligned_address  = address & ~(0x10 - 1);
	start_skip_count = address - aligned_address;

	uint32_t buffer[4];
	if (do_preserve) {
		signed int retval;
		retval = oper_read_memblock32(dap_con, aligned_address, buffer, 4);
		if (retval) {
			RELAY_RETURN(retval);
		}
	} else {
		memset(buffer, 0xFF, sizeof(buffer));
	}

	unsigned char* buffer_ptr = (unsigned char*)buffer;
	buffer_ptr += start_skip_count;
	for (unsigned int i = 0; i < data_len; i++) {
		*buffer_ptr = data[i];
		buffer_ptr++;
	}

	signed int retval;
	retval = _write_to_flash_page(dap_con, aligned_address, buffer[0], buffer[1], buffer[2], buffer[3], progress);
	if (retval) {
		RELAY_RETURN(retval);
	}

	return SUCCESS_STATUS;
}
static signed int write_to_flash_page(DAP_Connection* dap_con, uint32_t address, unsigned char* data, size_t data_len, int do_preserve) {
	// Does not need to handle data_len == 0.
	// Does not need to handle address + data_len type overflow.
	// These cases are checked in the wrapper function.
	unsigned int writes_to_do = 0;
	unsigned int writes_done = 0;

	if (address < 0x10000000 && address > 0x1033FFFF) {
		PRINT_ERR("Attempted to write to flash memory page at invalid address: 0x%08X.", address);
		return ERROR_MALFORMED_INPUT;
	}

	uint32_t start_aligned_address;
	start_aligned_address = address;
	start_aligned_address &= ~(0x10 - 1);
	if (start_aligned_address != address) {
		writes_to_do++;
	}

	uint32_t end_aligned_address;
	end_aligned_address = address + data_len;
	end_aligned_address &= ~(0x10 - 1);
	if (end_aligned_address != address + data_len) {
		writes_to_do++;
	}

	writes_to_do += (end_aligned_address - start_aligned_address) / 0x10;

	if (start_aligned_address == end_aligned_address) {
		signed int retval;
		retval = _write_to_flash_page_partial(dap_con, address, data, data_len, do_preserve, 100);
		if (retval) {
			dprintf(STDERR, "\n");
			RELAY_RETURN(retval);
		}
		dprintf(STDERR, "\n");
		return SUCCESS_STATUS;
	}

	uint32_t current_aligned_address;
	current_aligned_address = start_aligned_address;

	if (current_aligned_address != address) {
		uint32_t part_length;
		part_length = 0x10 - (address - current_aligned_address);
		signed int retval;
		writes_to_do++;
		writes_done++;
		retval = _write_to_flash_page_partial(dap_con, address, data, part_length, do_preserve, 100 * writes_done / writes_to_do);
		if (retval) {
			dprintf(STDERR, "\n");
			RELAY_RETURN(retval);
		}
		data += part_length;
		current_aligned_address += 0x10;
	}

	while (current_aligned_address != end_aligned_address) {
		uint32_t buffer[4];
		unsigned char* buffer_ptr = (unsigned char*)buffer;
		for (unsigned int i = 0; i < sizeof(*buffer) * 4; i++) {
			*buffer_ptr = *data;
			data++;
			buffer_ptr++;
		}
		/*
		for (unsigned short i = 0; i < 4; i++) (
			buffer[i] = 0;
			for (signed short j = 3; j >= 0; j--) {
				buffer[i] <<= 8;
				buffer[i] |= data[j];
			}
			data += 4;
		)
		*/

		signed int retval;
		writes_done++;
		retval = _write_to_flash_page(dap_con, current_aligned_address, buffer[0], buffer[1], buffer[2], buffer[3], 100 * writes_done / writes_to_do);
		if (retval) {
			dprintf(STDERR, "\n");
			RELAY_RETURN(retval);
		}
		/*
		uint32_t buffer[4];

		oper_read_memblock32(dap_con, current_aligned_address, buffer, 4);
		uint32_t data_offset = 0;
		if (current_aligned_address < address) {
			data_offset = address - current_aligned_address;
		}
		uint32_t data_length;
		data_length = 0x10;
		if ((current_aligned_address + 0x10) > (address + data_len)) {
			data_length -= (current_aligned_address + 0x10) - (address + data_len);
		}
		unsigned char* buffer_ptr = (unsigned char*)buffer;
		// Possible endianness issue using memcpy to copy into uint32_t buffer - TEST THIS!!
		for (uint32_t i = data_offset; i < data_length; i++) {
			buffer_ptr[i] = *data;
		}
		memcpy(buffer_ptr + data_offset, data, data_length);
		data += data_length;
		signed int retval;
		retval = _write_to_flash_page(dap_con, current_aligned_address, buffer[0], buffer[1], buffer[2], buffer[3]);
		if (retval) {
			return retval;
		}
		*/
		current_aligned_address += 0x10;
	}

	uint32_t end_address = address + data_len;
	uint32_t remaining_bytes = end_address - current_aligned_address;
	if (current_aligned_address != end_address) {
		signed int retval;
		writes_done++;
		retval = _write_to_flash_page_partial(dap_con, current_aligned_address, data, remaining_bytes, do_preserve, 100 * writes_done / writes_to_do);
		if (retval) {
			dprintf(STDERR, "\n");
			RELAY_RETURN(retval);
		}
	}

	dprintf(STDERR, "\n");
	return SUCCESS_STATUS;
}

static uint32_t _get_page_size(uint32_t address) {
	if (address < 0x10300000) {
		return 0x4000;
	} else {
		return 0x2000;
	}
	return 0;
}
static signed int _write_to_flash_partial(DAP_Connection* dap_con, uint32_t address, unsigned char* data, size_t data_len) {
	if (data_len == 0) {
		return SUCCESS_STATUS;
	}
	
	uint32_t page_size = _get_page_size(address);
	uint32_t page_aligned_address = address & ~(page_size - 1);
	
	unsigned char* buffer = malloc(page_size);
	if (buffer == NULL) {
		PRINT_ERR(ERRSTR_MEMALLOC);
		return ERROR_MEMALLOC;
	}
	
	signed int retval;
	retval = oper_read_memblock08(dap_con, page_aligned_address, buffer, page_size);
	if (retval) {
		RELAY_RETURN(retval);
	}
	
	uint32_t offset_within_page = address - page_aligned_address;
	for (uint32_t i = 0; i < data_len; i++) {
		buffer[i + offset_within_page] = data[i];
	}
	
	retval = erase_flash_page(dap_con, page_aligned_address);
	if (retval) {
		RELAY_RETURN(retval);
	}
	retval = write_to_flash_page(dap_con, page_aligned_address, buffer, page_size, 1);
	if (retval) {
		RELAY_RETURN(retval);
	}
	
	free(buffer);
	
	return SUCCESS_STATUS;
}
static signed int write_to_flash(DAP_Connection* dap_con, uint32_t address, unsigned char* data, size_t data_len, int do_preserve) {
	// Does not need to handle data_len == 0.
	// Does not need to handle address + data_len type overflow.
	// These cases are checked in the wrapper function.

	if (address < 0x10000000 && address > 0x1033FFFF) {
		PRINT_ERR("Attempted to write to flash memory at invalid address: 0x%08X.", address);
		return ERROR_MALFORMED_INPUT;
	}

	uint32_t start_aligned_address;
	start_aligned_address = address;
	start_aligned_address &= ~(_get_page_size(start_aligned_address) - 1);

	uint32_t end_aligned_address;
	end_aligned_address = address + data_len;
	end_aligned_address &= ~(_get_page_size(end_aligned_address) - 1);

	if (start_aligned_address == end_aligned_address) {
		if (do_preserve) {
			signed int retval;
			retval = _write_to_flash_partial(dap_con, address, data, data_len);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} else {
			signed int retval;
			retval = erase_flash_page(dap_con, start_aligned_address);
			if (retval) {
				RELAY_RETURN(retval);
			}
			retval = write_to_flash_page(dap_con, address, data, data_len, do_preserve);
			if (retval) {
				RELAY_RETURN(retval);
			}
		}
		return SUCCESS_STATUS;
	}

	uint32_t current_aligned_address;
	current_aligned_address = start_aligned_address;

	if (current_aligned_address != address) {
		uint32_t part_length;
		part_length = _get_page_size(current_aligned_address) - (address - current_aligned_address);
		if (do_preserve) {
			signed int retval;
			retval = _write_to_flash_partial(dap_con, address, data, part_length);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} else {
			signed int retval;
			retval = erase_flash_page(dap_con, current_aligned_address);
			if (retval) {
				RELAY_RETURN(retval);
			}
			retval = write_to_flash_page(dap_con, address, data, part_length, do_preserve);
			if (retval) {
				RELAY_RETURN(retval);
			}
		}
		data += part_length;
		current_aligned_address += 0x10;
	}

	while (current_aligned_address != end_aligned_address) {
		signed int retval;
		retval = erase_flash_page(dap_con, current_aligned_address);
		if (retval) {
			RELAY_RETURN(retval);
		}
		uint32_t page_size = _get_page_size(current_aligned_address);
		for (uint32_t i = 0; i < page_size; i += 0x10) {
			uint32_t buffer[4];
			unsigned char* buffer_ptr = (unsigned char*)buffer;
			for (unsigned int i = 0; i < 0x10; i++) {
				*buffer_ptr = *data;
				data++;
				buffer_ptr++;
			}
			retval = _write_to_flash_page(dap_con, current_aligned_address + i, buffer[0], buffer[1], buffer[2], buffer[3], 100 * (i + 0x10) / page_size);
			if (retval) {
				RELAY_RETURN(retval);
			}
		}
		dprintf(STDERR, "\n");
		current_aligned_address += page_size;
	}

	uint32_t end_address = address + data_len;
	uint32_t remaining_bytes = end_address - current_aligned_address;
	if (current_aligned_address != end_address) {
		if (do_preserve) {
			signed int retval;
			retval = _write_to_flash_partial(dap_con, current_aligned_address, data, remaining_bytes);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} else {
			signed int retval;
			retval = erase_flash_page(dap_con, current_aligned_address);
			if (retval) {
				RELAY_RETURN(retval);
			}
			retval = write_to_flash_page(dap_con, current_aligned_address, data, remaining_bytes, do_preserve);
			if (retval) {
				RELAY_RETURN(retval);
			}
		}
	}

	return SUCCESS_STATUS;
}

static signed int reset(DAP_Connection* dap_con, int halt) {
	if (halt) {
		signed int retval;
		retval = oper_write_mem32(dap_con, 0xE000EDF0, 0xA05F0003);
		if (retval) {
			RELAY_RETURN(retval);
		}
		uint32_t debugreg_demcr;
		retval = oper_read_mem32(dap_con,  0xE000EDFC, &debugreg_demcr);
		if (retval) {
			RELAY_RETURN(retval);
		}
		retval = oper_write_mem32(dap_con, 0xE000EDFC, debugreg_demcr | 0x1);
		if (retval) {
			RELAY_RETURN(retval);
		}

		retval = oper_write_mem32(dap_con, 0xE000ED0C, 0x05FA0000 | 0x1); // VECTRESET
		if (retval) {
			RELAY_RETURN(retval);
		}
	} else {
		signed int retval;
		retval = oper_write_mem32(dap_con, 0xE000ED0C, 0x05FA0000 | 0x4); // SYSRESETREQ
		if (retval) {
			RELAY_RETURN(retval);
		}
	}

	return SUCCESS_STATUS;
}

static signed int conn_init(DAP_Connection* dap_con, libusb_device_handle* d_handle) {
	signed int retval;
	retval = chip_reset(dap_con, 1);
	if (retval) {
		RELAY_RETURN(retval);
	}

	// Configure Clock - Set to 60Mhz internal oscillator (ISO)
	{
		uint32_t buffer;

		// Get GCR_CLKCTRL value
		retval = oper_read_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, &buffer);
		if (retval) {
			RELAY_RETURN(retval);
		}

		// Ensure the 60Mhz oscillator (ISO) is enabled
		buffer &= 0x3F3F2FC0; // Remove reserved bits.
		buffer |= 0x00040000; // Set the 60Mhz oscillator (ISO) enable bit.
		retval = oper_write_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, buffer);
		if (retval) {
			RELAY_RETURN(retval);
		}

		// Wait for the the 60 MHz oscillator (ISO) to be ready and get new value for GCR_CLKCTRL
		dprintf(STDOUT, "Wait for the ISO to be ready...");
		do {
			retval = oper_read_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, &buffer);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} while ((buffer & (1 << 26)) == 0);
		dprintf(STDOUT, "Done\n");

		// Set the SYS_OSC to the 60Mhz oscillator
		buffer &= 0x3F3F2FC0; // Remove reserved bits.
		buffer &= 0xFFFFFF3F; // Clear the clock divisor bits, reseting it to 0.
		buffer &= 0xFFFFF1FF; // Clear the clock source selection bits, reseting it to 0 (ISO).
		retval = oper_write_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, buffer);
		if (retval) {
			RELAY_RETURN(retval);
		}

		// Wait for the the system clock to be ready
		dprintf(STDOUT, "Wait for the SYSCLK to be ready...");
		do {
			retval = oper_read_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, &buffer);
			if (retval) {
				RELAY_RETURN(retval);
			}
		} while ((buffer & (1 << 13)) == 0);
		dprintf(STDOUT, "Done\n");
	}

	return SUCCESS_STATUS;
}
static signed int conn_destroy(DAP_Connection* dap_con) {
	signed int retval;
	retval = chip_reset(dap_con, 0);
	if (retval) {
		RELAY_RETURN(retval);
	}

	return SUCCESS_STATUS;
}

void chips_ff_max32690(DAP_Connection* dap_con) {
	dap_con->chip_pfns.chips_erase_flash_page = erase_flash_page;
	dap_con->chip_pfns.chips_write_to_flash_page = write_to_flash_page;
	dap_con->chip_pfns.chips_write_to_flash = write_to_flash;
	dap_con->chip_pfns.chips_reset = reset;
	dap_con->chip_pfns.chips_conn_init = conn_init;
	dap_con->chip_pfns.chips_conn_destroy = conn_destroy;
	return;
}
