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

void bb_reset() {
    printf("Resetting baseband\n");
    io_connect_t connect = 0;
    CFMutableDictionaryRef match = IOServiceMatching("AppleBaseband");
    io_service_t service = IOServiceGetMatchingService(0, match);
    
    kern_return_t err = IOServiceOpen(service, mach_task_self(), 0, &connect);
     if(err != KERN_SUCCESS) printf("Failed to connect to baseband driver: %s\n", mach_error_string(err));
    
    err = IOConnectCallScalarMethod(connect, 0, 0, 0, 0, 0);
    if(err != KERN_SUCCESS) printf("Failed to reset the baseband: %s\n", mach_error_string(err));
    
    sleep(1);
    IOServiceClose(connect);
    printf("Baseband should be in DLOAD mode now.\n");
}

void bb_enter_download_mode() {
    printf("Entering download mode\n");
    io_connect_t connect = 0;
    CFMutableDictionaryRef match = IOServiceMatching("AppleBaseband");
    io_service_t service = IOServiceGetMatchingService(0, match);
    
    kern_return_t err = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if(err != KERN_SUCCESS) printf("Failed to connect to baseband driver: %s\n", mach_error_string(err));
    
    uint64_t cmd = 0x1;
    err = IOConnectCallScalarMethod(connect, 0x1, &cmd, 0x1, 0, 0);
    if(err != KERN_SUCCESS) printf("Failed to put the baseband in DLOAD mode: %s\n", mach_error_string(err));
    
    IOServiceClose(connect);
    sleep(1);
    printf("Baseband should be in DLOAD mode now.\n");
}
