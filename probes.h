// SPDX-License-Identifier: 0BSD
/*
 * Copyright (C) 2023 Michael T. Kloos <michael@michaelkloos.com>
 */

#ifndef _HEADERINC_PROBES_H
#define _HEADERINC_PROBES_H

#include "main.h"
#include "dapctl.h"

signed int probes_find(DAP_Connection* dap_con, const char* probename);

#endif
