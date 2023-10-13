#ifndef _HEADERINC_LINK_H
#define _HEADERINC_LINK_H

#include "../main.h"

void link_print_bytes(unsigned char* data, unsigned int length);
signed int link_send_data(void* device_handle, unsigned char* data, unsigned int length);
signed int link_receive_data(void* device_handle, unsigned char* data, unsigned int length);

#endif
