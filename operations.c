#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define FLCn_ADDR   0x00
#define FLCn_CLKDIV 0x04
#define FLCn_CTRL   0x08
#define FLCn_INTR   0x24
#define FLCn_DATA_0 0x30
#define FLCn_DATA_1 0x34
#define FLCn_DATA_2 0x38
#define FLCn_DATA_3 0x3C
#define FLCn_ACTRL  0x40
#define FLCn_WELR0  0x80
#define FLCn_RLR0   0x84
#define FLCn_WELR1  0x88
#define FLCn_RLR1   0x8C
#define FLCn_WELR2  0x90
#define FLCn_RLR2   0x94
#define FLCn_WELR3  0x98
#define FLCn_RLR3   0x9C
#define FLCn_WELR4  0xA0
#define FLCn_RLR4   0xA4
#define FLCn_WELR5  0xA8
#define FLCn_RLR5   0xAC

signed int oper_write_reg(DAP_Connection* dap_con, unsigned int reg, uint32_t value) {
	unsigned int tr_req;
	tr_req = (reg & 0xC) | DAP_TRANSFER_MODE_WRITE;
	if (reg & OPER_REG_ACCESSMASK) {
		if (dap_con->sel_addr != (reg & 0xF0)) {
			oper_write_reg(dap_con, OPER_REG_DEBUG_SELECT, reg & 0xF0); // TODO: Handle Return Value
		}
		tr_req |= DAP_TRANSFER_ACCESS_PORT;
	}

	// Transfer
	{
		signed int retval;
		unsigned char transfer_request[] = {
			tr_req,
		};
		uint32_t transfer_buffer[] = {
			value,
		};
		retval = dap_transfer(dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		if (retval) {
			return -1;
		}
		if (tr_req == (DAP_TRANSFER_DEBUG_PORT | DAP_TRANSFER_MODE_WRITE | OPER_REG_DEBUG_SELECT)) {
			dap_con->sel_addr = value & 0xF0;
		}
	}

	return 0;
}
signed int oper_read_reg(DAP_Connection* dap_con, unsigned int reg, uint32_t* buffer) {
	unsigned int tr_req;
	tr_req = (reg & 0xC) | DAP_TRANSFER_MODE_READ;
	if (reg & OPER_REG_ACCESSMASK) {
		if (dap_con->sel_addr != (reg & 0xF0)) {
			oper_write_reg(dap_con, OPER_REG_DEBUG_SELECT, reg & 0xF0); // TODO: Handle Return Value
		}
		tr_req |= DAP_TRANSFER_ACCESS_PORT;
	}

	// Transfer
	{
		signed int retval;
		unsigned char transfer_request[] = {
			tr_req,
		};
		uint32_t transfer_buffer[] = {
			0x00000000,
		};
		retval = dap_transfer(dap_con,
		                      0,
		                      sizeof(transfer_request) / sizeof(*transfer_request),
		                      transfer_request,
		                      transfer_buffer);
		if (retval) {
			return -1;
		}
		*buffer = transfer_buffer[0];
	}

	return 0;
}

signed int oper_write_mem8(DAP_Connection* dap_con, uint32_t address, uint8_t value) {
	// Transfer - Set 1 byte address mode.  Disable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_08BITADDR |
		                                                      OPER_REG_ACCESS_CSWMODE_NOAUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Write to address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_DRW, value);
		assert(retval == 0);
	}

	return 0;
}
signed int oper_read_mem8(DAP_Connection* dap_con, uint32_t address, uint8_t* buffer) {
	// Transfer - Set 1 byte address mode.  Disable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_08BITADDR |
		                                                      OPER_REG_ACCESS_CSWMODE_NOAUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Read from address
	{
		signed int retval;
		uint32_t tmp;
		retval = oper_read_reg(dap_con, OPER_REG_ACCESS_DRW, &tmp);
		assert(retval == 0);
		*buffer = tmp;
	}

	return 0;
}

