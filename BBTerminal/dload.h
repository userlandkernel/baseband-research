//
//  dload.h
//  dloadtool
//
//  Created by Joshua Hill on 1/31/13.
//
//

#ifndef __dloadtool__dload__
#define __dloadtool__dload__

#include <stdio.h>
#include <IOKit/IOKitLib.h>
#include "dl-usb.h"

//Start      Code      Address            Size           CRC         End
//7e         0f        20 02 fe 00        01 00          ?? ??       7e
//7e         05        20 01 20 00                       9f 1f       7e

#define DLOAD WRITE                       0x01
#define DLOAD_ACK                         0x02
#define DLOAD_NAK                         0x03
#define DLOAD_ERASE                       0x04
#define DLOAD_EXECUTE                     0x05
#define DLOAD_NOP                         0x06
#define DLOAD_PARAM_REQ                   0x07
#define DLOAD_PARAM_RESP                  0x08
#define DLOAD_MEMORY_DUMP                 0x09
#define DLOAD_RESET                       0x0A
#define DLOAD_UNLOCK                      0x0B
#define DLOAD_SW_VER_REQ                  0x0C
#define DLOAD_SW_VERS_RESP                0x0D
#define DLOAD_POWERDOWN                   0x0E
#define DLOAD_WRITE_ADDR                  0x0F
#define DLOAD_MEMORY_DEBUG_QUERY          0x10  //Memory Debug Query
#define DLOAD_MEMORY_DEBUG_INFO           0x11  //Memory Debug Info
#define DLOAD_MEMORY_READ_REQ             0x12  //Memory Read Request
#define DLOAD_MEMORY_READ                 0x13  //Memory Read
#define DLOAD_MEMOY_UNFRAMED_READ_REQ     0x14  //Memory Unframed Read Request
#define DLOAD_SERIAL_NUMBER_READ_REQ      0x14  //Serial Number Read Request
#define DLOAD_SERIAL_NUMBER_READ_RESP     0x14  //Serial Number Read Response
#define DLOAD_MEMORY_UNFRAMED_READ_RESP   0x15  //Memory Unframed Read Response
//#define DLOAD_SERIAL_NUMBER_READ_REQ      0x16  //Serial Number Read Request
//#define DLOAD_SERIAL_NUMBER_READ_RESP     0x16

typedef struct {
    UInt16 code;
    UInt16 sequence;
    UInt8 unknown;
    UInt16 size;
} __attribute__((packed)) dload_firmware_header;

typedef struct {
    UInt8 code;
    UInt32 address;
    UInt16 size;
    UInt8 buffer[0];
} __attribute__((packed)) dload_write_addr;

typedef struct {
    UInt8 code;
    UInt32 address;
} __attribute__((packed)) dload_execute;

typedef struct {
    UInt8 code;
    UInt8 length;
    UInt8 version[0];
} __attribute__((packed)) dload_sw_version;

typedef struct {
    UInt8 code;
    UInt8 version;
    UInt8 min_version;
    UInt16 max_write;
    UInt8 model;
    UInt8 device_size;
    UInt8 device_type;
} __attribute__((packed)) dload_params;

int dload_get_params(USBInterface interface);
int dload_get_sw_version(USBInterface interface);
int dload_send_execute(USBInterface interface, UInt32 address);
int dload_upload_firmware(USBInterface interface, UInt32 address, const char* path);

int dload_read(USBInterface interface, UInt8* buffer, UInt32 size);
int dload_write(USBInterface interface, UInt8* buffer, UInt32 size);

int dload_request(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize);
int dload_response(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize);
int dload_escape(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize);
int dload_unescape(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize);

#endif /* defined(__dloadtool__dload__) */
