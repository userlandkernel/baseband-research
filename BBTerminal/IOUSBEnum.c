//
//  IOUSBEnum.c
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/4/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#include "IOUSBEnum.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <IOKit/usb/USB.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>

int print_configuration(IOUSBConfigurationDescriptor desc) {
    printf("\tLength: 0x%hhx\n", desc.bLength);
    printf("\tDescriptor Type: 0x%hhx\n", desc.bDescriptorType);
    printf("\tTotal Length: 0x%hx\n", desc.wTotalLength);
    printf("\tNum Interfaces: 0x%hhx\n", desc.bNumInterfaces);
    printf("\tConfiguration Value: 0x%hhx\n", desc.bConfigurationValue);
    printf("\tConfiguration: 0x%hhx\n", desc.iConfiguration);
    printf("\tAttributes: 0x%hhx\n", desc.bmAttributes);
    printf("\tMax Power: 0x%hhx\n", desc.MaxPower);
    return 0;
}

int print_interface(IOUSBInterfaceDescriptor desc) {
    printf("\t\tLength: 0x%hhx\n", desc.bLength);
    printf("\t\tDescriptor Type: 0x%hhx\n", desc.bDescriptorType);
    printf("\t\tInterface Number: 0x%hhx\n", desc.bInterfaceNumber);
    printf("\t\tAlternate Setting: 0x%hhx\n", desc.bAlternateSetting);
    printf("\t\tNum Endpoints: 0x%hhx\n", desc.bNumEndpoints);
    switch(desc.bInterfaceClass) {
        case kUSBAudioInterfaceClass:
            printf("\t\tInterface Class: Audio\n");
            break;
            
        case kUSBCommunicationControlInterfaceClass:
            printf("\t\tInterface Class: Communication Control\n");
            break;
            
        case kUSBCommunicationDataInterfaceClass:
            printf("\t\tInterface Class: Communication Data\n");
            break;
            
        case kUSBHIDInterfaceClass:
            printf("\t\tInterface Class: HID\n");
            break;
            
        case kUSBPhysicalInterfaceClass:
            printf("\t\tInterface Class: Physical\n");
            break;
            
        case kUSBImageInterfaceClass:
            printf("\t\tInterface Class: Image\n");
            break;
            
        case kUSBPrintingInterfaceClass:
            printf("\t\tInterface Class: Printing\n");
            break;
            
        case kUSBMassStorageInterfaceClass:
            printf("\t\tInterface Class: Storage\n");
            break;
            
        case kUSBChipSmartCardInterfaceClass:
            printf("\t\tInterface Class: Smart Card\n");
            break;
            
        case kUSBContentSecurityInterfaceClass:
            printf("\t\tInterface Class: Content Security\n");
            break;
            
        case kUSBVideoInterfaceClass:
            printf("\t\tInterface Class: Video\n");
            break;
            
        case kUSBPersonalHealthcareInterfaceClass:
            printf("\t\tInterface Class: Personal Healthcare\n");
            break;
            
        case kUSBDiagnosticDeviceInterfaceClass:
            printf("\t\tInterface Class: Diagnostic\n");
            break;
            
        case kUSBWirelessControllerInterfaceClass:
            printf("\t\tInterface Class: Wireless\n");
            break;
            
        case kUSBApplicationSpecificInterfaceClass:
            printf("\t\tInterface Class: Application Specific\n");
            break;
            
        case kUSBVendorSpecificInterfaceClass:
            printf("\t\tInterface Class: Vendor Specific\n");
            break;
            
        default:
            printf("\t\tInterface Class: 0x%hhx\n", desc.bInterfaceClass);
            break;
    }
    printf("\t\tInterface SubClass: 0x%hhx\n", desc.bInterfaceSubClass);
    printf("\t\tInterface Protocol: 0x%hhx\n", desc.bInterfaceProtocol);
    return 0;
}

int print_endpoint(IOUSBEndpointDescriptor desc) {
    printf("\t\t\tLength: 0x%hhx\n", desc.bLength);
    printf("\t\t\tDescriptor Type: 0x%hhx\n", desc.bDescriptorType);
    printf("\t\t\tEndpoint Address: 0x%hhx\n", desc.bEndpointAddress);
    printf("\t\t\tAttributes: 0x%hhx\n", desc.bmAttributes);
    switch(desc.bmAttributes & kUSBEndpointbmAttributesTransferTypeMask) {
        case kUSBControl:
            printf("\t\t\t\tTransfer Type: Control\n");
            break;
            
        case kUSBIsoc:
            printf("\t\t\t\tTransfer Type: Isoc\n");
            break;
            
        case kUSBBulk:
            printf("\t\t\t\tTransfer Type: Bulk\n");
            break;
            
        case kUSBInterrupt:
            printf("\t\t\t\tTransfer Type: Interrupt\n");
            break;
            
        case kUSBAnyType:
            printf("\t\t\t\tTransfer Type: Any\n");
            break;
            
        default:
            printf("\t\t\t\tTransfer Type: Unknown\n");
            break;
            
    }
    printf("\t\t\tMax Packet Size: 0x%hx\n", desc.wMaxPacketSize);
    printf("\t\t\tInterval: 0x%hhx\n", desc.bInterval);
    return 0;
}

