/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include <windows.h>
#include "msrnt.h"

int WINAPI _ReadMSR(ULONG ulECX, ULONG* ulEAX, ULONG* ulEDX)
{
	ULONG ReturnedLength;
 	BOOL IoctlResult;
	char buf[8]={0};

	IoctlResult = DeviceIoControl(
					handle,				// Handle to device
					IOCTL_READ_MSR,		// RDMSR code
					&ulECX,				// Buffer to driver
					sizeof(ulECX),		// Length of buffer in bytes.
					&buf,				// Buffer from driver.
					sizeof(buf),		// Length of buffer in bytes.
					&ReturnedLength,	// Bytes placed in outbuf.
					NULL				// 
					);
	memcpy(ulEAX,buf,4);
	memcpy(ulEDX,buf+4,4);

	return IoctlResult;
}

int WINAPI _WriteMSR(ULONG ulECX, ULONG* ulEAX, ULONG* ulEDX)
{

	ULONG ReturnedLength;
 	BOOL IoctlResult;
	ULONG result;
	char buf[12];
	memcpy(buf,&ulECX,4);
	memcpy(buf+4,ulEAX,4);
	memcpy(buf+8,ulEDX,4);
/*
	char s[256];
	wsprintf(s,"%08X %08X %08X",ulECX,*ulEAX,*ulEDX);
	MessageBox(NULL,s,s,MB_OK);
*/
	IoctlResult = DeviceIoControl(
					handle,				// Handle to device
					IOCTL_WRITE_MSR,	// WRMSR code
					&buf,				// Buffer to driver
					sizeof(buf),		// Length of buffer in bytes.
					&result,			// Buffer from driver.
					sizeof(result),		// Length of buffer in bytes.
					&ReturnedLength,	// Bytes placed in outbuf.
					NULL				// NULL
					);

	return IoctlResult;
}

void WINAPI _Hlt()
{
	ULONG ReturnedLength;
 	BOOL IoctlResult;
	ULONG result;
	char buf[4];

	IoctlResult = DeviceIoControl(
					handle,				// Handle to device
					IOCTL_HLT,			// HLT code
					&buf,				// Buffer to driver
					sizeof(buf),		// Length of buffer in bytes.
					&result,			// Buffer from driver.
					sizeof(result),		// Length of buffer in bytes.
					&ReturnedLength,	// Bytes placed in outbuf.
					NULL				// NULL
					);

	return ;
}
