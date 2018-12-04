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
    bbtty_start();
    bb_enter_download_mode();
    // Do any additional setup after loading the view, typically from a nib.
}


- (IBAction)do_send:(id)sender {
    baseband_write((char*)self.cmd.text.UTF8String, BBTY_USE_DEFAULT);
}


@end
