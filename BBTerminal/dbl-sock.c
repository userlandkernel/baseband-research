//
//  sock.cpp
//  dbltool
//
//  Created by Joshua Hill on 2/15/13.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#include "dbl-sock.h"
#include "dbl-utils.h"

int sock_listen(unsigned short portnum) {
    int s;
    struct sockaddr_in sa;
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0) {
        printf("Unable to open socket\n");
        return -1;
    }
    
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_port = htons(portnum);
    sa.sin_family = PF_INET;
    int x = bind(s, (struct sockaddr*)&sa, sizeof(struct sockaddr_in));
    if(x < 0) {
        printf("Unable to bind to socket address\n");
        close(s);
        return -1;
    }
    
    listen(s, 3);
    return s;
}

int sock_accept_connection(int sock) {
    int x = accept(sock, NULL, NULL);
    return x;
}


int sock_send(int sock, unsigned char* data, unsigned int size) {
    //hexdump(data, size);
    int x = (int)send(sock, data, size, 0);
    return x;
}

int sock_recv(int sock, unsigned char* data, unsigned int size) {
    int x = (int)recv(sock, data, size, 0);
    //hexdump(data, x);
    return x;
}
