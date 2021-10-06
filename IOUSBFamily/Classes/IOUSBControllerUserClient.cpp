/*
 * Copyright (c) 1998-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.2 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.  
 * Please see the License for the specific language governing rights and 
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */ 
#include <libkern/OSByteOrder.h>

#include <IOKit/assert.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMessage.h>

#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBControllerV2.h>
#include <IOKit/usb/IOUSBLog.h>

#include "IOUSBControllerUserClient.h"

#define super IOUserClient


//================================================================================================
//	IOKit stuff and Constants
//================================================================================================
//
OSDefineMetaClassAndStructors(IOUSBControllerUserClient, IOUserClient)

enum {
        kMethodObjectThis = 0,
        kMethodObjectOwner
    };
    
//================================================================================================
//	Method Table
//================================================================================================
//
const IOExternalMethod 
IOUSBControllerUserClient::sMethods[kNumUSBControllerMethods] =
{	
	{	// kUSBControllerUserClientOpen
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::open,		// func
		kIOUCScalarIScalarO,					// flags
		1,							// # of params in
		0							// # of params out
	},
	{	// kUSBControllerUserClientClose
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::close,		// func
		kIOUCScalarIScalarO,					// flags
		0,							// # of params in
		0							// # of params out
	},
	{	// kUSBControllerUserClientEnableLogger
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::EnableKernelLogger,	// func
		kIOUCScalarIScalarO,					// flags
		1,							// # of params in
		0							// # of params out
	},
	{	// kUSBControllerUserClientSetDebuggingLevel
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::SetDebuggingLevel,// func
		kIOUCScalarIScalarO,					// flags
		1,							// # of params in
		0							// # of params out
	},
	{	// kUSBControllerUserClientSetDebuggingType
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::SetDebuggingType,// func
		kIOUCScalarIScalarO,					// flags
		1,							// # of params in
		0							// # of params out
	},
	{	// kUSBControllerUserClientGetDebuggingLevel
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::GetDebuggingLevel,// func
		kIOUCScalarIScalarO,					// flags
		0,							// # of params in
		1							// # of params out
	},
	{	// kUSBControllerUserClientGetDebuggingType
		(IOService*)kMethodObjectThis,				// object
		( IOMethod ) &IOUSBControllerUserClient::GetDebuggingType,// func
		kIOUCScalarIScalarO,					// flags
		0,							// # of params in
		1							// # of params out
	},
	{	// kUSBControllerUserClientSetTestMode
		(IOService*)kMethodObjectThis,				// object
		( IOMethod )&IOUSBControllerUserClient::SetTestMode,	// func
		kIOUCScalarIScalarO,					// flags
		2,							// # of params in
		0							// # of params out
	}
};

const IOItemCount 
IOUSBControllerUserClient::sMethodCount = sizeof( IOUSBControllerUserClient::sMethods ) / 
							sizeof( IOUSBControllerUserClient::sMethods[ 0 ] );

void 
IOUSBControllerUserClient::SetExternalMethodVectors()
{
    fMethods = sMethods;
    fNumMethods = kNumUSBControllerMethods;
}



IOExternalMethod *
IOUSBControllerUserClient::getTargetAndMethodForIndex(IOService **target, UInt32 index)
{
    if (index < (UInt32)fNumMethods) 
    {
        if ((IOService*)kMethodObjectThis == fMethods[index].object)
            *target = this;
        else if ((IOService*)kMethodObjectOwner == fMethods[index].object)
            *target = fOwner;
        else
            return NULL;
	return (IOExternalMethod *) &fMethods[index];
    }
    else
	return NULL;
}



//================================================================================================

bool
IOUSBControllerUserClient::initWithTask(task_t owningTask, void *security_id, UInt32 type, OSDictionary * properties)
{
    IOLog("IOUSBControllerUserClient::initWithTask(type %ld)\n", type);
    
    if (!owningTask)
	return false;
	
    fTask = owningTask;
    fOwner = NULL;
    fGate = NULL;
    fDead = false;
    
    SetExternalMethodVectors();
    
    return (super::initWithTask(owningTask, security_id , type, properties));
}



bool 
IOUSBControllerUserClient::start( IOService * provider )
{
    fOwner = OSDynamicCast(IOUSBController, provider);
    IOLog("+IOUSBControllerUserClient::start (%p)\n", fOwner);
    if (!fOwner)
	return false;
    
    if(!super::start(provider))
        return false;

    return true;
}


