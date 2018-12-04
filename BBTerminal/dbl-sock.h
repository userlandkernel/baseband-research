//
//  sock.h
//  dbltool
//
//  Created by Joshua Hill on 2/15/13.
//
//

#ifndef __dbltool__sock__
#define __dbltool__sock__

int sock_listen(unsigned short portnum);
int sock_accept_connection(int sock);
int sock_send(int sock, unsigned char* data, unsigned int size);
int sock_recv(int sock, unsigned char* data, unsigned int size);

#endif /* defined(__dbltool__sock__) */