signed int oper_write_mem16(DAP_Connection* dap_con, uint32_t address, uint16_t value) {
	if (address & 0x1) {
		//printf("Error: oper_write_mem32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	// Transfer - Set 2 byte address mode.  Disable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_16BITADDR |
		                                                      OPER_REG_ACCESS_CSWMODE_NOAUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Write to address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_DRW, value);
		assert(retval == 0);
	}

	return 0;
}
signed int oper_read_mem16(DAP_Connection* dap_con, uint32_t address, uint16_t* buffer) {
	if (address & 0x1) {
		//printf("Error: oper_read_mem32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	// Transfer - Set 2 byte address mode.  Disable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_16BITADDR |
		                                                      OPER_REG_ACCESS_CSWMODE_NOAUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Read from address
	{
		signed int retval;
		uint32_t tmp;
		retval = oper_read_reg(dap_con, OPER_REG_ACCESS_DRW, &tmp);
		assert(retval == 0);
		*buffer = tmp;
	}

	return 0;
}

signed int oper_write_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t value) {
	if (address & 0x3) {
		//printf("Error: oper_write_mem32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	// Transfer - Set 4 byte address mode.  Disable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_32BITADDR | 0x23000000 |
		                                                      OPER_REG_ACCESS_CSWMODE_NOAUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Write to address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_DRW, value);
		assert(retval == 0);
	}

	return 0;
}
signed int oper_read_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t* buffer) {
	if (address & 0x3) {
		//printf("Error: oper_read_mem32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	// Transfer - Set 4 byte address mode.  Disable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_32BITADDR | 0x23000000 |
		                                                      OPER_REG_ACCESS_CSWMODE_NOAUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Read from address
	{
		signed int retval;
		uint32_t tmp;
		retval = oper_read_reg(dap_con, OPER_REG_ACCESS_DRW, &tmp);
		assert(retval == 0);
		*buffer = tmp;
	}

	return 0;
}

signed int oper_write_memblock32(DAP_Connection* dap_con, uint32_t address, uint32_t* values, uint32_t buffer_length) {
	if (address & 0x3) {
		//printf("Error: oper_write_memblock32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	// Transfer - Set 4 byte address mode.  Enable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_32BITADDR |
		                                                      OPER_REG_ACCESS_CSWMODE_AUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Write to address
	for (uint32_t i = 0; i < buffer_length; i++) {
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_DRW, values[i]);
		assert(retval == 0);
	}

	return 0;
}
signed int oper_read_memblock32(DAP_Connection* dap_con, uint32_t address, uint32_t* buffer, uint32_t buffer_length) {
	if (address & 0x3) {
		//printf("Error: oper_read_memblock32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	// Transfer - Set 4 byte address mode.  Enable auto-increment mode.
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_CSW, OPER_REG_ACCESS_CSWMODE_32BITADDR |
		                                                      OPER_REG_ACCESS_CSWMODE_AUTOINC);
		assert(retval == 0);
	}
	// Transfer - Set address
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_ACCESS_TAR, address);
		assert(retval == 0);
	}
	// Transfer - Read from address
	for (uint32_t i = 0; i < buffer_length; i++) {
		signed int retval;
		uint32_t tmp;
		retval = oper_read_reg(dap_con, OPER_REG_ACCESS_DRW, &tmp);
		assert(retval == 0);
		buffer[i] = tmp;
	}

	return 0;
}