IOReturn 
IOUSBControllerUserClient::open(bool seize)
{
    IOOptionBits	options = seize ?  (IOOptionBits)kIOServiceSeize : 0;

    IOLog("+IOUSBControllerUserClient::open\n");
    if (!fOwner)
        return kIOReturnNotAttached;

    if (!fOwner->open(this, options))
	return kIOReturnExclusiveAccess;

    return kIOReturnSuccess;
}



IOReturn 
IOUSBControllerUserClient::close()
{
    IOLog("+IOUSBControllerUserClient::close\n");
    if (!fOwner)
        return kIOReturnNotAttached;

    if (fOwner && (fOwner->isOpen(this)))
	fOwner->close(this);

    return kIOReturnSuccess;
}

IOReturn
IOUSBControllerUserClient::EnableKernelLogger(bool enable)
{
    IOLog("+IOUSBControllerUserClient::EnableKernelLogger\n");
    if (!fOwner)
        return kIOReturnNotAttached;
    
    KernelDebugEnable(enable);
    
    return kIOReturnSuccess;
}

IOReturn
IOUSBControllerUserClient::SetDebuggingLevel(KernelDebugLevel inLevel)
{
    IOLog("+IOUSBControllerUserClient::SetDebuggingLevel\n");
    if (!fOwner)
        return kIOReturnNotAttached;
    
    KernelDebugSetLevel(inLevel);
    
    return kIOReturnSuccess;
}

IOReturn
IOUSBControllerUserClient::SetDebuggingType(KernelDebuggingOutputType inType)
{
    IOLog("+IOUSBControllerUserClient::SetDebuggingType\n");
    if (!fOwner)
        return kIOReturnNotAttached;
    
    KernelDebugSetOutputType(inType);

    return kIOReturnSuccess;
}

IOReturn
IOUSBControllerUserClient::GetDebuggingLevel(KernelDebugLevel * inLevel)
{
     IOLog("+IOUSBControllerUserClient::GetDebuggingLevel\n");
   if (!fOwner)
        return kIOReturnNotAttached;
    
    *inLevel = KernelDebugGetLevel();

    return kIOReturnSuccess;
}

IOReturn
IOUSBControllerUserClient::GetDebuggingType(KernelDebuggingOutputType * inType)
{
    IOLog("+IOUSBControllerUserClient::GetDebuggingLevel\n");
    if (!fOwner)
        return kIOReturnNotAttached;
    
    *inType = KernelDebugGetOutputType();

    return kIOReturnSuccess;
}


IOReturn
IOUSBControllerUserClient::SetTestMode(UInt32 mode, UInt32 port)
{
    // this method only available for v2 controllers
    IOUSBControllerV2	*v2 = OSDynamicCast(IOUSBControllerV2, fOwner);

    IOLog("+IOUSBControllerUserClient::SetTestMode");
    if (!v2)
        return kIOReturnNotAttached;
    
    return v2->SetTestMode(mode, port);
}


void 
IOUSBControllerUserClient::stop( IOService * provider )
{
    IOLog("IOUSBControllerUserClient::stop\n");

    super::stop( provider );
}

IOReturn 
IOUSBControllerUserClient::clientClose( void )
{
    /*
     * Kill ourselves off if the client closes or the client dies.
     */
    IOLog("%s[%p]::clientClose isInactive = %d\n", getName(), this, isInactive());
    if( !isInactive())
        terminate();

    return( kIOReturnSuccess );
}

/*
IOReturn
IOUSBControllerUserClient::clientMemoryForType( UInt32 type, IOOptionBits * options, IOMemoryDescriptor ** memory )
{
    IOMemoryMap *    baseAddress;
    
    IOLog("IOUSBControllerUserClient::stop\n");

    if (!fOwner)
        return kIOReturnNotAttached;

    // Get the _deviceBase from the OHCI driver
    //
    baseAddress = fOwner->GetBaseAddress();

    if ( baseAddress )
    {
        IOLog("Got BaseAddress 0x%x\n",baseAddress->getVirtualAddress());
        fOHCIRegistersDescriptor = baseAddress->getMemoryDescriptor();
    }
    if (fOHCIRegistersDescriptor)
    {
        *memory = fOHCIRegistersDescriptor;
        *options = 0;
    }
}
*/
