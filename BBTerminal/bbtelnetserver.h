//
//  bbtelnetserver.h
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/3/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#ifndef bbtelnetserver_h
#define bbtelnetserver_h

#include <stdio.h>
void start_bbtelnetserver(void);

void stop_bbtelnetserver(void);

void bbtelnetserver_send(const char* msg);

#endif /* bbtelnetserver_h */
