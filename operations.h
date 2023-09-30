#ifndef _HEADERINC_OPERATIONS_H
#define _HEADERINC_OPERATIONS_H

#include "dap.h"
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#define OPER_REG_ACCESSMASK     (0x100)

#define OPER_REG_DEBUG_IDCODE    (0x00)
#define OPER_REG_DEBUG_ABORT     (0x00)
#define OPER_REG_DEBUG_CTRLSTAT  (0x04)
#define OPER_REG_DEBUG_RESEND    (0x08)
#define OPER_REG_DEBUG_SELECT    (0x08)
#define OPER_REG_DEBUG_RDBUFF    (0x0C)

#define OPER_REG_ACCESS_CSW      (0x00 | OPER_REG_ACCESSMASK)
#define OPER_REG_ACCESS_TAR      (0x04 | OPER_REG_ACCESSMASK)
#define OPER_REG_ACCESS_DRW      (0x0C | OPER_REG_ACCESSMASK)
#define OPER_REG_ACCESS_IDR      (0xFC | OPER_REG_ACCESSMASK)

#define OPER_REG_ACCESS_CSWMODE_08BITADDR (0x00)
#define OPER_REG_ACCESS_CSWMODE_16BITADDR (0x01)
#define OPER_REG_ACCESS_CSWMODE_32BITADDR (0x02)

#define OPER_REG_ACCESS_CSWMODE_NOAUTOINC (0x00)
#define OPER_REG_ACCESS_CSWMODE_AUTOINC   (0x10)

signed int oper_write_reg(DAP_Connection* dap_con, unsigned int reg, uint32_t value);
signed int oper_read_reg(DAP_Connection* dap_con, unsigned int reg, uint32_t* buffer);

signed int oper_write_mem8(DAP_Connection* dap_con, uint32_t address, uint8_t value);
signed int oper_read_mem8(DAP_Connection* dap_con, uint32_t address, uint8_t* buffer);

signed int oper_write_mem16(DAP_Connection* dap_con, uint32_t address, uint16_t value);
signed int oper_read_mem16(DAP_Connection* dap_con, uint32_t address, uint16_t* buffer);

signed int oper_write_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t value);
signed int oper_read_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t* buffer);

signed int oper_write_memblock32(DAP_Connection* dap_con, uint32_t address, uint32_t* values, uint32_t buffer_length);
signed int oper_read_memblock32(DAP_Connection* dap_con, uint32_t address, uint32_t* buffer, uint32_t buffer_length);

signed int oper_erase_flash_page(DAP_Connection* dap_con, uint32_t address, uint32_t page_size, uint32_t controller_address);
signed int oper_write_to_flash_page(DAP_Connection* dap_con, uint32_t address, uint32_t controller_address,
                                    uint32_t data_0, uint32_t data_1, uint32_t data_2, uint32_t data_3);

signed int oper_reset(DAP_Connection* dap_con, int halt);

signed int oper_init(DAP_Connection** dap_con, libusb_device_handle* d_handle);
signed int oper_destroy(DAP_Connection* dap_con);

#endif
