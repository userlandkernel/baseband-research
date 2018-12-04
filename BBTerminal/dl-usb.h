//
//  dbl-usb.h
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/4/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//
//

#ifndef dloadtool_usb_h
#define dloadtool_usb_h

#include <IOKit/usb/IOUSBLib.h>

typedef IOUSBDeviceInterface** USBDevice;
typedef IOUSBInterfaceInterface** USBInterface;

extern void CloseDevice(USBDevice device);
extern USBDevice OpenDevice(SInt32 vendorId, SInt32 productId);

extern UInt8 SetConfiguration(USBDevice device, UInt8 configuration);

extern USBInterface OpenInterface(USBDevice device, UInt8 interface, UInt8 alt_interface);
extern void CloseInterface(USBInterface interface);

extern int WriteBulk(USBInterface iface, UInt8 pipe, void* buf, UInt32 size);
extern int ReadBulk(USBInterface iface, UInt8 pipe, void* buf, UInt32* size);

#endif
