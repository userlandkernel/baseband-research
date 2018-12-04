//
//  bbtty.c
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/3/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#include "bbtty.h"
#include "bbtelnetserver.h"

FILE* tty_fp_default = NULL;

int
set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tcgetattr", errno);
        return -1;
    }
    
    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);
    
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    
    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void
set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf ("error %d from tggetattr", errno);
        return;
    }
    
    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
    
    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        printf ("error %d setting term attributes", errno);
}

bool
tty_exists(const char *tty)
{
    struct stat buffer;
    int exist = stat(tty, &buffer);
    if(exist == 0)
        return 1;
    else // -1
        return 0;
}

bool
tty_access(const char *tty)
{
    return access(tty, O_APPEND) == 0;
}

bool
bbtty_init(FILE *fp_tty)
{
    //not the best interface to access the baseband, but on iPhone 5 this seems the only way.
    
    if(!fp_tty)
        return 0;
    
    set_interface_attribs (fileno(fp_tty), B115200, 0); // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fileno(fp_tty), 0); // set no blocking
    
    
    
    return 1;
    
}

FILE*
bbtty_open(const char* tty_name)
{
    if(!tty_exists(tty_name)) return NULL;
    if(!tty_access(tty_name)) return NULL;
    
    FILE* fp_tty = fopen(tty_name, "a+");
    
    if(!fp_tty) {
        printf ("error %d opening %s: %s", errno, tty_name, strerror (errno));
        return NULL;
    }
    
    return fp_tty;
    
}

void* tty_logger(FILE* fp_tty)
{
    pthread_setname_np("Baseband TTY Logger");
    while(fp_tty)
    {
        char* buf = NULL;
        size_t size  = read(fileno(fp_tty), buf, 1024);
        getline(&buf, &size, fp_tty);
        if(size) {
            if(strchr((const char*)buf, '\n')) {
                bbtelnetserver_send((const char*)buf);
                printf("%s", buf);
            }
        }
        buf = NULL;
        usleep(20);
    }
    printf("We are now no longer logging any tty messages.\n");
    pthread_exit(NULL);
}

int
bbtty_start()
{
    printf("==== Baseband TTT ===\n");
    const char* ttyname = "/dev/tty.wlan-debug";
    
    FILE* fp_tty = bbtty_open(ttyname);
    if(!bbtty_init(fp_tty)) {
        printf("Failed to initialize tty.\n");
        return 0;
    }
    
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Get the default set of attributes
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Mark the thread as detached
    pthread_create(&thread, &attr, (void*)tty_logger, fp_tty); // Start logging tty output in a new thread
    
    tty_fp_default = fp_tty; //update global
    
    return 1;
}

kern_return_t baseband_write(char* msg, FILE* fp_tty)
{
    if(!fp_tty && !tty_fp_default) return KERN_INVALID_ARGUMENT;
    
    if(!fp_tty && tty_fp_default) fp_tty = tty_fp_default;
    printf("Writing %s\n", msg);
    fwrite(msg, sizeof(char), strlen(msg), fp_tty);
    return KERN_SUCCESS;
}

void bbtty_close(FILE* fp_tty)
{
    if(!fp_tty) fp_tty = tty_fp_default;
    if(fp_tty)
        fclose(fp_tty);
}
