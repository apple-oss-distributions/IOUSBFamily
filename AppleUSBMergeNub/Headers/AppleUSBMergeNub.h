/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1998-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOKIT_APPLEUSBMERGENUB_H
#define _IOKIT_APPLEUSBMERGENUB_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>

#include <IOKit/usb/IOUSBBus.h>
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/USB.h>

class AppleUSBMergeNub : public IOService
{
    OSDeclareDefaultStructors(AppleUSBMergeNub)
    
public:
    IOService *			probe(IOService *provider, SInt32 *score);
    virtual bool 		MergeDictionaryIntoProvider(IOService *  provider, OSDictionary *  mergeDict);
    virtual bool		MergeDictionaryIntoDictionary(OSDictionary *  sourceDictionary,  OSDictionary *  targetDictionary);

};
    
#endif _IOKIT_APPLEUSBMERGENUB_H