int enumerate_configuration(IOUSBConfigurationDescriptorPtr configuration) {
    UInt8* end = NULL;
    UInt8* buffer = NULL;
    
    print_configuration(*configuration);
    
    buffer = (UInt8*) configuration;
    end = buffer + configuration->wTotalLength;
    buffer += configuration->bLength;
    
    
    while (buffer < end) {
        UInt8 len = buffer[0];
        UInt8 type = buffer[1];
        
        if (len > 0) {
            if (type == kUSBInterfaceDesc) {
                printf("\tInterface\n");
                IOUSBInterfaceDescriptorPtr interface = (IOUSBInterfaceDescriptorPtr) buffer;
                print_interface(*interface);
            } else if(type == kUSBEndpointDesc) {
                printf("\t\tEndpoint\n");
                IOUSBEndpointDescriptorPtr endpoint = (IOUSBEndpointDescriptorPtr) buffer;
                print_endpoint(*endpoint);
            }
            buffer += len;
        }
    }
    
    return 0;
}

int enumerate_device(io_service_t service) {
    SInt32 score;
    UInt16 vendor;
    UInt16 product;
    UInt16 release;
    UInt32 location;
    HRESULT result;
    kern_return_t kr;
    IOCFPlugInInterface** plugin = NULL;
    IOUSBDeviceInterface** device = NULL;
    if(!service) return -1;
    kr = IOCreatePlugInInterfaceForService(service,
                                           kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
                                           &plugin, &score);
    
    io_name_t deviceName;
    kr = IORegistryEntryGetName(service, deviceName);
    if (KERN_SUCCESS != kr) {
        deviceName[0] = '\0';
    }
    
    kr = IOObjectRelease(service);
    if ((kr != kIOReturnSuccess) || !plugin) {
        printf("Unable to create a plug-in for: %s @ %#x\n", deviceName, service);
        return -1;
    }
    
    result = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID *)&device);
    (*plugin)->Release(plugin);
    
    if (result || !device) {
        printf("Couldn’t create a device interface\n");
        return -1;
    }
    
    //Open the device to change its state
    kr = (*device)->USBDeviceOpen(device);
    if (kr == kIOReturnSuccess) {
        printf("Device Name: %s\n", deviceName);
        kr = (*device)->GetDeviceVendor(device, &vendor);
        printf("Vendor ID: 0x%hx\n", vendor);
        kr = (*device)->GetDeviceProduct(device, &product);
        printf("Product ID: 0x%hx\n", product);
        kr = (*device)->GetDeviceReleaseNumber(device, &release);
        printf("Version: 0x%hx\n", release);
        kr = (*device)->GetLocationID(device, &location);
        printf("Location: %#x\n", (unsigned int)location);
        
        UInt8 config = 0;
        UInt8 numconfigs = 0;
        IOUSBConfigurationDescriptorPtr desc;
        kr = (*device)->GetNumberOfConfigurations(device, &numconfigs);
        for(config = 0; config < numconfigs; config++) {
            printf("Configuration: %hhx\n", config);
            kr = (*device)->GetConfigurationDescriptorPtr(device, config, &desc);
            enumerate_configuration(desc);
        }
        
        //Close this device and release object
        kr = (*device)->USBDeviceClose(device);
    }
    kr = (*device)->Release(device);
    
    return 0;
}

int enumerate_usb_devices(void)
{
    kern_return_t kr;
    mach_port_t masterPort;
    CFMutableDictionaryRef matchingDict;
    
    printf("Starting iOSUSBEnum\n");
    
    //Create a master port for communication with the I/O Kit
    kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (kr != KERN_SUCCESS || !masterPort) {
        printf("Couldn’t create a master I/O Kit port\n");
        return -1;
    }
    
    //Set up matching dictionary for class IOUSBDevice and its subclasses
    //
    matchingDict = IOServiceMatching("AppleUSBHSICDevice");
    //matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (matchingDict == NULL) {
        printf("Couldn’t create a USB matching dictionary\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return -1;
    }
    
    io_iterator_t iter;
    kr = IOServiceGetMatchingServices(masterPort, matchingDict, &iter);
    if(kr != KERN_SUCCESS) {
        printf("Unable to find matching services\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return -1;
    }
    
    io_service_t device;
    while((device = IOIteratorNext(iter))) {
        enumerate_device(device);
    }
    if(matchingDict != NULL)
       // CFRelease(matchingDict);
    
    //Finished with master port
    mach_port_deallocate(mach_task_self(), masterPort);
    
    return 0;
}