signed int oper_erase_flash_page(DAP_Connection* dap_con, uint32_t address, uint32_t page_size, uint32_t controller_address) {
	if (address & (page_size - 1)) {
		//printf("Error: oper_erase_flash_page(): Address Misaligned: 0x%08X, Page Size: 0x%08X.\n", address, page_size);
		return -5;
	}

	oper_write_mem32(dap_con, controller_address + FLCn_INTR, 0x00000000); // Disable interrupts and clear status flags.

	// Wait for FLCn_CTRL.pend == 0
	{
		uint32_t tmp0;
		do {
			oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
		} while (tmp0 & 0x01000000);
	}

	oper_write_mem32(dap_con, controller_address + FLCn_CLKDIV, 0x00000078); // Reset Clock Divisor.  (Expects 120Mhz SYS_CLK)
	oper_write_mem32(dap_con, controller_address + FLCn_ADDR,   address);    // Set which page to erase.
	oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20005500); // Unlock flash.  Set erase mode: page.
	oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20005504); // Start erase operation.

	// Wait for FLCn_CTRL.pend == 0 && FLCn_CTRL.pge == 0
	{
		uint32_t tmp0;
		do {
			oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			//printf("Probing Flash Erase Op State: 0x%08X\n", tmp0);
		} while (tmp0 & 0x01000004);
	}
	// Erase operation should now be complete

	oper_write_mem32(dap_con, controller_address + FLCn_CTRL, 0x00000000); // Lock flash.

	// Check exit status
	{
		uint32_t tmp0;
		oper_read_mem32(dap_con, controller_address + FLCn_INTR, &tmp0);
		if (tmp0 & 0x00000002) { // Flash access fault?
			//printf("Error: oper_erase_flash_page(): Flash Access Fault: 0x%08X, Page Size: 0x%08X.\n", address, page_size);
			return -1;
		}
		if (tmp0 & 0x00000001) { // Flash operation complete?
			//printf("Erase Flash Operation Complete.  Page Address: 0x%08X.\n", address);
			return 0;
		}
	}

	// Should not be reachable.
	//printf("Error: oper_erase_flash_page(): Code pathway should not be reachable: 0x%08X, Page Size: 0x%08X.\n", address, page_size);
	return -5;
}
signed int oper_write_to_flash_page(DAP_Connection* dap_con, uint32_t address, uint32_t controller_address,
                                    uint32_t data_0, uint32_t data_1, uint32_t data_2, uint32_t data_3) {
	if (address & (0x4 - 1)) {
		//printf("Error: oper_write_flash_page(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	oper_write_mem32(dap_con, controller_address + FLCn_INTR, 0x00000000); // Disable interrupts and clear status flags.

	// Wait for FLCn_CTRL.pend == 0
	{
		uint32_t tmp0;
		do {
			oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
		} while (tmp0 & 0x01000000);
	}

	oper_write_mem32(dap_con, controller_address + FLCn_CLKDIV, 0x00000078); // Reset Clock Divisor.  (Expects 120Mhz SYS_CLK)
	oper_write_mem32(dap_con, controller_address + FLCn_ADDR,   address);    // Set the address to write to.
	oper_write_mem32(dap_con, controller_address + FLCn_DATA_0, data_0);     // Set part 1/4 of the data to write.
	oper_write_mem32(dap_con, controller_address + FLCn_DATA_1, data_1);     // Set part 2/4 of the data to write.
	oper_write_mem32(dap_con, controller_address + FLCn_DATA_2, data_2);     // Set part 3/4 of the data to write.
	oper_write_mem32(dap_con, controller_address + FLCn_DATA_3, data_3);     // Set part 4/4 of the data to write.
	oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20000000); // Unlock flash.
	oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20000001); // Start write operation.

	// Wait for FLCn_CTRL.pend == 0 && FLCn_CTRL.wr == 0
	{
		uint32_t tmp0;
		do {
			oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			//printf("Probing Flash Write Op State: 0x%08X\n", tmp0);
		} while (tmp0 & 0x01000001);
	}
	// Write operation should now be complete

	oper_write_mem32(dap_con, controller_address + FLCn_CTRL, 0x00000000); // Lock flash.

	// Check exit status
	{
		uint32_t tmp0;
		oper_read_mem32(dap_con, controller_address + FLCn_INTR, &tmp0);
		if (tmp0 & 0x00000002) { // Flash access fault?
			//printf("Error: oper_write_flash_page(): Flash Access Fault: 0x%08X.\n", address);
			return -1;
		}
		if (tmp0 & 0x00000001) { // Flash operation complete?
			//printf("Write Flash Operation Complete\n");
			return 0;
		}
	}

	// Should not be reachable.
	//printf("Error: oper_write_flash_page(): Code pathway should not be reachable: 0x%08X.\n", address);
	return -5;

	return 0;
}

