/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#ifndef __I_SYS_INFO_H__
#define __I_SYS_INFO_H__

#include <windows.h>

interface ISysInfo {	// : public IUnknown{
	virtual char* GetInfo( DWORD ID , char* pStr ) = 0;
	virtual int	GetString( DWORD ID , char* pStr ) = 0;
	virtual int	GetData( DWORD ID , DWORD* pData ) = 0;
	virtual int	SetData( DWORD ID , DWORD Data ) = 0;
};

extern "C" __declspec (dllexport) ISysInfo* CreateSysInfo(DWORD code);
// extern "C" __declspec (dllexport) ISysInfo* CreateSysInfoEx(DWORD code,DWORD mask);
extern "C" __declspec (dllexport) void DestroySysInfo(ISysInfo* p);

#endif