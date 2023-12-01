#ifndef _HEADERINC_ERROR_H
#define _HEADERINC_ERROR_H

#include "main.h"

// Error Codes ------------------------------------
#define SUCCESS_STATUS           0x00 // Success.  No Error.

#define ERROR_LIBUSB_SETUP      -0x01 // Error during LIBUSB init, device discovery, or setup
#define ERROR_LIBUSB_TXDEV      -0x02 // Error sending data to USB device
#define ERROR_LIBUSB_RXDEV      -0x03 // Error receiving data from USB device
#define ERROR_LIBUSB_CLEAN      -0x04 // Error cleaning up LIBUSB connection to device

#define ERROR_MALFORMED_INPUT   -0x05 // Error parameters passed to function or program failed a sanity check
#define ERROR_DEV_CMD_FAILED    -0x06 // Error request sent to device returned an error
#define ERROR_DEV_CMD_ERRONEOUS -0x07 // Error request sent to device returned an unexpected response

#define ERROR_NOMATCHDEV        -0x08 // Error no matching device was found

#define ERROR_MEMALLOC          -0x09 // Error memory allocation failure

#define ERROR_UNSPECIFIED       -0x0A // Error general or unknown failure
#define ERROR_UNIMPL            -0x0B // Error Feature not implemented.  The condition encountered is known, 
                                      //       but the functionality needed to handle it is not in place.

#define ERROR_FILEIO            -0x0C // Error preforming file operation. (open, close, read, write, fstat)
// ------------------------------------------------

// Error Strings ----------------------------------
#define ERRSTR_MEMALLOC           "Memory Allocation Failure."
// ------------------------------------------------

// Error Macros -----------------------------------
#define PRINT_ERR(format_str, ...) { dprintf(STDERR, "Error [Func: \"%s()\", File: \"%s\", Line: %d]: " format_str "\n", __func__, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); }
#define RELAY_RETURN(return_code) { dprintf(STDERR, " ... in Func: \"%s()\", File: \"%s\", Line: %d", __func__, __FILE__, __LINE__); return return_code; }
// ------------------------------------------------

#endif
