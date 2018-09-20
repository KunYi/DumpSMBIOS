/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2004 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

//#include "ISysInfo.h"
#include "SysInfo.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

extern "C" void InitializeDll();
extern "C" void DeinitializeDll();

extern HRESULT GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );

extern "C" __declspec (dllexport) ISysInfo* CreateSysInfo(DWORD code)
{
	TIMECAPS tc;
	timeGetDevCaps(&tc,sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin);
	InitializeDll();
	return new CSysInfo(code);
}
/*
extern "C" __declspec (dllexport) ISysInfo* CreateSysInfoEx(DWORD code,DWORD mask)
{
	return new CSysInfo(code,mask);
}
*/

extern "C" __declspec (dllexport) void DestroySysInfo(ISysInfo* p)
{
	TIMECAPS tc;
	timeGetDevCaps(&tc,sizeof(TIMECAPS));
	timeEndPeriod(tc.wPeriodMin);
	DeinitializeDll();
	delete (CSysInfo*)p;
}

int	CSysInfo::SetData( DWORD ID , DWORD Data )
{
	if(ID >= CPU_BASE && ID < CPU_BASE + 0xFFF && CPU != NULL){
		return CPU->SetData( ID , Data );
	}
	return -1;
}

char* CSysInfo::GetInfo( DWORD ID , char* s )
{
	GetString(ID, s);
	if( atof(s) < 0 ){
		strcpy(s,"");
	}
	return s;
}

int	CSysInfo::GetData( DWORD ID , DWORD* pData )
{
	if(ID == SI_STATUS){
		*pData = gFlagStatus;
		return gFlagStatus;
	}
	// CPU UPDATE
	if(ID >= CPU_UPDATE_BASE && ID < CPU_UPDATE_BASE + 32)
	{
#ifdef _WIN32
		DWORD mask = 1 << (ID - CPU_UPDATE_BASE);
		// pData == TimerType
		if(CPU != NULL){
			CPU->Init( mask, *pData );
		}else{
			CPU = new CCpuInfo( mask );
		}
		*pData = 0;
#else if // for UNIX
		if(CPU != NULL){
			CPU->Init();
		}else{
			CPU = new CCpuInfo();
		}
#endif
	}
	if(ID >= PCI_DEVICE_BASE && ID < PCI_DEVICE_BASE + 0xFFF && PCI != NULL){return PCI->GetData( ID , pData );}
	if(ID >= PCI_BASE && ID < PCI_BASE + 0xFFF && PCI != NULL){return PCI->GetData( ID , pData );}
	if(ID >= CPU_BASE && ID < CPU_BASE + 0xFFF && CPU != NULL){return CPU->GetData( ID , pData );}
	if(ID >= DMI_BASE && ID < DMI_BASE + 0xFFF && DMI != NULL){return DMI->GetData( ID , pData );}

	*pData = -1;
	return -1;
}

int CSysInfo::GetString( DWORD ID , char* s )
{
	DWORD data;

	if( ID == CPU_PLATFORM_NAME && DMI != NULL && CPU != NULL){
		DMI->GetString( DMI_CPU_SOCKET , s );
		CPU->GetData(CPU_FAMILY, &data);
		if(data == 6){
			if(strstr(s,"Slot 1") != NULL ){
				strcpy(s,"Slot 1");
				return 0;
			}else if(strstr(s,"Slot A") != NULL || strstr(s,"Slot-A") != NULL){
				strcpy(s,"Slot A");
				return 0;
			}else if(strstr(s,"Socket A") != NULL || strstr(s,"Socket-A") != NULL){
				strcpy(s,"Socket A");
				return 0;
			}
		}
		return CPU->GetString( ID , s );
	}

	if(ID >= PCI_DEVICE_BASE && ID < PCI_DEVICE_BASE + 0xFFF && PCI != NULL){return PCI->GetString( ID , s );}
	if(ID >= PCI_BASE && ID < PCI_BASE + 0xFFF && PCI != NULL){return PCI->GetString( ID , s );}
	if(ID >= CPU_BASE && ID < CPU_BASE + 0xFFF && CPU != NULL){return CPU->GetString( ID , s );}
	if(ID >= DMI_BASE && ID < DMI_BASE + 0xFFF && DMI != NULL){return DMI->GetString( ID , s );}
	
	// CSysInfo //
	switch ( ID )
	{
		case	SI_VERSION:		strcpy(s,SYS_INFO_VERSION);	break;
		case	SI_DATE:		strcpy(s,SYS_INFO_DATE);	break;
		case	SI_AUTHOR:		strcpy(s,SYS_INFO_AUTHOR);	break;
		case	DIRECT_X_VERSION:
				DWORD i;
				GetDXVersion( &i, s, 16);
				break;
		default:
			strcpy(s,"");
			return -1;
	}
	return 0;
}