//
//  usb.cpp
//  dloadtool
//
//  Created by Joshua Hill on 1/31/13.
//
//

#include <stdio.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include "dbl-usb.h"

void CloseDevice(USBDevice device) {
    (*device)->USBDeviceClose(device);
    (*device)->Release(device);
}

USBDevice OpenDevice(SInt32 vendorId, SInt32 productId) {
    SInt32 score;
    HRESULT result;
    kern_return_t kr;
    io_service_t service;
    mach_port_t masterPort;
    IOCFPlugInInterface** plugin;
    IOUSBDeviceInterface** device;
    CFMutableDictionaryRef matchingDict;
    
    SInt32 vendor = vendorId;
    SInt32 product = productId;
    
    kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (kr || !masterPort) {
        fprintf(stderr, "Couldn’t create a master IOKit port\n");
        return NULL;
    }
    
    matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matchingDict) {
        fprintf(stderr, "Couldn’t create a USB matching dictionary\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return NULL;
    }
    
    CFDictionarySetValue(matchingDict, CFSTR(kUSBVendorName),
                         CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &vendor));
    CFDictionarySetValue(matchingDict, CFSTR(kUSBProductName),
                         CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &product));
    
    service = IOServiceGetMatchingService(masterPort, matchingDict);
    if(!service) {
        fprintf(stderr, "Couldn't find matching USB service\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return NULL;
    }
    
    kr = IOCreatePlugInInterfaceForService(service, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);
    if(kr != kIOReturnSuccess) {
        fprintf(stderr, "Couldn't create USB device interface plugin\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return NULL;
    }
    IOObjectRelease(service);
    
    result = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*) &device);
    (*plugin)->Release(plugin);
    
    if(result || !device) {
        fprintf(stderr, "Couldn't query USB device interface plugin\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return NULL;
    }
    
    kr = (*device)->USBDeviceOpen(device);
    if(kr) {
        fprintf(stderr, "Couldn't open USB device\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        (*device)->Release(device);
        return NULL;
    }
    
    return device;
}


UInt8 SetConfiguration(USBDevice device, UInt8 configuration) {
    UInt8 num;
    UInt8 config;
    UInt8 current;
    kern_return_t kr;
    if(device) {
        kr = (*device)->GetConfiguration(device, &current);
        if(current != configuration) {
            kr = (*device)->GetNumberOfConfigurations(device, &num);
            if(configuration <= num) {
                (*device)->SetConfiguration(device, configuration);
                config = configuration;
            } else config = current;
        } else config = current;
    } else config = -1;
    
    return config;
}


USBInterface OpenInterface(USBDevice device, UInt8 interface, UInt8 alt_interface) {
    UInt8 alt;
    UInt8 current;
    SInt32 score;
    HRESULT result;
    kern_return_t kr;
    io_service_t service;
    io_iterator_t iterator;
    IOCFPlugInInterface** plugin;
    IOUSBInterfaceInterface** iface = NULL;
    IOUSBFindInterfaceRequest request;
    
    if(device) {
        request.bInterfaceClass = kIOUSBFindInterfaceDontCare;
        request.bAlternateSetting = kIOUSBFindInterfaceDontCare;
        request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
        request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
        
        kr = (*device)->CreateInterfaceIterator(device, &request, &iterator);
        if(kr != kIOReturnSuccess) {
            fprintf(stderr, "Couldn't create USB device interface iterator\n");
            return NULL;
        }
        
        while((service = IOIteratorNext(iterator))) {
            kr = IOCreatePlugInInterfaceForService(service, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID,
                                                   &plugin, &score);
            if(kr != kIOReturnSuccess) {
                fprintf(stderr, "Couldn't create USB device interface plugin\n");
                continue;
            }
            IOObjectRelease(service);
            
            result = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID*) &iface);
            (*plugin)->Release(plugin);
            
            if(result || !iface) {
                fprintf(stderr, "Couldn't query USB device interface plugin\n");
                continue;
            }
            
            kr = (*iface)->GetInterfaceNumber(iface, &current);
            if(kr != kIOReturnSuccess) {
                fprintf(stderr, "Couldn't get current USB device interface\n");
                (*iface)->Release(iface);
                continue;
            }
            
            if(current == interface) {
                kr = (*iface)->USBInterfaceOpen(iface);
                if(kr != kIOReturnSuccess) {
                    fprintf(stderr, "Couldn't open USB device interface\n");
                    (*iface)->Release(iface);
                    return NULL;
                }
                
                if(alt_interface != 0) {
                    kr = (*iface)->GetAlternateSetting(iface, &alt);
                    if(kr != kIOReturnSuccess) {
                        fprintf(stderr, "Couldn't get USB device interface alternate setting\n");
                        (*iface)->USBInterfaceClose(iface);
                        (*iface)->Release(iface);
                        return NULL;
                    }
                    if(alt_interface == alt) {
                        kr = (*iface)->SetAlternateInterface(iface, alt);
                    } else {
                        fprintf(stderr, "Invalid alternate interface\n");
                    }
                }
                break;
            }
        }
    }
    
    return iface;
}

void CloseInterface(USBInterface iface) {
    (*iface)->USBInterfaceClose(iface);
    (*iface)->Release(iface);
}

int WriteBulk(USBInterface iface, UInt8 pipe, void* buf, UInt32 size) {
    kern_return_t kr;
    if(iface) {
        kr = (*iface)->WritePipe(iface, pipe, buf, size);
        if(kr != kIOReturnSuccess) {
            fprintf(stderr, "Couldn't write to USB device interface\n");
            return -1;
        }
    }
    return 0;
}

int ReadBulk(USBInterface iface, UInt8 pipe, void* buf, UInt32* size) {
    kern_return_t kr;
    if(iface) {
        kr = (*iface)->ReadPipe(iface, pipe, buf, size);
        if(kr != kIOReturnSuccess) {
            fprintf(stderr, "Couldn't read from USB device interface\n");
            return -1;
        }
    }
    return 0;
}

