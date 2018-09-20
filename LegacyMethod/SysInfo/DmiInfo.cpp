/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "DmiInfo.h"
#include "../SysInfo/ItemID.h"

#include <stdio.h>
#include "Pcifunc.h"

CDmiInfo::CDmiInfo()
{
	InitData();
}

CDmiInfo::~CDmiInfo()
{

}

int CDmiInfo::GetString( DWORD ID , char* s )
{
	switch(ID)
	{
	case	DMI_VERSION:			strcpy(s,DmiVersion);				break;
	case	DMI_BIOS_VENDOR:		strcpy(s,DmiBiosVendor);			break;
	case	DMI_BIOS_VERSION:		strcpy(s,DmiBiosVersion);			break;
	case	DMI_BIOS_RELEASE_DATE:	strcpy(s,DmiBiosReleaseDate);		break;
	case	DMI_BIOS_ROM_SIZE:		strcpy(s,DmiBiosRomSize);			break;
	case	DMI_MOTHER_MANUFACTURER:strcpy(s,DmiMotherManufacturer);	break;
	case	DMI_MOTHER_PRODUCT:		strcpy(s,DmiMotherProduct);			break;
	case	DMI_MOTHER_VERSION:		strcpy(s,DmiMotherVersion);			break;
	case	DMI_CPU_SOCKET:			strcpy(s,DmiCpuSocket);				break;
	case	DMI_CPU_MANUFACTURER:	strcpy(s,DmiCpuManufacturer);		break;
	case	DMI_CPU_VERSION:		strcpy(s,DmiCpuVersion);			break;
	case	DMI_CPU_CURRENT_CLOCK:	strcpy(s,DmiCpuCurrentClock);		break;
	case	DMI_CPU_EXTERNAL_CLOCK:	strcpy(s,DmiCpuExternalClock);		break;
	case	DMI_CPU_MAX_CLOCK:		strcpy(s,DmiCpuMaxClock);			break;
	default:
		strcpy(s,"");
		return -1;
		break;
	}
	return 0;
}

int CDmiInfo::GetData( DWORD ID , DWORD* d)
{
	switch(ID)
	{
	case	DMI_STATUS:					*d = DmiStatus;					break;
	default:
		*d = -1;
		return -1;
	}
	return 0;
}

#define MAP_MEMORY_SIZE 64*1024
#define MAKEDWORD(a,b,c,d)	((d<<24)+(c<<16)+(b<<8)+(a))

static BOOL CheckSum(const BYTE *buf, int length)
{
	BYTE sum = 0;
	for(int i=0;i<length;i++){
		sum += buf[i];
	}
	return (sum==0);
}

static void DmiString(char* str, DmiHeader* dmi, UCHAR id)
{
	char *p = (char *)dmi;
	p += dmi->Length;
	while(id >1 && *p)
	{
		p += strlen(p);
		p++;
		id--;
	}
	// ASCII Filter
	for(DWORD i=0; i<strlen(p); i++){
		if(p[i]<32 || p[i]==127){
			p[i]='.';
		}
	}
	strcpy(str, p);
}

void CDmiInfo::InitData()
{
	DmiStatus = FALSE;
	DmiVersion[0] = '\0';
	DmiBiosVendor[0] = '\0';
	DmiBiosVersion[0] = '\0';
	DmiBiosReleaseDate[0] = '\0';
	DmiBiosRomSize[0] = '\0';
	DmiMotherManufacturer[0] = '\0';
	DmiMotherProduct[0] = '\0';
	DmiMotherVersion[0] = '\0';
	DmiCpuSocket[0] = '\0';
	DmiCpuManufacturer[0] = '\0';
	DmiCpuVersion[0] = '\0';
	DmiCpuCurrentClock[0] = '\0';
	DmiCpuExternalClock[0] = '\0';
	DmiCpuMaxClock[0] = '\0';

	UCHAR b[MAP_MEMORY_SIZE];
	UCHAR *d = NULL;
	_MemReadBlockChar(0x000F0000, b, MAP_MEMORY_SIZE);

	int j;
	UCHAR *p;
	UCHAR *next;
	DWORD EntryPoint;
	DWORD StructureLength;
	DWORD NumberOfStructures;
	BOOL Flag = FALSE;
	p = (UCHAR *)b;
	for(j = 0;j< MAP_MEMORY_SIZE;j+=16)
	{
		if( memcmp( p, "_SM_", 4) == 0 ){
			sprintf(DmiVersion,"%d.%d",p[6],p[7]);
		}
		if( memcmp( p, "_DMI_", 5) == 0 ){
			EntryPoint = MAKEDWORD(p[8],p[9],p[0xA],p[0xB]);
			StructureLength = MAKEWORD(p[6],p[7]);
			NumberOfStructures = MAKEWORD(p[0xC],p[0xD]);
			if( CheckSum(p,0xF) ){
				DmiStatus = TRUE;
				Flag = TRUE;
			}
			break;
		}
		p+=16;
	}

	if( Flag == FALSE ){
		return ;
	}
	d = new UCHAR[StructureLength];
	_MemReadBlockChar(EntryPoint, d, StructureLength);

	DWORD num;
	p = d;
	DmiHeader* dmi;
	for(num = 0;num < NumberOfStructures; num++){
		dmi = (DmiHeader*)p;
		switch ( dmi->Type ){
///////////////////////////////////////////////////////////////////////////////////////////////////
// 00 BIOS Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 0:
			DmiString(DmiBiosVendor,dmi,p[4]);
			DmiString(DmiBiosVersion,dmi,p[5]);
			DmiString(DmiBiosReleaseDate,dmi,p[8]);
			sprintf(DmiBiosRomSize, "%d", ( (p[9] + 1) << 16 ) / 1024 );
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 01 System Information
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
		case 1:
			DmiString(DmiMotherManufacturer,dmi,p[4]);
			DmiString(DmiMotherProduct,dmi,p[5]);
			DmiString(DmiMotherVersion,dmi,p[6]);
			break;
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
// 02 Base Board Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 2:
			DmiString(DmiMotherManufacturer,dmi,p[4]);
			DmiString(DmiMotherProduct,dmi,p[5]);
			DmiString(DmiMotherVersion,dmi,p[6]);
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 04 Processor Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 4:
			DmiString(DmiCpuSocket,dmi,p[4]);
			DmiString(DmiCpuManufacturer,dmi,p[7]);
			DmiString(DmiCpuVersion,dmi,p[0x10]);
			sprintf(DmiCpuExternalClock,"%d",MAKEWORD(p[0x12],p[0x13]));
			sprintf(DmiCpuMaxClock,"%d",MAKEWORD(p[0x14],p[0x15]));
			sprintf(DmiCpuCurrentClock,"%d",MAKEWORD(p[0x16],p[0x17]));
			break;
			}
		// next 
		next = p + dmi->Length;
		while( next[0] !=0 || next[1] != 0 )
			next++;
		p = next + 2;
	}
	delete d;
}