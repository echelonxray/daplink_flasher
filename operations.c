#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define OPER_REG_ACCESSMASK     (0x100)

#define OPER_REG_DEBUG_IDCODE   (0x00)
#define OPER_REG_DEBUG_ABORT    (0x00)
#define OPER_REG_DEBUG_CTRLSTAT (0x04)
#define OPER_REG_DEBUG_RESEND   (0x08)
#define OPER_REG_DEBUG_SELECT   (0x08)
#define OPER_REG_DEBUG_RDBUFF   (0x0C)

#define OPER_REG_ACCESS_CSW     (0x00 | OPER_REG_ACCESSMASK)
#define OPER_REG_ACCESS_TAR     (0x04 | OPER_REG_ACCESSMASK)
#define OPER_REG_ACCESS_DRW     (0x0C | OPER_REG_ACCESSMASK)
#define OPER_REG_ACCESS_IDR     (0xFC | OPER_REG_ACCESSMASK)

signed int oper_write_reg(DAP_Connection* dap_con, unsigned int reg, unsigned int value) {
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
		assert(retval == 0); // TODO: Replace with return error code
		if (tr_req == (DAP_TRANSFER_DEBUG_PORT | DAP_TRANSFER_MODE_WRITE | OPER_REG_DEBUG_SELECT)) {
			dap_con->sel_addr = value & 0xF0;
		}
	}

	return 0;
}
signed int oper_read_reg(DAP_Connection* dap_con, unsigned int reg, unsigned int* buffer) {
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
		assert(retval == 0); // TODO: Replace with return error code
		*buffer = transfer_buffer[0];
	}

	return 0;
}

signed int oper_write_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t value) {
	if (address & 0x3) {
		printf("Error: oper_write_mem32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
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
		printf("Error: oper_read_mem32(): Address Misaligned: 0x%08X.\n", address);
		return -5;
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
		unsigned int tmp;
		retval = oper_read_reg(dap_con, OPER_REG_ACCESS_DRW, &tmp);
		assert(retval == 0);
		*buffer = tmp;
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
		printf("Connect.\n");
	}

	// SWJ Sequence
	{
		signed int retval;
		unsigned char sequence[] = {
			0x00,
		};
		retval = dap_swj_sequence(local_dap_con, 0x08, sequence, sizeof(sequence) / sizeof(*sequence));
		assert(retval == 0);
		printf("SWJ Sequence.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		unsigned int buffer;
		retval = oper_read_reg(local_dap_con, OPER_REG_DEBUG_IDCODE, &buffer);
		assert(retval == 0);
		printf("Transfer - Read Debug Port IDCODE register: 0x%08X.\n", buffer);
	}

	// Transfer - Enable AP register access and configure DB SELECT to enable access to the AP ID register
	{
		signed int retval;
		retval = oper_write_reg(local_dap_con, OPER_REG_DEBUG_CTRLSTAT, 0x50000000);
		assert(retval == 0);
		retval = oper_write_reg(local_dap_con, OPER_REG_DEBUG_SELECT, 0x000000F0);
		assert(retval == 0);
		printf("Transfer - Enable AHB-AP register access and set the SELECT register.\n");
	}

	// Transfer - Read ID register
	{
		signed int retval;
		unsigned int buffer;
		retval = oper_read_reg(local_dap_con, OPER_REG_ACCESS_IDR, &buffer);
		assert(retval == 0);
		printf("Transfer - Read Access Port IDR register: 0x%08X.\n", buffer);
	}

	*dap_con = local_dap_con;

	return 0;
}
signed int oper_destroy(DAP_Connection* dap_con) {
	// Disconnect
	{
		signed int retval;
		retval = dap_disconnect(dap_con);
		assert(retval == 0);
		printf("Disconnect.\n");
	}

	free(dap_con);

	return 0;
}
