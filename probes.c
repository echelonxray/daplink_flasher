#include "main.h"
#include "probes.h"
#include <string.h>

static char* probe_names[] = {
    "max32625pico",
};
signed int probes_find(DAP_Connection* dap_con, char* probename) {
    size_t array_length = sizeof(probe_names) / sizeof(*probe_names);
    for (size_t i = 0; i < array_length; i++) {
        if (strcmp(probename, probe_names[i]) == 0) {
            return 0;
        }
    }
    return -1;
}