/*
signed int oper_program_flash(DAP_Connection* dap_con, uint32_t address, uint8_t* buffer, uint32_t buffer_length) {
	if (address & 0x3) {
		//printf("Error: oper_read_memblock32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
	}

	uint32_t size_of_used_pages;
	uint32_t start_page_address;
	uint32_t end_page_address;
	start_page_address = address & 0x;
	size_of_used_pages
	void* page_backup;
	page_backup = malloc(size_of_used_pages);
	assert(! oper_read_memblock32(dap_con, address, page_backup, size_of_used_pages / 4) );

	return 0;
}
*/

signed int oper_reset(DAP_Connection* dap_con, int halt) {
	if (halt) {
		oper_write_mem32(dap_con, 0xE000EDF0, 0xA05F0003);
		uint32_t debugreg_demcr;
		oper_read_mem32(dap_con,  0xE000EDFC, &debugreg_demcr);
		oper_write_mem32(dap_con, 0xE000EDFC, debugreg_demcr | 0x1);

		oper_write_mem32(dap_con, 0xE000ED0C, 0x05FA0000 | 0x1); // VECTRESET
	} else {
		oper_write_mem32(dap_con, 0xE000ED0C, 0x05FA0000 | 0x4); // SYSRESETREQ
	}

	return 0;
}

signed int oper_init(DAP_Connection** dap_con, libusb_device_handle* d_handle) {
	DAP_Connection* local_dap_con;

	local_dap_con = malloc(sizeof(DAP_Connection));
	local_dap_con->device_handle = d_handle;

	// Connect
	{
		signed int retval;
		retval = dap_connect(local_dap_con, DAP_CONNECT_PORT_MODE_SWD);
		assert(retval == 0);
		//printf("Connect.\n");
	}

	// SWJ Sequence
	{
		signed int retval;
		unsigned char sequence[] = {
			0x00,
		};
		retval = dap_swj_sequence(local_dap_con, 0x08, sequence, sizeof(sequence) / sizeof(*sequence));
		assert(retval == 0);
		//printf("SWJ Sequence.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		uint32_t buffer;
		retval = oper_read_reg(local_dap_con, OPER_REG_DEBUG_IDCODE, &buffer);
		assert(retval == 0);
		//printf("Transfer - Read Debug Port IDCODE register: 0x%08X.\n", buffer);
	}

	// Transfer - Enable AP register access and configure DB SELECT to enable access to the AP ID register
	{
		signed int retval;
		retval = oper_write_reg(local_dap_con, OPER_REG_DEBUG_CTRLSTAT, 0x50000000);
		assert(retval == 0);
		retval = oper_write_reg(local_dap_con, OPER_REG_DEBUG_SELECT, 0x000000F0);
		assert(retval == 0);
		//printf("Transfer - Enable AHB-AP register access and set the SELECT register.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		uint32_t buffer;
		retval = oper_read_reg(local_dap_con, OPER_REG_ACCESS_IDR, &buffer);
		assert(retval == 0);
		//printf("Transfer - Read Access Port IDR register: 0x%08X.\n", buffer);
	}

	// TODO: Check SOC Model
	// TODO: Reset and Halt
	// TODO: Configure Clock

	*dap_con = local_dap_con;

	return 0;
}
signed int oper_destroy(DAP_Connection* dap_con) {
	// Disconnect
	{
		signed int retval;
		retval = dap_disconnect(dap_con);
		assert(retval == 0);
		//printf("Disconnect.\n");
	}

	free(dap_con);

	return 0;
}
