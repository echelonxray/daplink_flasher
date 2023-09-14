#include "dap.h"

#include "link.h"
#include <stdio.h>
#include <string.h>

#define LINK_BUFFER_LENGTH 64

signed int dap_info(DAP_Connection* dap_con, unsigned int id) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;

	memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_INFO;
	data[1] = id;

	retval = link_send_data(dap_con->device_handle, data, 2);
	if (retval < 0) {
		printf("Error: dap_info(): link_send_data() failed.\n");
		return -1;
	}

	retval = link_receive_data(dap_con->device_handle, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		printf("Error: dap_info(): link_receive_data() failed.\n");
		return -2;
	}

	if (data[0] != DAP_INFO) {
		printf("Error: dap_info(): Erroneous return data/structure.\n");
		return -3;
	}

	if (data[1] == 0) {
		printf("Error: dap_info(): No data returned.\n");
		return -4;
	}

	// TODO

	return 0;
}
signed int dap_connect(DAP_Connection* dap_con, unsigned int mode) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;

	memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_CONNECT;
	data[1] = mode;

	retval = link_send_data(dap_con->device_handle, data, 2);
	if (retval < 0) {
		printf("Error: dap_connect(): link_send_data() failed.\n");
		return -1;
	}

	retval = link_receive_data(dap_con->device_handle, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		printf("Error: dap_connect(): link_receive_data() failed.\n");
		return -2;
	}

	if (data[0] != DAP_CONNECT) {
		printf("Error: dap_connect(): Erroneous return data/structure.\n");
		return -3;
	}

	if (data[1] == 0) {
		printf("Error: dap_connect(): Connection initialization failed.\n");
		return -4;
	}

	if (data[1] != mode && mode != 0) {
		printf("Error: dap_connect(): Returned connection mode (%d) does not match requested mode.\n", data[1]);
		return -5;
	}

	return 0;
}
signed int dap_disconnect(DAP_Connection* dap_con) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;

	memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_DISCONNECT;

	retval = link_send_data(dap_con->device_handle, data, 1);
	if (retval < 0) {
		printf("Error: dap_disconnect(): link_send_data() failed.\n");
		return -1;
	}

	retval = link_receive_data(dap_con->device_handle, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		printf("Error: dap_disconnect(): link_receive_data() failed.\n");
		return -2;
	}

	if (data[0] != DAP_DISCONNECT) {
		printf("Error: dap_disconnect(): Erroneous return data/structure.\n");
		return -3;
	}
	if (data[1] == DAP_STATUS_ERROR) {
		printf("Error: dap_disconnect(): Returned failure status.\n");
		return -4;
	}

	return 0;
}
signed int dap_transfer(DAP_Connection* dap_con, unsigned int dap_index, unsigned int transfer_count, unsigned char* req_buffer, uint32_t* data_buffer) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;
	unsigned int i;
	unsigned int j;
	unsigned int k;

	memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_TRANSFER;
	data[1] = dap_index;
	data[2] = transfer_count;
	j = 3;
	k = 3;
	for (i = 0; i < transfer_count; i++) {
		if (j >= LINK_BUFFER_LENGTH) { // Don't overflow the TX buffer
			printf("Error: dap_transfer(): TX data buffer overflow. Type A.\n");
			return -5;
		}
		unsigned char request;
		request = req_buffer[i];
		data[j] = request;
		j++;
		if ((request & 0x02) == 0) { // Is TX request?
			if (j > LINK_BUFFER_LENGTH - 4) { // Don't overflow the TX buffer
				printf("Error: dap_transfer(): TX data buffer overflow. Type B.\n");
				return -5;
			}
			uint32_t tx32;
			tx32 = data_buffer[i];
			data[j + 0] = (tx32 >>  0) & 0xFF;
			data[j + 1] = (tx32 >>  8) & 0xFF;
			data[j + 2] = (tx32 >> 16) & 0xFF;
			data[j + 3] = (tx32 >> 24) & 0xFF;
			j += 4;
		} else {
			if (k > LINK_BUFFER_LENGTH - 4) { // Don't overflow the RX buffer
				printf("Error: dap_transfer(): RX data buffer overflow.\n");
				return -5;
			}
			k += 4;
		}
	}

	retval = link_send_data(dap_con->device_handle, data, j);
	if (retval < 0) {
		printf("Error: dap_transfer(): link_send_data() failed.\n");
		return -1;
	}

	retval = link_receive_data(dap_con->device_handle, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		printf("Error: dap_transfer(): link_receive_data() failed.\n");
		return -2;
	}

	if (data[0] != DAP_TRANSFER) {
		printf("Error: dap_transfer(): Erroneous return data/structure.\n");
		return -3;
	}
	if (data[1] != transfer_count) {
		printf("Error: dap_transfer(): Returned wrong transfer count: %d, expected: %d.\n", data[1], transfer_count);
		return -5;
	}
	if (data[2] != 0x01) {
		printf("Error: dap_transfer(): Returned failure status.\n");
		return -4;
	}

	k = 3;
	for (i = 0; i < transfer_count; i++) {
		unsigned char request;
		request = req_buffer[i];
		if (request & 0x02) { // Is RX request?
			uint32_t rx32;
			rx32 = 0;
			rx32 |= data[k + 3];
			rx32 <<= 8;
			rx32 |= data[k + 2];
			rx32 <<= 8;
			rx32 |= data[k + 1];
			rx32 <<= 8;
			rx32 |= data[k + 0];
			data_buffer[i] = rx32;
			k += 4;
		}
	}

	return 0;
}
signed int dap_swj_sequence(DAP_Connection* dap_con, unsigned int bit_count, unsigned char* data_buffer, unsigned int data_buffer_length) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;
	unsigned int i;

	memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_SWJ_SEQUENCE;
	data[1] = bit_count;
	for (i = 0; i < data_buffer_length; i++) {
		if (i > (LINK_BUFFER_LENGTH - (2 + 1))) { // Don't overflow the buffer [ (LINK_BUFFER_LENGTH - (PREAMBLE_LENGTH + CONVERT_TO_INDEX)) == MAX_BUFFER_INDEX ]
			printf("Error: dap_swj_sequence(): TX data buffer overflow.\n");
			return -5;
		}
		data[2 + i] = data_buffer[i];
	}

	retval = link_send_data(dap_con->device_handle, data, 2 + i);
	if (retval < 0) {
		printf("Error: dap_swj_sequence(): link_send_data() failed.\n");
		return -1;
	}

	retval = link_receive_data(dap_con->device_handle, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		printf("Error: dap_swj_sequence(): link_receive_data() failed.\n");
		return -2;
	}

	if (data[0] != DAP_SWJ_SEQUENCE) {
		printf("Error: dap_swj_sequence(): Erroneous return data/structure.\n");
		return -3;
	}
	if (data[1] == DAP_STATUS_ERROR) {
		printf("Error: dap_swj_sequence(): Returned failure status.\n");
		return -4;
	}

	return 0;
}
