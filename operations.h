#ifndef _HEADERINC_OPERATIONS_H
#define _HEADERINC_OPERATIONS_H

#include "dap.h"
#include <libusb-1.0/libusb.h>

signed int oper_write_reg(DAP_Connection* dap_con, unsigned int reg, unsigned int value);
signed int oper_read_reg(DAP_Connection* dap_con, unsigned int reg, unsigned int* buffer);

signed int oper_write_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t value);
signed int oper_read_mem32(DAP_Connection* dap_con, uint32_t address, uint32_t* buffer);

signed int oper_init(DAP_Connection** dap_con, libusb_device_handle* d_handle);
signed int oper_destroy(DAP_Connection* dap_con);

#endif
