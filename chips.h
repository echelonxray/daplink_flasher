#ifndef _HEADERINC_CHIPS_H
#define _HEADERINC_CHIPS_H

#include "main.h"
#include <stdint.h>

typedef signed int (*ChipsEraseFlashPage_PFN)(DAP_Connection* dap_con,
                                              uint32_t address);
typedef signed int (*ChipsWriteToFlashPage_PFN)(DAP_Connection* dap_con,
                                                uint32_t address,
                                                unsigned char* data);

typedef signed int (*ChipsReset_PFN)(DAP_Connection* dap_con,
                                     int halt);

typedef signed int (*ChipsConnInit_PFN)(DAP_Connection* dap_con);
typedef signed int (*ChipsConnDestroy_PFN)(DAP_Connection* dap_con);

signed int chip_erase_flash_page(DAP_Connection* dap_con, uint32_t address);
signed int chip_write_to_flash_page(DAP_Connection* dap_con, uint32_t address, unsigned char* data);

signed int chip_reset(DAP_Connection* dap_con, int halt);

signed int chip_conn_init(DAP_Connection* dap_con);
signed int chip_conn_destroy(DAP_Connection* dap_con);

signed int chips_find(DAP_Connection* dap_con, char* chipname);

#endif
