//
//  dload.cpp
//  dloadtool
//
//  Created by Joshua Hill on 1/31/13.
//
//


#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dl-usb.h"
#include "dl-util.h"
#include "dload.h"

int dload_get_params(USBInterface interface) {
    UInt8 output[0x100];
    UInt8 request = 0x0;
    dload_params* response = NULL;
    
    request = DLOAD_PARAM_REQ;
    memset(output, '\0', sizeof(output));
    dload_write(interface, &request, sizeof(request));
    dload_read(interface, output, sizeof(output));
    if(output[0] == DLOAD_PARAM_RESP) {
        response = (dload_params*) output;
        printf("Protocol Version: 0x%hhx\n", response->version);
        printf("Min Protocol Version: 0x%hhx\n", response->min_version);
        printf("Max Write Size: 0x%hx\n", flip_endian16(response->max_write));
        printf("Model: 0x%hhx\n", response->model);
        printf("Device Size: 0x%hhx\n", response->device_size);
        printf("Device Type: 0x%hhx\n", response->device_type);
        
    } else {
        fprintf(stderr, "Error receiving software parameters!!\n");
        return -1;
    }
    return 0;
}

int dload_get_sw_version(USBInterface interface) {
    UInt8 output[0x100];
    UInt8 request = 0x0;
    dload_sw_version* response = NULL;
    
    request = DLOAD_SW_VER_REQ;
    memset(output, '\0', sizeof(output));
    dload_write(interface, &request, sizeof(request));
    dload_read(interface, output, sizeof(output));
    if(output[0] == DLOAD_SW_VERS_RESP) {
        response = (dload_sw_version*) output;
        printf("Software Version: %s\n", response->version);
    } else {
        fprintf(stderr, "Error receiving software version!!\n");
        return -1;
    }
    return 0;
}

//Frame      Code      Address                Size
//           0x0f       0x20012000            0x100
//7e         0x05       0x20012000
unsigned char done[] = "\x7e\x05\x20\x01\x20\x00\x9f\x1f\x7e";
int dload_upload_firmware(USBInterface interface, UInt32 address, const char* path) {
    FILE* fd = NULL;
    UInt8 input[0x100];
    UInt8 output[0x100];
    UInt32 addr = 0x20012000;
    
    dload_write_addr* packet = (dload_write_addr*) malloc(sizeof(dload_write_addr) + sizeof(output));
    fd = fopen(path, "rb");
    if(fd != NULL) {
        printf("Opened firmware at %s\n", path);
        while(!feof(fd)) {
            memset(input, '\0', sizeof(input));
            memset(output, '\0', sizeof(output));
            size_t len = fread(output, 1, sizeof(output), fd);
            printf("Read buffer of size %zd\n", len);
            if(len != 0) {
                memset(packet, '\0', sizeof(dload_write_addr) + sizeof(output));
                memcpy(packet->buffer, output, sizeof(output));
                packet->code = DLOAD_WRITE_ADDR;
                packet->size = flip_endian16(len);
                packet->address = flip_endian32(addr);
                
                dload_write(interface, (UInt8*) packet, (unsigned int)(sizeof(dload_write_addr) + len));
                dload_read(interface, input, sizeof(input));
                if(input[0] != 0x2) {
                    fprintf(stderr, "Error 0x%hhx!!!\n", input[0]);
                    free(packet);
                    return -1;
                }
                addr += len;
                
            }
        }
        
        fclose(fd);
    }
    free(packet);
    
    return 0;
}

int dload_send_execute(USBInterface interface, UInt32 address) {
    UInt8 input[0x100];
    memset(input, '\0', sizeof(input));
    
    dload_execute* request = (dload_execute*) malloc(sizeof(dload_execute));
    request->code = DLOAD_EXECUTE;
    request->address = flip_endian32(address);
    
    dload_write(interface, (UInt8*) request, sizeof(dload_execute));
    dload_read(interface, input, sizeof(input));
    
    free(request);
    return 0;
}

int dload_read(USBInterface interface, UInt8* buffer, UInt32 size) {
    UInt32 insize = 0;
    UInt32 outsize = 0;
    UInt8* inbuf = NULL;
    UInt8* outbuf = NULL;
    
    insize = size;
    inbuf = (UInt8*) malloc(size);
    
    printf("Recv:\n");
    ReadBulk(interface, 1, inbuf, &insize);
    if(insize > 0) {
        dload_response(inbuf, insize, &outbuf, &outsize);
        if(outsize <= size) {
            hexdump(outbuf, outsize);
            memcpy(buffer, outbuf, outsize);
        }
    }
    
    free(inbuf);
    free(outbuf);
    return outsize;
}

