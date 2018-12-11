# iPhone Baseband Hacking

This repository is part of a project that is the result of many hours of research.  
In this repository you can find utilities and documents explaining how to hack your baseband and providing an easy interface to it.  
It primarily focusses on the Qualcomm baseband chips used in 32-bit iPhones and iPads.  

The baseband is a chip on the iPhone motherboard with its own flash memory, ram and cpu.  
The baseband therefore also runs an operating system completely seperated from iOS, its own bootloader, its own recovery modes. 
The only references in iOS to it are via the IOKit-based kernel driver, communication center and the serial tty device.  

A baseband chip is used for network operations such as Texting, Calling and LTE.  
There are many ways to communicate with the chip each with its own purpose and functionality.  

Custom firmware has not yet been created but hopefully can be achieved in the future it by porting the OKL4 kernel and creating an image accordingly fitting with the format used by Qualcomm.  
Baseband firmware is codesigned and therefore a defeat for the validation of the signatures is needed for loading custom firmware.


## Serial Logging

Serial logging will not be the same accross iOS versions and devices.  
Some devices are featured to send AT carrier commands, other only privide a very basic logging interface.  
In the future more research can perhaps point out to better serial logging interfaces of the baseband.  

## Hardening and Exploit mitigations

It looks as if Qualcomm has not paid much attention to preventing exploitation through hardening.  
ASLR is not implemented in the Qualcomm baseband firmware of the iPhone 5 and below.  
Neither is there any proper memory protection through secure mappings or kernel patch protection.  
The stack looks executable as well at first sight, making the baseband firmware an easy target to exploit.


## Radio based exploitation

Via Software Defined Radio such as the HackRF One attackers can remotely compromise the baseband and trigger a buffer overflow. 
You can read more about this (soon).  


## Local exploitation

One can overflow by using specific carrier commands.  
This exploit has also been used in carrier unlocking locked iPhones in the past.  


## Local exploitation (firmware)

One can craft custom firmware and load it in DLOAD mode.  
The custom firmware can exploit a vulnerability in the way the image headers are treated (ELF) therefore an attacker is able to defeat the codesigning requirements.  


## Local (serial?) exploitation

One can corrupt kernel memory in a debug buffer by using the QKRT_writeq() function.
It allows an attacker to specify a negative integer value which makes it possible to overflow a kernel debugging buffer.
However, this might be hard to exploit


## Baseband debugging

In diagnostic mode, called sahara mode, one can debug the baseband through QXDM.
QXDM is internal and proprietary software from Qualcomm.  
Officially it is not available in public.  
QXDM requires a valid login-based license that is activated by providing a username, password and admin code.  
Cracking QXDM is covered in this document: (soon).  
The baseband will not store any crashlogs on the NAND, the full crashlogs including registers are stored in protected NVRAM.  



## Useful to know

The kernel driver used to manage the baseband is accessible through IOKit while jailbroken.  
By sending scalar input to the driver one can make the baseband enter DLOAD mode or reset.  
The code of this kernel driver from Apple for the iPhone 3G, iPhone 2G and iPod Touch has been leaked in the past.  
It is available here at: https://github.com/UKERN-Developers/apple-baseband  

Apple's IOUSBFamily is OpenSource as well.  
It is a swiss army knife for accessing a baseband in DLOAD mode.  
It can be found here: https://github.com/UKERN-Developers/IOUSBFamily  


## References, Papers and Writeups

I have documented my research on the iPhoneWiki for those who want a brief overview of documentation resulted from the research.  
My personal approach and thoughts, progress and findings are explained and continued at my website https://kernelprogrammer.com  
QualComm has a lot of (internal) documents available as well, some not public, some are which are useful for understanding the baseband.  
OKL4 seems to have become private, or discontinued.  
However, the wayback machine never lets you down, the source of the kernel can be downloaded here: http://web.archive.org/web/20130518105803/http://wiki.ok-labs.com/downloads/release-2.1.1-fix.7/okl4_2.1.1-fix.7.tar.gz  
The old Wiki of the kernel is also cached by the wayback machine: http://web.archive.org/web/20150303212126/http://wiki.ok-labs.com/  
A big thank you goes out to posixninja for his documentation on the older baseband and his headstart on writing tools for accessing it.  
Another thank you goes out to @hackerfantastic for pointing me out to the vulnerabilities related to the ELF firmware.


