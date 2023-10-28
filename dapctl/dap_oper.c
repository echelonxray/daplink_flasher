#include "../main.h"
#include "dap_oper.h"
#include "../chips.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

signed int oper_write_reg(DAP_Connection* dap_con, unsigned int reg, uint32_t value) {
	unsigned int tr_req;
	tr_req = (reg & 0xC) | DAP_TRANSFER_MODE_WRITE;

	// Is this an Access Port operation?
	if (reg & OPER_REG_ACCESSMASK) {
		// If it is an Access Port operation, is the address select register set correctly?
		if (dap_con->sel_addr != (reg & 0xF0)) {
			// If not, update it by recurring into this function.
			// This won't cause an infinite recursion loop because the update is a
			// Debug Port operation.  Not an Access Port operation.
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
			// Transfer Failure
			return -1;
		}
		// Success

		// Was this a Debug Port command to update the Access Port selection address?
		if (tr_req == (DAP_TRANSFER_DEBUG_PORT | DAP_TRANSFER_MODE_WRITE | OPER_REG_DEBUG_SELECT)) {
			// If so, update the saved address to the new value.
			dap_con->sel_addr = value & 0xF0;
		}
	}

	return 0;
}
signed int oper_read_reg(DAP_Connection* dap_con, unsigned int reg, uint32_t* buffer) {
	unsigned int tr_req;
	tr_req = (reg & 0xC) | DAP_TRANSFER_MODE_READ;

	// Is this an Access Port operation?
	if (reg & OPER_REG_ACCESSMASK) {
		// If it is an Access Port operation, is the address select register set correctly?
		if (dap_con->sel_addr != (reg & 0xF0)) {
			// If not, update it.
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
			// Transfer Failure
			return -1;
		}
		// Success

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
		//dprintf(STDOUT, "Error: oper_write_mem32(): Address Misaligned: 0x%08X.\n", address);
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
		//dprintf(STDOUT, "Error: oper_read_mem32(): Address Misaligned: 0x%08X.\n", address);
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
		//dprintf(STDOUT, "Error: oper_write_mem32(): Address Misaligned: 0x%08X.\n", address);
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
		//dprintf(STDOUT, "Error: oper_read_mem32(): Address Misaligned: 0x%08X.\n", address);
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
		//dprintf(STDOUT, "Error: oper_write_memblock32(): Address Misaligned: 0x%08X.\n", address);
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
		//dprintf(STDOUT, "Error: oper_read_memblock32(): Address Misaligned: 0x%08X.\n", address);
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

signed int oper_init(DAP_Connection* dap_con) {
	// Connect
	{
		signed int retval;
		retval = dap_connect(dap_con, DAP_CONNECT_PORT_MODE_SWD);
		assert(retval == 0);
		//dprintf(STDOUT, "Connect.\n");
	}

	// SWJ Sequence
	{
		signed int retval;
		unsigned char sequence[] = {
			0x00,
		};
		retval = dap_swj_sequence(dap_con, 0x08, sequence, sizeof(sequence) / sizeof(*sequence));
		assert(retval == 0);
		//dprintf(STDOUT, "SWJ Sequence.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		uint32_t buffer;
		retval = oper_read_reg(dap_con, OPER_REG_DEBUG_IDCODE, &buffer);
		assert(retval == 0);
		//dprintf(STDOUT, "Transfer - Read Debug Port IDCODE register: 0x%08X.\n", buffer);
	}

	// Transfer - Enable AP register access and configure DB SELECT to enable access to the AP ID register
	{
		signed int retval;
		retval = oper_write_reg(dap_con, OPER_REG_DEBUG_CTRLSTAT, 0x50000000);
		assert(retval == 0);
		retval = oper_write_reg(dap_con, OPER_REG_DEBUG_SELECT, 0x000000F0);
		assert(retval == 0);
		//dprintf(STDOUT, "Transfer - Enable AHB-AP register access and set the SELECT register.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		uint32_t buffer;
		retval = oper_read_reg(dap_con, OPER_REG_ACCESS_IDR, &buffer);
		assert(retval == 0);
		//dprintf(STDOUT, "Transfer - Read Access Port IDR register: 0x%08X.\n", buffer);
	}

	// TODO: Check SOC Model

	// Reset and Halt
	//dprintf(STDOUT, "Reset and halt...");
	//chip_reset(local_dap_con, 1);
	//dprintf(STDOUT, "Done\n");

	//dprintf(STDOUT, "Chip Conn Init...");
	//chip_conn_init(local_dap_con);
	//dprintf(STDOUT, "Done\n");

	return 0;
}
signed int oper_destroy(DAP_Connection* dap_con) {
	// Reset and release from halt
	chip_reset(dap_con, 0);

	// Disconnect
	{
		signed int retval;
		retval = dap_disconnect(dap_con);
		assert(retval == 0);
		//dprintf(STDOUT, "Disconnect.\n");
	}

	return 0;
}
