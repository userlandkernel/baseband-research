//
//  main.cpp
//  dloadtool
//
//  Created by Joshua Hill on 1/30/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include "dl-usb.h"
#include "dl-util.h"
#include "dload.h"

typedef enum {
    kDLoadNone,
    kDLoadSend,
    kDLoadUpload
} dload_action;

int dload_interact (int argc, const char * argv[]) {
    UInt32 addr = 0;
    UInt8 config = 0;
    USBDevice device;
    USBInterface iface;
    dload_action action = kDLoadNone;
    
    printf("Starting dload_interact\n");
    if(argc >= 1) {
        if(strcmp(argv[1], "-c") == 0) {
            // Send custom packets
            action = kDLoadSend;
            
        } else if(strcmp(argv[1], "-f") == 0) {
            // Upload firmware file
            action = kDLoadUpload;
            
        } else {
            // Upload dbl
            action = kDLoadNone;
        }
    } else {
        printf("Usage: dload_interact [-f firmware] [-c command1 command2 ... commandN]\n");
        return -1;
    }
    
    //Vendor ID: 0x5c6
    //Product ID: 0x9008
    device = OpenDevice(0x5c6, 0x9008);
    if(device) {
        printf("Device Opened\n");
        config = SetConfiguration(device, 1);
        if(config == 1) {
            printf("Configuration %hhx set\n", config);
            iface = OpenInterface(device, 0, 0);
            if(iface) {
                printf("Interface Opened\n");
                
                if(action == kDLoadSend && argc >= 3) {
                    int i = 0;
                    int v = 0;
                    unsigned char input[0x200];
                    unsigned char output[0x200];
                    for(v = 2; v < argc; v++) {
                        const char* arg = (const char*) argv[v];
                        unsigned int size = (unsigned int)(strlen(arg) / 2);
                        memset(output,'\0', sizeof(output));
                        memset(input, '\0', sizeof(input));
                        for(i = 0; i < size; i++) {
                            unsigned int byte = 0;
                            sscanf(arg, "%02x", &byte);
                            output[i] = byte;
                            arg += 2;
                        }
                        dload_write(iface, output, size);
                        dload_read(iface, input, sizeof(input));
                    }
                }
                
                if(action == kDLoadUpload && argc == 3) {
                    addr = 0x20012000;
                    dload_get_params(iface);
                    dload_get_sw_version(iface);
                    dload_upload_firmware(iface, addr, argv[2]);
                    dload_send_execute(iface, addr);
                }
                
                if(action == kDLoadNone && argc == 1) {
                    addr = 0x20012000;
                    dload_get_params(iface);
                    dload_get_sw_version(iface);
                    dload_upload_firmware(iface, addr, "/usr/local/standalone/firmware/Trek/dbl.mbn");
                    dload_send_execute(iface, addr);
                }
                
                printf("Closing Interface\n");
                CloseInterface(iface);
                
            } else {
                fprintf(stderr, "Couldn't open device interface\n");
            }
        }
        
        
        CloseDevice(device);
    }
    
    return 0;
}

