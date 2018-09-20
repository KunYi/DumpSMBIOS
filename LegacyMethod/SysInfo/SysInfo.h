/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

//#include <windows.h>
#include "ISysInfo.h"
#include "ItemID.h"

#include "CpuInfo.h"
#include "PciInfo.h"
#include "DmiInfo.h"

#include "Pcifunc.h"
#include "Port32.h"

DWORD gFlagStatus;

class CSysInfo : public ISysInfo
{
protected:
	CCpuInfo* CPU;
	CPciInfo* PCI;
	CDmiInfo* DMI;

public:
	CSysInfo(DWORD code){
		gFlagStatus = DLLSTATUS_OTHERERROR;
		CPU = NULL;
		PCI = NULL;
		DMI = NULL;
		gFlagStatus = getdllstatus();
		if(code & MODE_CPU){CPU = new CCpuInfo(0x01);}
		if(code & MODE_PCI){PCI = new CPciInfo();}
		if(code & MODE_DMI){DMI = new CDmiInfo();}
	}	

	~CSysInfo(){
		if(PCI != NULL){delete PCI;}
		if(CPU != NULL){delete CPU;}
		if(DMI != NULL){delete DMI;}
	}

	// ISysInfo
	char* GetInfo( DWORD ID , char* pStr );
	int	GetString( DWORD ID , char* pStr );
	int	GetData( DWORD ID , DWORD* pData );
	int	SetData( DWORD ID , DWORD Data );
};

//	COM //
/*
	ULONG RefCount;
	// IUnknown
	HRESULT QueryInterface(REFIID riid , void **ppvObj);
	ULONG AddRef();
	ULONG Release();
*/
