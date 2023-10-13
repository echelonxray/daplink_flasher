#ifndef _HEADERINC_MAIN_H
#define _HEADERINC_MAIN_H

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#define ERRH_MEMALLOC { dprintf(STDERR, "Error: Memory Allocation Failure.  File: \"%s\", Line: %d\n", __FILE__, __LINE__); exit(1); }
#define HELP_REM "Use command flags \"--help\", \"-h\", or \"-?\" to show help and command usage information.\n"
#define HELP_MSG "TODO: Help Message\n"

typedef struct {
    void* chips_erase_flash_page;
    void* chips_write_to_flash_page;
    void* chips_reset;
    void* chips_conn_init;
    void* chips_conn_destroy;
} ChipsPFNs;

typedef struct {
	void* device_handle;
	unsigned int sel_addr;
	ChipsPFNs chip_pfns;
} DAP_Connection;

#endif
