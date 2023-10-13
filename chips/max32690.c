#include "../main.h"
#include "../dapctl/dap_oper.h"
#include <stdio.h>
#include <assert.h>

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
	if (address & (page_size - 1)) {
		//dprintf(STDOUT, "Error: oper_erase_flash_page(): Address Misaligned: 0x%08X, Page Size: 0x%08X.\n", address, page_size);
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

	oper_write_mem32(dap_con, controller_address + FLCn_CLKDIV, 0x0000003C); // Reset Clock Divisor.  (Expects 60Mhz SYS_CLK)
	oper_write_mem32(dap_con, controller_address + FLCn_ADDR,   address);    // Set which page to erase.
	oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20005500); // Unlock flash.  Set erase mode: page.
	oper_write_mem32(dap_con, controller_address + FLCn_CTRL,   0x20005504); // Start erase operation.

	// Wait for FLCn_CTRL.pend == 0 && FLCn_CTRL.pge == 0
	{
		uint32_t tmp0;
		do {
			oper_read_mem32(dap_con, controller_address + FLCn_CTRL, &tmp0);
			//dprintf(STDOUT, "Probing Flash Erase Op State: 0x%08X\n", tmp0);
		} while (tmp0 & 0x01000004);
	}
	// Erase operation should now be complete

	oper_write_mem32(dap_con, controller_address + FLCn_CTRL, 0x00000000); // Lock flash.

	// Check exit status
	{
		uint32_t tmp0;
		oper_read_mem32(dap_con, controller_address + FLCn_INTR, &tmp0);
		if (tmp0 & 0x00000002) { // Flash access fault?
			//dprintf(STDOUT, "Error: oper_erase_flash_page(): Flash Access Fault: 0x%08X, Page Size: 0x%08X.\n", address, page_size);
			return -1;
		}
		if (tmp0 & 0x00000001) { // Flash operation complete?
			//dprintf(STDOUT, "Erase Flash Operation Complete.  Page Address: 0x%08X.\n", address);
			return 0;
		}
	}

	// Should not be reachable.
	//dprintf(STDOUT, "Error: oper_erase_flash_page(): Code pathway should not be reachable: 0x%08X, Page Size: 0x%08X.\n", address, page_size);
	return -5;
}
static signed int write_to_flash_page(DAP_Connection* dap_con, uint32_t address, unsigned char* data) {
	if (address & (0x4 - 1)) {
		//dprintf(STDOUT, "Error: oper_write_flash_page(): Address Misaligned: 0x%08X.\n", address);
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

	oper_write_mem32(dap_con, controller_address + FLCn_CLKDIV, 0x0000003C); // Reset Clock Divisor.  (Expects 60Mhz SYS_CLK)
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
			//dprintf(STDOUT, "Probing Flash Write Op State: 0x%08X\n", tmp0);
		} while (tmp0 & 0x01000001);
	}
	// Write operation should now be complete

	oper_write_mem32(dap_con, controller_address + FLCn_CTRL, 0x00000000); // Lock flash.

	// Check exit status
	{
		uint32_t tmp0;
		oper_read_mem32(dap_con, controller_address + FLCn_INTR, &tmp0);
		if (tmp0 & 0x00000002) { // Flash access fault?
			//dprintf(STDOUT, "Error: oper_write_flash_page(): Flash Access Fault: 0x%08X.\n", address);
			return -1;
		}
		if (tmp0 & 0x00000001) { // Flash operation complete?
			//dprintf(STDOUT, "Write Flash Operation Complete\n");
			return 0;
		}
	}

	// Should not be reachable.
	//dprintf(STDOUT, "Error: oper_write_flash_page(): Code pathway should not be reachable: 0x%08X.\n", address);
	return -5;

	return 0;
}

/*
signed int oper_program_flash(DAP_Connection* dap_con, uint32_t address, uint8_t* buffer, uint32_t buffer_length) {
	if (address & 0x3) {
		//dprintf(STDOUT, "Error: oper_read_memblock32(): Address Misaligned: 0x%08X.\n", address);
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

static signed int reset(DAP_Connection* dap_con, int halt) {
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

static signed int conn_init(DAP_Connection* dap_con) {
	// Configure Clock - Set to 60Mhz internal oscillator (ISO)
	{
		uint32_t buffer;
		signed int retval;

		// Get GCR_CLKCTRL value
		retval = oper_read_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, &buffer);
		assert(retval == 0);

		// Ensure the 60Mhz oscillator (ISO) is enabled
		buffer &= 0x3F3F2FC0; // Remove reserved bits.
		buffer |= 0x00040000; // Set the 60Mhz oscillator (ISO) enable bit.
		retval = oper_write_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, buffer);
		assert(retval == 0);

		// Wait for the the 60 MHz oscillator (ISO) to be ready and get new value for GCR_CLKCTRL
		dprintf(STDOUT, "Wait for the ISO to be ready...");
		do {
			retval = oper_read_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, &buffer);
			assert(retval == 0);
		} while ((buffer & (1 << 26)) == 0);
		dprintf(STDOUT, "Done\n");

		// Set the the 60Mhz oscillator to SYS_OSC
		buffer &= 0x3F3F2FC0; // Remove reserved bits.
		buffer &= 0xFFFFFF3F; // Clear the clock divisor bits, reseting it to 0.
		buffer &= 0xFFFFF1FF; // Clear the clock source selection bits, reseting it to 0 (ISO).
		retval = oper_write_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, buffer);
		assert(retval == 0);

		// Wait for the the system clock to be ready
		dprintf(STDOUT, "Wait for the SYSCLK to be ready...");
		do {
			retval = oper_read_mem32(dap_con, GCR_ROOTADDR + GCR_CLKCTRL, &buffer);
			assert(retval == 0);
		} while ((buffer & (1 << 13)) == 0);
		dprintf(STDOUT, "Done\n");
	}

	return 0;
}
static signed int conn_destroy(DAP_Connection* dap_con) {
	return 0;
}

void chips_ff_max32690(DAP_Connection* dap_con) {
	dap_con->chip_pfns.chips_erase_flash_page = erase_flash_page;
	dap_con->chip_pfns.chips_write_to_flash_page = write_to_flash_page;
	dap_con->chip_pfns.chips_reset = reset;
	dap_con->chip_pfns.chips_conn_init = conn_init;
	dap_con->chip_pfns.chips_conn_destroy = conn_destroy;
	return;
}
