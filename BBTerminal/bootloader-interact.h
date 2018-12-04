//
//  bootloader-interact.h
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/4/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#ifndef bootloader_interact_h
#define bootloader_interact_h

#include <stdio.h>
int bootloader_interact (const char* bbticket, const char* osbl, const char* amss, uint32_t VID, uint32_t PID);
#endif /* bootloader_interact_h */
