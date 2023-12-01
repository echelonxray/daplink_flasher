// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2023 Michael T. Kloos <michael@michaelkloos.com>
 */

#include "../main.h"
#include "dap_cmds.h"
#include "../errors.h"
#include <stdio.h>
//#include <string.h>

#define LINK_BUFFER_LENGTH 64

signed int dap_info(DAP_Connection* dap_con, unsigned int id) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;

	//memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_INFO;
	data[1] = id;

	retval = link_send_data(dap_con, data, 2);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	retval = link_receive_data(dap_con, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	if (data[0] != DAP_INFO) {
		PRINT_ERR("Erroneous return data/structure: 0x%02X.", (unsigned int)data[0]);
		return ERROR_DEV_CMD_ERRONEOUS;
	}

	if (data[1] == 0) {
		PRINT_ERR("No data returned.");
		return ERROR_DEV_CMD_ERRONEOUS;
	}

	// TODO

	return SUCCESS_STATUS;
}
signed int dap_connect(DAP_Connection* dap_con, unsigned int mode) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;

	//memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_CONNECT;
	data[1] = mode;

	retval = link_send_data(dap_con, data, 2);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	retval = link_receive_data(dap_con, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	if (data[0] != DAP_CONNECT) {
		PRINT_ERR("Erroneous return data/structure: 0x%02X.", (unsigned int)data[0]);
		return ERROR_DEV_CMD_ERRONEOUS;
	}

	if (data[1] == 0) {
		PRINT_ERR("DAP Connection Initialization failed.");
		return ERROR_DEV_CMD_FAILED;
	}

	if (data[1] != mode && mode != 0) {
		PRINT_ERR("Returned connection mode (0x%02X) does not match requested mode.", (unsigned int)data[1]);
		return ERROR_DEV_CMD_ERRONEOUS;
	}

	return SUCCESS_STATUS;
}
signed int dap_disconnect(DAP_Connection* dap_con) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;

	//memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_DISCONNECT;

	retval = link_send_data(dap_con, data, 1);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	retval = link_receive_data(dap_con, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	if (data[0] != DAP_DISCONNECT) {
		PRINT_ERR("Erroneous return data/structure: 0x%02X.", (unsigned int)data[0]);
		return ERROR_DEV_CMD_ERRONEOUS;
	}

	if (data[1] == DAP_STATUS_ERROR) {
		PRINT_ERR("DAP Disconnection failed.");
		return ERROR_DEV_CMD_FAILED;
	}

	return SUCCESS_STATUS;
}
signed int dap_transfer(DAP_Connection* dap_con, unsigned int dap_index, unsigned int transfer_count, unsigned char* req_buffer, uint32_t* data_buffer) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;
	unsigned int i;
	unsigned int j;
	unsigned int k;

	//memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_TRANSFER;
	data[1] = dap_index;
	data[2] = transfer_count;
	j = 3;
	k = 3;
	for (i = 0; i < transfer_count; i++) {
		if (j >= LINK_BUFFER_LENGTH) { // Don't overflow the TX buffer
			PRINT_ERR("TX data buffer overflow.");
			return ERROR_MALFORMED_INPUT;
		}
		unsigned char request;
		request = req_buffer[i];
		data[j] = request;
		j++;
		if ((request & 0x02) == 0) { // Request Type?
			// TX request?
			if (j > LINK_BUFFER_LENGTH - 4) { // Don't overflow the TX buffer
				PRINT_ERR("TX data buffer overflow.");
				return ERROR_MALFORMED_INPUT;
			}
			uint32_t tx32;
			tx32 = data_buffer[i];
			data[j + 0] = (tx32 >>  0) & 0xFF;
			data[j + 1] = (tx32 >>  8) & 0xFF;
			data[j + 2] = (tx32 >> 16) & 0xFF;
			data[j + 3] = (tx32 >> 24) & 0xFF;
			j += 4;
		} else {
			// RX request
			if (k > LINK_BUFFER_LENGTH - 4) { // Don't overflow the RX buffer
				PRINT_ERR("RX data buffer overflow.");
				return ERROR_MALFORMED_INPUT;
			}
			k += 4;
		}
	}

	retval = link_send_data(dap_con, data, j);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	retval = link_receive_data(dap_con, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	if (data[0] != DAP_TRANSFER) {
		PRINT_ERR("Erroneous return data/structure: 0x%02X.", (unsigned int)data[0]);
		return ERROR_DEV_CMD_ERRONEOUS;
	}
	if (data[1] != transfer_count) {
		PRINT_ERR("Returned wrong transfer count: %u, expected: %u.", (unsigned int)data[1], transfer_count);
		return ERROR_DEV_CMD_ERRONEOUS;
	}
	if (data[2] != 0x01) {
		PRINT_ERR("DAP Transfer failed.");
		return ERROR_DEV_CMD_FAILED;
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

	return SUCCESS_STATUS;
}
signed int dap_swj_sequence(DAP_Connection* dap_con, unsigned int bit_count, unsigned char* data_buffer, unsigned int data_buffer_length) {
	unsigned char data[LINK_BUFFER_LENGTH];
	signed int retval;
	unsigned int i;

	//memset(data, 0, LINK_BUFFER_LENGTH);
	data[0] = DAP_SWJ_SEQUENCE;
	data[1] = bit_count;
	for (i = 0; i < data_buffer_length; i++) {
		if (i > (LINK_BUFFER_LENGTH - (2 + 1))) { // Don't overflow the buffer [ (LINK_BUFFER_LENGTH - (PREAMBLE_LENGTH + CONVERT_TO_INDEX)) == MAX_BUFFER_INDEX ]
			PRINT_ERR("TX data buffer overflow.");
			return ERROR_MALFORMED_INPUT;
		}
		data[2 + i] = data_buffer[i];
	}

	retval = link_send_data(dap_con, data, 2 + i);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	retval = link_receive_data(dap_con, data, LINK_BUFFER_LENGTH);
	if (retval < 0) {
		RELAY_RETURN(retval);
	}

	if (data[0] != DAP_SWJ_SEQUENCE) {
		PRINT_ERR("Erroneous return data/structure: 0x%02X.", (unsigned int)data[0]);
		return ERROR_DEV_CMD_ERRONEOUS;
	}
	if (data[1] == DAP_STATUS_ERROR) {
		PRINT_ERR("DAP SWJ Sequence failed.");
		return ERROR_DEV_CMD_FAILED;
	}

	return SUCCESS_STATUS;
}
