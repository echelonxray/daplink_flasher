#ifndef _HEADERINC_ERROR_H
#define _HEADERINC_ERROR_H

#include "main.h"

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

#endif
