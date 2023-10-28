#include "main.h"
#include "probes.h"
#include "errors.h"
#include <string.h>

typedef void (*ProbesFF)(DAP_Connection* dap_con);
void probes_ff_max32625pico(DAP_Connection* dap_con);
static ProbesFF probe_ff_functions[] = {
    probes_ff_max32625pico,
};
static char* probe_names[] = {
    "max32625pico",
};
signed int probes_find(DAP_Connection* dap_con, const char* probename) {
    size_t array_length = sizeof(probe_names) / sizeof(*probe_names);
    for (size_t i = 0; i < array_length; i++) {
        if (strcmp(probename, probe_names[i]) == 0) {
            ProbesFF probes_ff = probe_ff_functions[i];
            probes_ff(dap_con);
            return SUCCESS_STATUS;
        }
    }
    return ERROR_NOMATCHDEV;
}
