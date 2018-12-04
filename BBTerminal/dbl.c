//
//  dbl.cpp
//  dbltool
//
//  Created by Joshua Hill on 2/5/13.
//
//

#include <sys/socket.h>
#include <stdio.h>

#include "dbl.h"
#include "dbl-usb.h"
#include "dbl-sock.h"
#include "dbl-utils.h"

int dbl_send_params(USBInterface interface, dbl_param_req_t* request) {
    dbl_param_resp_t response;
    //02 00 00 00 // Command
    response.header.code = DBL_PARAM_RESP;
    //30 00 00 00 // Size
    response.header.size = sizeof(dbl_param_resp_t);
    //01 00 00 00 // 0x01??
    response.unknown1 = 0x1;
    //01 00 00 00 // 0x01??
    response.unknown2 = 0x1;
    //00 00 00 00 // 0x00??
    response.unknown3 = 0x0;
    //00 01 00 00 // 0x100??
    response.file = request->file;
    //00 00 00 00 // 0x0
    response.unknown5 = 0x0;
    //00 00 00 00 // 0x0
    response.unknown6 = 0x0;
    //00 00 00 00 // 0x0
    response.unknown7 = 0x0;
    //00 00 00 00 // 0x0
    response.unknown8 = 0x0;
    //00 00 00 00 // 0x0
    response.unknown9 = 0x0;
    //00 00 00 00 // 0x0
    response.unknown10 = 0x0;
    WriteBulk(interface, 2, &response, sizeof(response));
    hexdump((unsigned char*) &response, sizeof(response));
    return 0;
}

int dbl_send_memory(USBInterface interface, dbl_memory_req_t* request, const char* osbl, const char* amss) {
    FILE* fd = NULL;
    unsigned char* buffer = NULL;
    if(request->file == DBL_MEMORY_REQ_OSBL) {
        // Send OSBL data
        fd = fopen(osbl, "rb");
        if(fd) {
            buffer = (unsigned char*) malloc(request->length);
            if(buffer) {
                fseek(fd, request->offset, SEEK_SET);
                int x = (int)fread(buffer, 1, request->length, fd);
                if(x == request->length) {
                    WriteBulk(interface, 2, buffer, x);
                    //hexdump((unsigned char*) buffer, x);
                }
            }
            fclose(fd);
        }
    } else if(request->file == DBL_MEMORY_REQ_AMSS) {
        // Send AMSS data
        fd = fopen(amss, "rb");
        if(fd) {
            buffer = (unsigned char*) malloc(request->length);
            if(buffer) {
                fseek(fd, request->offset, SEEK_SET);
                int x = (int)fread(buffer, 1, request->length, fd);
                if(x == request->length) {
                    WriteBulk(interface, 2, buffer, x);
                    //hexdump((unsigned char*) buffer, x);
                }
            }
            fclose(fd);
        }
    } else {
        printf("Request from unknown file\n");
    }
    
    if(buffer != NULL) free(buffer);
    return 0;
}

int dbl_send_execute(USBInterface interface) {
    dbl_execute_req_t response;
    response.header.code = DBL_EXECUTE_REQ;
    response.header.size = sizeof(dbl_execute_req_t);
    WriteBulk(interface, 2, &response, sizeof(response));
    hexdump((unsigned char*) &response, sizeof(response));
    return 0;
}

int dbl_send_bbticket_params(USBInterface interface) {
    dbl_bbticket_resp_t response;
    response.header.code = DBL_BBTICKET_RESP;
    response.header.size = sizeof(dbl_bbticket_resp_t);
    response.unknown1 = 0x0;
    WriteBulk(interface, 2, &response, sizeof(response));
    hexdump((unsigned char*) &response, sizeof(response));
    return 0;
}

int dbl_send_bbticket_memory(USBInterface interface, dbl_bbticket_data_req_t* request, const char* bbticket) {
    if(request->bbchipid != 0 && request->bbsernum != 0) {
        printf("Found a NONCE, please connect\n");
        
        int x = 0;
        int s = sock_listen(10000);
        if(s > 0) {
            x = sock_accept_connection(s);
            if(x > 0) {
                hexdump(request->nonce, sizeof(request->nonce));
                sock_send(x, request->nonce, sizeof(request->nonce));
                printf("Nonce sent\n");
                
                unsigned int size = 0;
                unsigned char data[0x2000];
                size = sock_recv(x, data, sizeof(data));
                printf("BBTicket received\n");
                printf("Got data of size 0x%x\n", size);
                
                // And then upload it to the baseband
                dbl_bbticket_data_resp_t* response = NULL;
                response = (dbl_bbticket_data_resp_t*) malloc(sizeof(dbl_bbticket_data_resp_t) + size);
                response->header.code = DBL_BBTICKET_DATA_RESP;
                response->header.size = sizeof(dbl_bbticket_data_resp_t) + size;
                memcpy(response->data, data, size);
                WriteBulk(interface, 2, response, response->header.size);
                hexdump((unsigned char*) response, response->header.size);
            }
        }
        
    } else {
        unsigned int size = 0;
        unsigned char data[0x2000];
        dbl_bbticket_data_resp_t* response = NULL;
        FILE* fd = fopen(bbticket, "r");
        if(fd) {
            size = (unsigned int)fread(data, 1, 0x2000, fd);
            if(size > 0) {
                response = (dbl_bbticket_data_resp_t*) malloc(sizeof(dbl_bbticket_data_resp_t) + size);
                response->header.code = DBL_BBTICKET_DATA_RESP;
                response->header.size = sizeof(dbl_bbticket_data_resp_t) + size;
                memcpy(response->data, data, size);
                WriteBulk(interface, 2, response, response->header.size);
                hexdump((unsigned char*) response, response->header.size);
            }
            fclose(fd);
        }
    }
    return 0;
}

int dbl_send_bbticket_execute(USBInterface interface) {
    dbl_bbticket_execute_resp_t response;
    response.header.code = DBL_BBTICKET_EXECUTE_RESP;
    response.header.size = sizeof(dbl_bbticket_execute_resp_t);
    response.result = 0x1;
    WriteBulk(interface, 2, &response, sizeof(response));
    hexdump((unsigned char*) &response, sizeof(response));
    return 0;
}
