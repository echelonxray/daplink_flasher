#include "main.h"
#include <stdio.h>
#include <string.h>
#include "dapctl/dap_oper.h"
#include "chips.h"

signed int chip_erase_flash_page(DAP_Connection* dap_con, uint32_t address) {
    ChipsEraseFlashPage_PFN chips_erase_flash_page;
    chips_erase_flash_page = dap_con->chip_pfns.chips_erase_flash_page;
    return chips_erase_flash_page(dap_con, address);
}
signed int chip_write_to_flash_page(DAP_Connection* dap_con, uint32_t address, unsigned char* data) {
    ChipsWriteToFlashPage_PFN chips_write_to_flash_page;
    chips_write_to_flash_page = dap_con->chip_pfns.chips_write_to_flash_page;
    return chips_write_to_flash_page(dap_con, address, data);
}

signed int chip_reset(DAP_Connection* dap_con, int halt){
    ChipsReset_PFN chips_reset;
    chips_reset = dap_con->chip_pfns.chips_reset;
    return chips_reset(dap_con, halt);
}

signed int chip_conn_init(DAP_Connection* dap_con) {
    ChipsConnInit_PFN chips_conn_init;
    chips_conn_init = dap_con->chip_pfns.chips_conn_init;
    return chips_conn_init(dap_con);
}
signed int chip_conn_destroy(DAP_Connection* dap_con) {
    ChipsConnDestroy_PFN chips_conn_destroy;
    chips_conn_destroy = dap_con->chip_pfns.chips_conn_destroy;
    return chips_conn_destroy(dap_con);
}

void chips_ff_max32690(DAP_Connection* dap_con);
typedef void (*ChipsFF)(DAP_Connection* dap_con);
static ChipsFF chip_ff_functions[] = {
    chips_ff_max32690,
};
static char* chip_names[] = {
    "max32690",
};
signed int chips_find(DAP_Connection* dap_con, char* chipname) {
    size_t array_length = sizeof(chip_names) / sizeof(*chip_names);
    for (size_t i = 0; i < array_length; i++) {
        if (strcmp(chipname, chip_names[i]) == 0) {
            ChipsFF chips_ff = chip_ff_functions[i];
            chips_ff(dap_con);
            return 0;
        }
    }
    return -1;
}
