/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#ifndef __MSR_NT_H__
#define __MSR_NT_H__
/*
#define DRIVERNAME_BASE		"CpuInfo"
#define DRIVERNAME "\\\\.\\"DRIVERNAME_BASE

#define DRIVERID			"CpuInfo"
#define DRIVERFILENAME		"CpuInfo.sys"
*/
extern "C" HANDLE handle;

int WINAPI _ReadMSR(ULONG ulECX, ULONG* ulEAX, ULONG* ulEDX);
int WINAPI _WriteMSR(ULONG ulECX, ULONG* ulEAX, ULONG* ulEDX);
void WINAPI _Hlt();

#define MSR_TYPE 40000

#define IOCTL_READ_MSR \
	CTL_CODE(MSR_TYPE, 0x981, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_WRITE_MSR \
	CTL_CODE(MSR_TYPE, 0x982, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HLT \
	CTL_CODE(MSR_TYPE, 0x983, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif // __MSR_NT_H__
