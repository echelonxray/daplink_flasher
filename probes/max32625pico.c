// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2023 Michael T. Kloos <michael@michaelkloos.com>
 */

#include "../main.h"
#include "../dapctl.h"

void probes_ff_max32625pico(DAP_Connection* dap_con) {
	dap_con->usbvid_str = "0x0D28";
	dap_con->usbpid_str = "0x0204";
	return;
}