int dload_write(USBInterface interface, UInt8* buffer, UInt32 size) {
    UInt32 outsize = 0;
    UInt8* outbuf = NULL;
    
    printf("Send:\n");
    dload_request(buffer, size, &outbuf, &outsize);
    if(outsize > 0) {
        WriteBulk(interface, 2, outbuf, outsize);
        //hexdump(buffer, size);
    }
    
    free(outbuf);
    return outsize;
}

int dload_request(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize) {
    UInt32 size = 0;
    UInt8* inbuf = NULL;
    UInt8* outbuf = NULL;
    UInt8* buffer = NULL;
    unsigned short crc = 0;
    
    inbuf = (UInt8*) malloc(insize+2); // plus 2 for the crc
    memset(inbuf, '\0', insize+2);
    memcpy(inbuf, input, insize); // copy the original data into our buffer with the crc
    
    crc = dm_crc16 ((const char*) input, insize); // perform the crc or the original data
    inbuf[insize] = crc & 0xFF; // add first byte of crc
    inbuf[insize+1] = (crc >> 8) & 0xFF; // add second byte of crc
    
    dload_escape(inbuf, insize+2, &outbuf, &size); // escape all control and escape characters
    
    buffer = (UInt8*) malloc(size+2); // plus 2 for start and end control characters
    memset(buffer, '\0', size+2);
    memcpy(&buffer[1], outbuf, size); // copy our crc'd and escaped characters into final buffer
    
    buffer[0] = 0x7E; // Add our beginning control character
    buffer[size+1] = 0x7E; // Add out ending control character
    
    free(inbuf); // We don't need this anymore
    free(outbuf); // We don't need this anymore
    
    *output = buffer;
    *outsize = size+2;
    return 0;
}

int dload_response(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize) {
    UInt32 size = 0;
    UInt8* outbuf = NULL;
    dload_unescape(input, insize, &outbuf, &size);
    
    unsigned short crc = dm_crc16((const char*) &outbuf[1], size-4);
    unsigned short chk = *((unsigned short*) &outbuf[size-3]);
    if(crc != chk) {
        fprintf(stderr, "Invalid CRC!!!\n");
        size += 2;
    }
    UInt8* buffer = (UInt8*) malloc(size-4);
    memset(buffer, '\0', size-4);
    memcpy(buffer, &outbuf[1], size-4);
    free(outbuf);
    
    *output = buffer;
    *outsize = size-4;
    return 0;
}

int dload_escape(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize) {
    int i = 0;
    unsigned int size = 0;
    for(i = 0; i < insize; i++) {
        if(input[i] == 0x7E || input[i] == 0x7D) {
            size++;
        }
        size++;
    }
    
    int o = 0;
    unsigned char* buffer = NULL;
    buffer = (unsigned char*) malloc(size);
    memset(buffer, '\0', size);
    for(i = 0; i < insize; i++) {
        if(input[i] == 0x7E) {
            buffer[o] = 0x7D;
            buffer[o+1] = 0x7E ^ 0x20;
            o++;
        } else if(input[i] == 0x7D) {
            buffer[o] = 0x7D;
            buffer[o+1] = 0x7D ^ 0x20;
            o++;
        } else {
            buffer[o] = input[i];
        }
        o++;
    }
    
    *outsize = size;
    *output = buffer;
    return 0;
}

int dload_unescape(UInt8* input, UInt32 insize, UInt8** output, UInt32* outsize) {
    int i = 0;
    unsigned int size = insize;
    for(i = insize; i >= 0; i--) {
        if(input[i] == 0x7D) size--;
    }
    
    int o = 0;
    unsigned char* buffer = NULL;
    buffer = (unsigned char*) malloc(size);
    memset(buffer, '\0', size);
    for(i = 0; i <= insize; i++) {
        if(input[i] == 0x7D) {
            buffer[o] = input[i+1] ^ 0x20;
            i++;
        } else {
            buffer[o] = input[i];
        }
        o++;
    }
    
    *outsize = size;
    *output = buffer;
    return 0;
}
