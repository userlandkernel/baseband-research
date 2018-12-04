//
//  bbtelnetserver.c
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/3/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#include "bbtelnetserver.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char* bbtelnet_msg = NULL;
pthread_t bbtelnet_thread;
int comm_fd;
void* bbtelnetserver(void)
{
    pthread_setname_np("Baseband remote Telnet Server\n");
    int listen_fd;
    
    struct sockaddr_in servaddr;
    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero( &servaddr, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(666);
    
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
    listen(listen_fd, 10);
    
    comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
    
    while(1)
    {
    }
}

void start_bbtelnetserver()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Get the default set of attributes
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Mark the thread as detached
    pthread_create(&bbtelnet_thread, &attr, (void*)bbtelnetserver, NULL); // Start logging tty output in a new thread
}

void stop_bbtelnetserver()
{
    if(bbtelnet_thread)
        pthread_kill(bbtelnet_thread, -1);
}

void bbtelnetserver_send(const char* msg)
{
      write(comm_fd, msg, strlen(msg)+1);
}
