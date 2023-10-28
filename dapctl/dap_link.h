#ifndef _HEADERINC_DAPCTL_LINK_H
#define _HEADERINC_DAPCTL_LINK_H

#include "../main.h"
#include "../dapctl.h"
#include <stdint.h>

//void link_print_bytes(unsigned char* data, unsigned int length);
signed int link_send_data(DAP_Connection* dap_con, unsigned char* data, unsigned int length);
signed int link_receive_data(DAP_Connection* dap_con, unsigned char* data, unsigned int length);
signed int link_find_and_connect(DAP_Connection* dap_con, uint16_t usb_vid, uint16_t usb_pid);
signed int link_disconnect(DAP_Connection* dap_con);

#endif
