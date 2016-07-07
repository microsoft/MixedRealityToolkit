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

// Mostly from http://www.codeproject.com/internet/SendTo.asp and
// Also see http://www.codeguru.com/cpp/i-n/network/messaging/article.php/c5417/

#ifndef __SENDFILETO_H__
#define __SENDFILETO_H__

#include "WindowsIncludes.h"
#include <mapi.h>


class CSendFileTo
{
public:
	bool SendMail(HWND hWndParent, const char *strAttachmentFilePath, const char *strAttachmentFileName,const char *strSubject, const char *strBody, const char *strRecipient);
};

#endif
