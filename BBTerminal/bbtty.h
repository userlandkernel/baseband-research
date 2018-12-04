//
//  bbtty.h
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/3/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#ifndef bbtty_h
#define bbtty_h

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#include <pthread.h>
#include <mach/mach.h>

int bbtty_start(void);
kern_return_t baseband_write(char* msg, FILE* fp_tty);
void bbtty_close(FILE* fp_tty);

#define BBTY_USE_DEFAULT NULL

#endif /* bbtty_h */
