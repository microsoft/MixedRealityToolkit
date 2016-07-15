// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// Modal view used to enter the chat server IP/Port

#import <UIKit/UIKit.h>
#import "ChatServerDetailsProtocol.h"

@protocol ChatServerDetailsProtocol;

@interface ServerDetailsViewController : UIViewController
{
    id<ChatServerDetailsProtocol> delegate;
    UITextField* mServerIP;
    UITextField* mServerPort;
}

-(IBAction)dismissServerDetailsView;

@property (nonatomic, assign) id<ChatServerDetailsProtocol> delegate;
@property (nonatomic, retain) IBOutlet UITextField *mServerIP;
@property (nonatomic, retain) IBOutlet UITextField *mServerPort;

@end
