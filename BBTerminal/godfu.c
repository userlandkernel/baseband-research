//
//  godfu.c
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/3/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#include "godfu.h"
#include <dlfcn.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <dlfcn.h>
#include <IOKit/IOKitLib.h>

int bb_reset() {
    printf("Resetting baseband\n");
    io_connect_t connect = 0;
    CFMutableDictionaryRef match = IOServiceMatching("AppleBaseband");
    io_service_t service = IOServiceGetMatchingService(0, match);
    
    kern_return_t err = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if(err != KERN_SUCCESS) {
        printf("Failed to connect to baseband driver: %s\n", mach_error_string(err));
        IOServiceClose(connect);
        return 0;
    }
        
    err = IOConnectCallScalarMethod(connect, 0, 0, 0, 0, 0);
    if(err != KERN_SUCCESS) {
        printf("Failed to reset the baseband: %s\n", mach_error_string(err));
        IOServiceClose(connect);
        return 0;
    }
        
    sleep(1);
    
    if(connect != MACH_PORT_NULL)
        IOServiceClose(connect);
    
    printf("Baseband should be in DLOAD mode now.\n");
    return 1;
}

int bb_enter_download_mode() {
    io_connect_t connect = 0;
    CFMutableDictionaryRef match = IOServiceMatching("AppleBaseband");
    io_service_t service = IOServiceGetMatchingService(0, match);
    
    kern_return_t err = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if(err != KERN_SUCCESS) {
        printf("Failed to connect to baseband driver: %s\n", mach_error_string(err));
        return 0;
    }
    
    printf("Connected to the baseband driver: %#x\n", connect);
    printf("Entering download mode\n");
    uint64_t cmd = 0x1;
    err = IOConnectCallScalarMethod(connect, 0x1, &cmd, 0x1, 0, 0);
    
    if(err != KERN_SUCCESS) {
        printf("Failed to put the baseband in DLOAD mode: %s\n", mach_error_string(err));
        return 0;
    }
    
    if(connect != MACH_PORT_NULL)
        IOServiceClose(connect);
    
    sleep(1);
    printf("Baseband should be in DLOAD mode now.\n");
    return 1;
}
