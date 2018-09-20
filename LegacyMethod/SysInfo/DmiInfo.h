/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include <windows.h>

typedef struct _DmiHeader
{
	UCHAR Type;
	UCHAR Length;
	WORD  Handle;
}DmiHeader;

class CDmiInfo
{
public:
	CDmiInfo();
	~CDmiInfo();
	int		GetString(DWORD ID,char* pStr);
	int		GetData(DWORD ID,DWORD* pData);
	void	InitData();
protected:
	BOOL	DmiStatus;
	char	DmiVersion[8];
	char	DmiBiosVendor[65];
	char	DmiBiosVersion[65];
	char	DmiBiosReleaseDate[65];
	char	DmiBiosRomSize[65];
	char	DmiMotherManufacturer[65];
	char	DmiMotherProduct[65];
	char	DmiMotherVersion[65];
	char	DmiCpuSocket[65];
	char	DmiCpuManufacturer[65];
	char	DmiCpuVersion[65];
	char	DmiCpuCurrentClock[65];
	char	DmiCpuExternalClock[65];
	char	DmiCpuMaxClock[65];
};