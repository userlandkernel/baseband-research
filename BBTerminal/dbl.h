//
//  dbl.h
//  dbltool
//
//  Created by Joshua Hill on 2/5/13.
//
//

#ifndef __dbltool__dbl__
#define __dbltool__dbl__

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>

#include "dbl-usb.h"

#define DBL_PARAM_REQ     0x01
#define DBL_PARAM_RESP    0x02
#define DBL_MEMORY_REQ    0x03
#define DBL_MEMORY_RESP   0x04
#define DBL_EXECUTE_REQ   0x05
#define DBL_EXECUTE_RESP  0x06

#define DBL_BBTICKET_REQ           0xa0
#define DBL_BBTICKET_RESP          0xa1
#define DBL_BBTICKET_DATA_REQ      0xa2
#define DBL_BBTICKET_DATA_RESP     0xa3
#define DBL_BBTICKET_RESULT        0xa4
#define DBL_BBTICKET_EXECUTE_REQ   0xa5
#define DBL_BBTICKET_EXECUTE_RESP  0xa6

#define DBL_MEMORY_REQ_AMSS 0x02
#define DBL_MEMORY_REQ_OSBL 0x0b

typedef struct {
    unsigned int code;
    unsigned int size;
} dbl_header_t;

typedef struct { // 0x01
    dbl_header_t header;
    unsigned int unknown1;  //02 00 00 00 // 0x02??
    unsigned int unknown2;  //01 00 00 00 // 0x01??
    unsigned int unknown3;  //00 40 00 00 // 0x4000??
    unsigned int file;      //00 01 00 00 // 0x100??
    unsigned int unknown5;  //02 00 00 00 // 0x02??
    unsigned int unknown6;  //00 00 00 00 // 0x0??
    unsigned int unknown7;  //00 00 00 00 // 0x0??
    unsigned int unknown8;  //00 00 00 00 // 0x0??
    unsigned int unknown9;  //00 00 00 00 // 0x0??
    unsigned int unknown10; //00 00 00 00 // 0x0??
} dbl_param_req_t;

typedef struct { // 0x02
    dbl_header_t header;
    unsigned int unknown1;  //01 00 00 00 // 0x01??
    unsigned int unknown2;  //01 00 00 00 // 0x01??
    unsigned int unknown3;  //00 00 00 00 // 0x00??
    unsigned int file;      //00 01 00 00 // 0x100??
    unsigned int unknown5;  //00 00 00 00 // 0x0
    unsigned int unknown6;  //00 00 00 00 // 0x0
    unsigned int unknown7;  //00 00 00 00 // 0x0
    unsigned int unknown8;  //00 00 00 00 // 0x0
    unsigned int unknown9;  //00 00 00 00 // 0x0
    unsigned int unknown10; //00 00 00 00 // 0x0
} dbl_param_resp_t;

typedef struct { // 0x03
    dbl_header_t header;
    unsigned int file;
    unsigned int offset;
    unsigned int length;
} dbl_memory_req_t;

typedef struct { // 0x04
    dbl_header_t header;
    unsigned int file; //0b 00 00 00 // File??
    unsigned int response; //00 00 00 00
} dbl_memory_resp_t;

typedef struct { // 0x05
    dbl_header_t header;
} dbl_execute_req_t;

typedef struct { // 0x06
    dbl_header_t header;
    unsigned int file; //00 01 00 00 // 0x100 = BBTicket, 0x0 = OSBL, 0x01 = AMSS ??
} dbl_execute_resp_t;

typedef struct { // 0xa0
    dbl_header_t header;
} dbl_bbticket_req_t;

typedef struct { // 0xa1
    dbl_header_t header;
    unsigned int unknown1;
} dbl_bbticket_resp_t;

typedef struct { // 0xa2
    dbl_header_t header;
    unsigned int unknown1;
    unsigned char nonce[20];
    unsigned int bbsernum;
    unsigned int bbchipid;
} dbl_bbticket_data_req_t;

typedef struct { // 0xa3
    dbl_header_t header;
    unsigned char data[0];
} dbl_bbticket_data_resp_t;

typedef struct { // 0xa4
    dbl_header_t header;
} dbl_bbticket_result_t;

typedef struct { // 0xa5
    dbl_header_t header;
    
    unsigned int unknown1; //00 00 00 00
    unsigned int unknown2; //00 00 00 00
    unsigned int unknown3; //00 00 00 00
    unsigned int unknown4; //00 00 00 00
    unsigned int unknown5; //00 00 00 00
    unsigned int unknown6; //00 00 00 00
    unsigned int unknown7; //00 00 00 00
    unsigned int unknown8; //00 00 00 00
    unsigned int unknown9; //00 00 00 00
    unsigned int unknown10; //00 00 00 00
    unsigned int unknown11; //00 00 00 00
    unsigned int unknown12; //00 00 00 00
    unsigned int unknown13; //00 00 00 00
    unsigned int unknown14; //00 00 00 00
    unsigned int unknown15; //00 00 00 00
} dbl_bbticket_execute_req_t;

typedef struct { // 0xa6
    dbl_header_t header;
    unsigned int result;
} dbl_bbticket_execute_resp_t;

int dbl_send_params(USBInterface interface, dbl_param_req_t* request);
int dbl_send_memory(USBInterface interface, dbl_memory_req_t* request, const char* osbl, const char* amss);
int dbl_send_execute(USBInterface interface);
int dbl_send_bbticket_params(USBInterface interface);
int dbl_send_bbticket_memory(USBInterface interface, dbl_bbticket_data_req_t* request, const char* bbticket);
int dbl_send_bbticket_execute(USBInterface interface);

#endif /* defined(__dbltool__dbl__) */
