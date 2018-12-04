//
//  util.h
//  usblogger
//
//  Created by Joshua Hill on 1/29/13.
//
//

#ifndef usblogger_util_h
#define usblogger_util_h

#include <sys/types.h>


typedef u_int8_t boolean;
#ifndef TRUE
#define TRUE ((u_int8_t) 1)
#endif
#ifndef FALSE
#define FALSE ((u_int8_t) 0)
#endif

extern void hexdump (unsigned char *data, unsigned int amount);

uint16_t flip_endian16(uint16_t value);
uint32_t flip_endian32(uint32_t value);

u_int16_t dm_crc16 (const char *buffer, size_t len);


#endif
