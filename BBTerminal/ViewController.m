//
//  ViewController.m
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/2/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#import "ViewController.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>

#include "bbtty.h"
#include "godfu.h"
#include "bbtelnetserver.h"
#include "IOUSBEnum.h"
#include "dload-interact.h"
#include "bootloader-interact.h"


@interface ViewController ()
@property (weak, nonatomic) IBOutlet UITextField *cmd;
@property (weak, nonatomic) IBOutlet UIButton *send;
- (IBAction)do_send:(id)sender;

@end

@implementation ViewController



- (void)viewDidLoad {
    [super viewDidLoad];
    start_bbtelnetserver();
    printf("Waiting for server to start on port 666...\n");
    sleep(1);
    
    bbtty_start(); //start serial logger
    if(bb_enter_download_mode()){ //enter DLOAD mode
        enumerate_usb_devices(); //enumerate to find baseband device and verify it is in DLOAD.
        bootloader_interact(NULL, NULL, NULL, 0, 0);
        dload_interact(0, NULL);
    }
    
}


- (IBAction)do_send:(id)sender {
    baseband_write((char*)self.cmd.text.UTF8String, BBTY_USE_DEFAULT);
}


@end
