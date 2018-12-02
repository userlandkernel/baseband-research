//
//  AppDelegate.h
//  BBTerminal
//
//  Created by Sem Voigtländer on 12/2/18.
//  Copyright © 2018 Sem Voigtländer. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreData/CoreData.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@property (readonly, strong) NSPersistentContainer *persistentContainer;

- (void)saveContext;


@end

