/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2004 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "PciInfo.h"
#include "../SysInfo/ItemID.h"

#include "Pcifunc.h"
#include <stdio.h>

static void SearchVendorName(DWORD VendorID,char* path,char* FileName,char* DeviceName,BOOL flag);
static int SearchDeviceName(DWORD VendorID,DWORD DeviceID,DWORD RevisionID,char* path,char* FileName,char* DeviceName);
static void SearchDeviceNamePciDB(DWORD VendorID,DWORD DeviceID,DWORD RevisionID,char* path,char* FileName,char* DeviceName);
static int SearchChipSetName(DWORD NVendorID,DWORD NDeviceID,DWORD NRevisionID,DWORD SVendorID,DWORD SDeviceID,DWORD SRevisionID,char* path,char* FileName,char* DeviceName);
static void SearchClassName(DWORD ClassID,char* path,char* FileName,char* ClassName);

CPciInfo::CPciInfo()
{
	InitData();
	if( getdllstatus() == DLLSTATUS_NOERROR )
	{
		GetDeviceID();
		SetDeviceName();
		Exception();
		SearchAllDevice();
	}
}

CPciInfo::~CPciInfo()
{
}

void CPciInfo::Exception()
{
	// E7210 or 875P
	if(NorthID == 0x80862578){
		if(SouthID == 0x808625A1){ // Intel 6300ESB
			strcpy(NorthDeviceName,"Intel E7210");
		}else{
			strcpy(NorthDeviceName,"Intel 875P");
		}
	}

	// 845D or 845MP
	if(NorthID == 0x80861A30 && 4 <= NorthRevisionID && NorthRevisionID < 11){
		if(SouthID == 0x8086248C){ //82801CAM (ICH3-M)
			strcpy(NorthDeviceName,"Intel 82845MP");
		}else{
			strcpy(NorthDeviceName,"Intel 82845D");
		}
	}

#ifndef _X86_64
	unsigned int dwEAX,Family;
	_asm
	{
		mov eax,1
		cpuid
		mov dwEAX,eax
	}
	Family	= (dwEAX>>8) & 0xF;

	// for VIA C7
	if(NorthID == 0x11060314){
		if(Family == 0x6){
			strcpy(ChipSetNameSI,"VIA CN700");
			strcpy(NorthDeviceName,"VIA CN700");
		}
	}

	// 852GM or 855GM
	if(NorthID == 0x80863580){
		if(Family = 0xF){ // Pentium 4-M
			strcpy(ChipSetNameSI,"Intel 852GM (Montara-GML)");
			strcpy(NorthDeviceName,"Intel 82852GM");			
		}else{ // Pentium M
			strcpy(ChipSetNameSI,"Intel 855GM (Montara-GM)");
			strcpy(NorthDeviceName,"Intel 82855GM");
		}
	}
#endif

	// South 判定不能チップ対策
	if(SouthID == 0x10395513 || SouthID == 0x10390008){
		if(NorthID == 0x10390658){strcpy(SouthDeviceName,"SiS 963");}
		if(NorthID == 0x10390655){strcpy(SouthDeviceName,"SiS 963");}
		if(NorthID == 0x10390651){strcpy(SouthDeviceName,"SiS 961/962");}
		if(NorthID == 0x10390650){strcpy(SouthDeviceName,"SiS 961/962");}
		if(NorthID == 0x10390648){strcpy(SouthDeviceName,"SiS 963");}
		if(NorthID == 0x10390646){strcpy(SouthDeviceName,"SiS 961/962");}
		if(NorthID == 0x10390645){strcpy(SouthDeviceName,"SiS 961");}

		if(NorthID == 0x10390755){strcpy(SouthDeviceName,"SiS 963");}
		if(NorthID == 0x10390746){strcpy(SouthDeviceName,"SiS 963");}

		if(NorthID == 0x10390740){strcpy(SouthDeviceName,"SiS 961");}
	}

	// 統合チップセット対策 //
	// SiS 745
	if(NorthID == 0x10390745){strcpy(SouthDeviceName,"SiS 745");}
	// SiS 735
	if(NorthID == 0x10390735){strcpy(SouthDeviceName,"SiS 735");}
	// SiS 733
	if(NorthID == 0x10390733){strcpy(SouthDeviceName,"SiS 733");}
	// SiS 730
	if(NorthID == 0x10390730){strcpy(SouthDeviceName,"SiS 730");}

	// SiS 630
	if(NorthID == 0x10390630){strcpy(SouthDeviceName,"SiS 630");}
	// SiS 633
	if(NorthID == 0x10390633){strcpy(SouthDeviceName,"SiS 633");}
	// SiS 635
	if(NorthID == 0x10390635){strcpy(SouthDeviceName,"SiS 635");}

}

int CPciInfo::GetString( DWORD ID , char* s )
{
	if(ID >= PCI_CLASS_NAME_BASE && ID < PCI_CLASS_NAME_BASE + MAX_SUPPORT_PCI_DEVICE){
		strcpy(s,ClassName[ID - PCI_CLASS_NAME_BASE]);				return 0;
	}else if(ID >= PCI_VENDOR_NAME_BASE && ID < PCI_VENDOR_NAME_BASE + MAX_SUPPORT_PCI_DEVICE){
		strcpy(s,VendorName[ID - PCI_VENDOR_NAME_BASE]);			return 0;
	}else if(ID >= PCI_DEVICE_NAME_BASE && ID < PCI_DEVICE_NAME_BASE + MAX_SUPPORT_PCI_DEVICE){
		strcpy(s,DeviceName[ID - PCI_DEVICE_NAME_BASE]);			return 0;
	}else if(ID >= PCI_SUB_VENDOR_NAME_BASE && ID < PCI_SUB_VENDOR_NAME_BASE + MAX_SUPPORT_PCI_DEVICE){
		strcpy(s,SubVendorName[ID - PCI_SUB_VENDOR_NAME_BASE]);		return 0;
	}

	switch(ID)
	{
	case	PCI_CHIP_SET_NAME_SI:		strcpy(s,ChipSetNameSI);		break;
	case	PCI_NORTH_VENDOR_NAME:		strcpy(s,NorthVendorName);		break;
	case	PCI_NORTH_DEVICE_NAME:		strcpy(s,NorthDeviceName);		break;
	case	PCI_NORTH_SUB_NAME:			strcpy(s,NorthSubName);			break;
	case	PCI_NORTH_CLASS_NAME:		strcpy(s,NorthClassName);		break;
	case	PCI_SOUTH_VENDOR_NAME:		strcpy(s,SouthVendorName);		break;
	case	PCI_SOUTH_DEVICE_NAME:		strcpy(s,SouthDeviceName);		break;
	case	PCI_SOUTH_SUB_NAME:			strcpy(s,SouthSubName);			break;
	case	PCI_SOUTH_CLASS_NAME:		strcpy(s,SouthClassName);		break;
	case	PCI_VIDEO_VENDOR_NAME:		strcpy(s,VideoVendorName);		break;
	case	PCI_VIDEO_DEVICE_NAME:		strcpy(s,VideoDeviceName);		break;
	case	PCI_VIDEO_SUB_NAME:			strcpy(s,VideoSubName);			break;
	case	PCI_VIDEO_CLASS_NAME:		strcpy(s,VideoClassName);		break;
	default:
		strcpy(s,"");
		return -1;
	}
	return 0;
}

int CPciInfo::GetData( DWORD ID , DWORD* d)
{
	if(ID >= PCI_PCI_FUNCTION_BASE && ID < PCI_PCI_FUNCTION_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = PciFunction[ID - PCI_PCI_FUNCTION_BASE];		return 0;
	}else if(ID >= PCI_PCI_DEVICE_BASE && ID < PCI_PCI_DEVICE_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = PciDevice[ID - PCI_PCI_DEVICE_BASE];			return 0;
	}else if(ID >= PCI_PCI_BUS_BASE && ID < PCI_PCI_BUS_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = PciBus[ID - PCI_PCI_BUS_BASE];			return 0;
	}else if(ID >= PCI_CLASS_ID_BASE && ID < PCI_CLASS_ID_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = ClassID[ID - PCI_CLASS_ID_BASE];			return 0;
	}else if(ID >= PCI_SUBSYSTEM_ID_BASE && ID < PCI_SUBSYSTEM_ID_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = SubSystemID[ID - PCI_SUBSYSTEM_ID_BASE];	return 0;
	}else if(ID >= PCI_REVISION_ID_BASE && ID < PCI_REVISION_ID_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = RevisionID[ID - PCI_REVISION_ID_BASE];		return 0;
	}else if(ID >= PCI_DEVICE_ID_BASE && ID < PCI_DEVICE_ID_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = DeviceID[ID - PCI_DEVICE_ID_BASE];			return 0;
	}else if(ID >= PCI_VENDOR_ID_BASE && ID < PCI_VENDOR_ID_BASE + MAX_SUPPORT_PCI_DEVICE){
		*d = VendorID[ID - PCI_VENDOR_ID_BASE];			return 0;
	}

	switch(ID)
	{
	case	PCI_UNKNOWN:				*d = PciUnknown;		break;
	case	PCI_NORTH_VENDOR_ID:		*d = NorthVendorID;		break;
	case	PCI_NORTH_DEVICE_ID:		*d = NorthDeviceID;		break;
	case	PCI_NORTH_REVISION_ID:		*d = NorthRevisionID;	break;
	case	PCI_NORTH_SUSSYSTEM_ID:		*d = NorthSubSystemID;	break;
	case	PCI_NORTH_ID:				*d = NorthID;			break;
	case	PCI_SOUTH_VENDOR_ID:		*d = SouthVendorID;		break;
	case	PCI_SOUTH_DEVICE_ID:		*d = SouthDeviceID;		break;
	case	PCI_SOUTH_REVISION_ID:		*d = SouthRevisionID;	break;
	case	PCI_SOUTH_SUSSYSTEM_ID:		*d = SouthSubSystemID;	break;
	case	PCI_SOUTH_ID:				*d = SouthID;			break;
	case	PCI_VIDEO_VENDOR_ID:		*d = VideoVendorID;		break;
	case	PCI_VIDEO_DEVICE_ID:		*d = VideoDeviceID;		break;
	case	PCI_VIDEO_REVISION_ID:		*d = VideoRevisionID;	break;
	case	PCI_VIDEO_SUSSYSTEM_ID:		*d = VideoSubSystemID;	break;
	case	PCI_VIDEO_ID:				*d = VideoID;			break;
	case	PCI_NUMBER_OF_DEVICE:		*d = DetectedDevice;	break;
	default:
		*d = -1;
		return -1;
	}
	return 0;
}

void CPciInfo::InitData( )
{
	strcpy(ChipSetNameSI,"");
	strcpy(NorthVendorName,"");
	strcpy(NorthDeviceName,"");
	strcpy(NorthSubName,"");
	strcpy(NorthClassName,"");
	strcpy(SouthVendorName,"");
	strcpy(SouthDeviceName,"");
	strcpy(SouthSubName,"");
	strcpy(SouthClassName,"");
	strcpy(VideoVendorName,"");
	strcpy(VideoDeviceName,"");
	strcpy(VideoSubName,"");
	strcpy(VideoClassName,"");

	PciUnknown = 0;

	NorthVendorID	= 0;
	NorthDeviceID	= 0;
	NorthRevisionID = 0;
	NorthSubSystemID= 0;
	NorthID			= 0;

	SouthVendorID	= 0;
	SouthDeviceID	= 0;
	SouthRevisionID = 0;
	SouthSubSystemID= 0;
	SouthID			= 0;

	VideoVendorID	= 0;
	VideoDeviceID	= 0;
	VideoRevisionID = 0;
	VideoSubSystemID= 0;
	VideoID			= 0;

	DetectedDevice	= 0;

	for(int i=0;i < MAX_SUPPORT_PCI_DEVICE;i++)
	{
		VendorID[i] = 0;
		DeviceID[i] = 0;
		RevisionID[i] = 0;
		SubSystemID[i] = 0;
		ClassID[i] = 0;
		PciBus[i] = 0;
		PciDevice[i] = 0;
		PciFunction[i] = 0;
		strcpy(ClassName[i],"");
		strcpy(VendorName[i],"");
		strcpy(DeviceName[i],"");
		strcpy(SubVendorName[i],"");
	}
}

DWORD gAddress;
DWORD gNorthID;

BOOL gK7FVID = FALSE;
BOOL gK8FVID = FALSE;

void SetPcr();
void UnsetPcr();
void EnableK8FVID();
double GetK8HTMulti();

void SetPcr()
{
	DWORD address = gAddress;
	DWORD NorthID = gNorthID;
	UCHAR c;

	if( gK7FVID == TRUE ){
		return;
	}
	gK7FVID = TRUE;

	// Enable Change Multiplier for AMD 761
	if(	NorthID == 0x1022700C // AMD-762 (760MP/MPX)
	||	NorthID == 0x1022700E // AMD-761 (760)
	){
		c = _pciConfigReadChar( address >> 16, 0x44);
		if( ! (c & 0x1) ){
			c |= 0x1;
			_pciConfigWriteChar( address >> 16, 0x44, c);
		}
	}
	// Enable Change Multiplier for VIA KT133
	if(	NorthID == 0x11060305 // KT133
	||	NorthID == 0x11060391 // KX133
	||	NorthID == 0x11060601 // PLE133
	||	NorthID == 0x11060605 // ProSavegeDDR PM133/PN133
	||	NorthID == 0x11063112 // Apollo KLE133
	){
		c = _pciConfigReadChar( address >> 16, 0x55);
		if( ! (c & 0x4) ){ 
			c |= 0x4;
			_pciConfigWriteChar( address >> 16, 0x55, c);
		}
	}

	// Enable Change Multiplier for NVIDIA nForce2
	if(	NorthID == 0x10DE01E0 // nForce2
	||	NorthID == 0x10DE01F0 // nForce2
	){
		c = _pciConfigReadChar( address >> 16, 0xF6);
		if( ! (c & 0x10) ){ // Bit 4 
			c |= 0x10;
			_pciConfigWriteChar( address >> 16, 0xF6, c);
		}
		c = _pciConfigReadChar( address >> 16, 0xE7);
		if( ! (c & 0x10) ){ // Bit 4 
			c |= 0x10;
			_pciConfigWriteChar( address >> 16, 0xE7, c);
		}
	}

}

void UnsetPcr()
{
	DWORD address = gAddress;
	DWORD NorthID = gNorthID;
	UCHAR c;

	// Disable Change Multiplier for AMD 761
	if(	NorthID == 0x1022700C // AMD-762 (760MP/MPX)
	||	NorthID == 0x1022700E // AMD-761 (760)
	){
		c = _pciConfigReadChar( address >> 16, 0x44);
		if( c & 0x1 ){ // Bit 0 
			c ^= 0x1;
			_pciConfigWriteChar( address >> 16, 0x44, c);
		}
	}
	// Disable Change Multiplier for VIA KT133
	if(	NorthID == 0x11060305 // KT133
	||	NorthID == 0x11060391 // KX133
	||	NorthID == 0x11060601 // PLE133
	||	NorthID == 0x11060605 // ProSavegeDDR PM133/PN133
	||	NorthID == 0x11063112 // Apollo KLE133
	){
		c = _pciConfigReadChar( address >> 16, 0x55);
		if( c & 0x4 ){ // Bit 2 
			c ^= 0x4;
			_pciConfigWriteChar( address >> 16, 0x55, c);
		}
	}

	// Disable Change Multiplier for NVIDIA nForce2
	if(	NorthID == 0x10DE01E0 // nForce2
	||	NorthID == 0x10DE01F0 // nForce2
	){
		c = _pciConfigReadChar( address >> 16, 0xF6);
		if( c & 0x10 ){ // Bit 4 
			c ^= 0x10;
			_pciConfigWriteChar( address >> 16, 0xF6, c);
		}
		c = _pciConfigReadChar( address >> 16, 0xE7);
		if( c & 0x10 ){ // Bit 4 
			c ^= 0x10;
			_pciConfigWriteChar( address >> 16, 0xE7, c);
		}
	}
}

void EnableK8FVID()
{
	DWORD address;
	UCHAR c;

	if( gK8FVID == TRUE ){
		return;
	}
	gK8FVID = TRUE;

	address = _pciFindPciDevice(0x1022, 0x1103, 0);
	if(address == 0x86){
		return;
	}
	c = _pciConfigReadChar( address >> 16, 0x82);
	c |= 0x04;
	_pciConfigWriteChar( address >> 16, 0x82, c);
}

double GetK8HTMulti()
{
	DWORD address;
	DWORD c;

	address = _pciFindPciDevice(0x1022, 0x1100, 0);
	if((address & 0xFF) == 0x86){
		return -1.0;
	}
	c = _pciConfigReadLong( address >> 16, 0x88);

	switch((c >> 8) & 0xF){
	case 0x0: return 1.0; // 200MHz
	case 0x2: return 2.0; // 400MHz
	case 0x4: return 3.0; // 600MHz
	case 0x5: return 4.0; // 800MHz
	case 0x6: return 5.0; // 1000MHz
	case 0x7: return 6.0; // 1200MHz
	case 0x8: return 7.0; // 1400MHz
	case 0x9: return 8.0; // 1600MHz
	case 0xA: return 9.0; // 1800MHz
	case 0xB: return 10.0;// 2000MHz
	case 0xC: return 11.0;// 2200MHz
	case 0xD: return 12.0;// 2400MHz
	case 0xE: return 13.0;// 2600MHz
	case 0xF: return 0.5; // 100MHz
	default:
		return -1.0;
	}
	return -1.0;
}

void CPciInfo::GetDeviceID()
{
	DWORD data,address;

	// SouthBridge // ISA (LPC) -> Busmaster IDE ->  Non Busmaster IDE -> EISA
	address = _pciFindPciClass(0x06,0x01,0x00,0);
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x01,0x01,0x80,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x01,0x01,0x8A,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x01,0x01,0xFA,0); }

	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x01,0x01,0x01,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x01,0x01,0x00,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x06,0x02,0x00,0); }
	
	data = _pciConfigReadLong(address >> 16 ,0);
	SouthVendorID = data & 0x0000FFFF;
	SouthDeviceID = (data & 0xFFFF0000) >> 16;
	SouthID = (SouthVendorID << 16) + SouthDeviceID;
	data = _pciConfigReadLong( address >> 16, 0x8);
	SouthRevisionID = data & 0xFF;
	data = _pciConfigReadLong( address >> 16, 0x2C);
	SouthSubSystemID = ( ( data & 0x0000FFFF ) << 16 ) + ( ( data & 0xFFFF0000 ) >> 16);

	// NorthBridge // HostBridge
	gAddress = address = _pciFindPciClass(0x06,0x00,0x00,0);
	
	// First, check NorthBridge Device
	data = _pciConfigReadLong(address >> 16 ,0);
	NorthVendorID = data & 0x0000FFFF;
	NorthDeviceID = (data & 0xFFFF0000) >> 16;
	gNorthID = NorthID = (NorthVendorID << 16) + NorthDeviceID;

	// exception //
/*
	if( NorthVendorID == 0x1022 && NorthDeviceID == 0x1100 && 
		SouthVendorID == 0x10DE && SouthDeviceID == 0x0050 ){ // NVIDIA nForce4
		gAddress = address = _pciFindPciClass(0x05,0x80,0x00,0); // Ohter Memory Device
	}else if( NorthVendorID == 0x1022 && NorthDeviceID == 0x1100 &&
		SouthVendorID == 0x10DE && (SouthDeviceID == 0x0260 || SouthDeviceID == 0x0261) ){ // NVIDIA GeForce 6100/6150
		gAddress = address = _pciFindPciClass(0x06,0x04,0x00,0); // PCI Express Bridge
	}else{
		gAddress = address = _pciFindPciClass(0x06,0x00,0x00,0);
	}
*/
	if( NorthVendorID == 0x1022 && NorthDeviceID == 0x1100 && SouthVendorID == 0x10DE){ // AMD-8131 + NVIDIA
		gAddress = address = _pciFindPciClass(0x05,0x80,0x00,0);
		if( (address & 0xFF ) == 0x86){	gAddress = address = _pciFindPciClass(0x06,0x04,0x00,0); }
		if( (address & 0xFF ) == 0x86){	gAddress = address = _pciFindPciClass(0x05,0x00,0x00,0); }
	}

	data = _pciConfigReadLong(address >> 16 ,0);
	NorthVendorID = data & 0x0000FFFF;
	NorthDeviceID = (data & 0xFFFF0000) >> 16;
	gNorthID = NorthID = (NorthVendorID << 16) + NorthDeviceID;

	data = _pciConfigReadLong( address >> 16, 0x8);
	NorthRevisionID = data & 0xFF;
	data = _pciConfigReadLong( address >> 16, 0x2C);
	NorthSubSystemID = ( ( data & 0x0000FFFF ) << 16 ) + ( ( data & 0xFFFF0000 ) >> 16);

	// VideoChip //
	// VGA Compatible Controller -> XGA Controller -> 3D Controller -> Other Display Controller
	address = _pciFindPciClass(0x03,0x00,0x00,0);
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x00,0x01,0x00,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x03,0x01,0x00,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x03,0x02,0x00,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x03,0x80,0x00,0); }
	if( (address & 0xFF ) == 0x86){	address = _pciFindPciClass(0x04,0x00,0x00,0); }

	data = _pciConfigReadLong(address >> 16 ,0);
	VideoVendorID = data & 0x0000FFFF;
	VideoDeviceID = (data & 0xFFFF0000) >> 16;
	VideoID = (VideoVendorID << 16) + VideoDeviceID;
	data = _pciConfigReadLong( address >> 16, 0x8);
	VideoRevisionID = data & 0xFF;
	data = _pciConfigReadLong( address >> 16, 0x2C);
	VideoSubSystemID = ( ( data & 0x0000FFFF ) << 16 ) + ( ( data & 0xFFFF0000 ) >> 16);

//	char str[32];sprintf(str,"%04X%04X",SouthID,SouthDeviceID);MessageBox(NULL,str,str,MB_OK);
}

void CPciInfo::SetDeviceName()
{
	// Vendor Check //
	char path[MAX_PATH];char* ptrEnd;
	::GetModuleFileName(NULL,path, MAX_PATH);
	if ( (ptrEnd = strrchr(path, '\\')) != NULL ){*ptrEnd = '\0';}
//	MessageBox(NULL,aINIFile,aINIFile,MB_OK);

	if( SearchDeviceName(NorthVendorID,NorthDeviceID,NorthRevisionID,path,"chip.pci",NorthDeviceName) == 0 ){PciUnknown++;}
	if( SearchDeviceName(SouthVendorID,SouthDeviceID,SouthRevisionID,path,"chip.pci",SouthDeviceName) == 0 ){PciUnknown++;}
	if( SearchDeviceName(VideoVendorID,VideoDeviceID,VideoRevisionID,path,"video.pci",VideoDeviceName) == 0 ){PciUnknown++;}
	if( SearchChipSetName(NorthVendorID,NorthDeviceID,NorthRevisionID,SouthVendorID,SouthDeviceID,SouthRevisionID,path,"chipset.pci",ChipSetNameSI) == 0 ){PciUnknown++;}

}


static void SearchVendorName(DWORD VendorID,char* path,char* FileName,char* DeviceName,BOOL flag)
{
	FILE* fp;
	char ID[16];
	char buf[1024];
	char *id;
	char *name;
	char *token;
	char FullPath[MAX_PATH];

	if(VendorID != 0){
		sprintf(ID,"%04X",VendorID);
		sprintf(DeviceName,"Unknown");
		wsprintf(FullPath,"%s\\data\\%s",path,FileName);
		fp = fopen(FullPath,"r");
		if(fp == NULL){
			wsprintf(FullPath,"%s\\%s",path,FileName);
			fp = fopen(FullPath,"r");
		}
		if(fp != NULL){
			while( fgets(buf,1024,fp) ){
				if(buf[0] == ';' || buf[0] == '\r' || buf[0] == '\n' || buf[0] == '\t'){continue;}
				token = strtok( buf , ",\t\n" );	id    = token;
				token = strtok( NULL, ",\t\n" );	name   = token;
				if( name != NULL && _stricmp(ID,id) == 0 && flag == FALSE){
					sprintf(DeviceName,"%s Unknown",name);
					break;
				}else if(name != NULL && _stricmp(ID,id) == 0){
					sprintf(DeviceName,"%s",name);
					break;
				}
			}
			fclose(fp);
		}
	}else{
		sprintf(DeviceName,"");	
	}
//	MessageBox(NULL,ID,id,MB_OK);
}

static void SearchDeviceNamePciDB(DWORD VendorID,DWORD DeviceID,DWORD RevisionID,char* path,char* FileName,char* DeviceName)
{
	FILE* fp = NULL;
	int flag=0;
	char ID[16];
	char Rev[16];
	char buf[1024];
	char *id;
	char *name;
	char *token;
	char FullPath[MAX_PATH];

	sprintf(ID,"%04X%04X",VendorID,DeviceID);
	sprintf(Rev,"%02X",RevisionID);

// pci.db //
	wsprintf(FullPath,"%s\\data\\%s",path,FileName);
	fp = fopen(FullPath,"r");
	if(fp == NULL){
		wsprintf(FullPath,"%s\\%s",path,FileName);
		fp = fopen(FullPath,"r");
	}
	if(fp != NULL){
		while( fgets(buf,1024,fp) ){
			// 現在は d のみ対応
			if(buf[0] == ';' || buf[0] == '\r' || buf[0] == '\n' || buf[0] == '\t'){continue;}
			token = strtok( buf, ",\t\n" );		id    = token;
			token = strtok( NULL, ",\t\n" );	name  = token;
			if( name != NULL && _stricmp(ID,id) == 0 ){
				strcpy(DeviceName,name);
				flag = 1;
				break;
			}
		}
		fclose(fp);
	}
	if(flag == 0){strcpy(DeviceName,"Unknown");}
}

static int SearchDeviceName(DWORD VendorID,DWORD DeviceID,DWORD RevisionID,char* path,char* FileName,char* DeviceName)
{
	FILE* fp = NULL;
	int flag=0;
	char ID[16];
	char Rev[16];
	char buf[1024];
	char *id;
	char *rev;
	char *name;
	char *token;
	char FullPath[MAX_PATH];

	sprintf(ID,"%04X%04X",VendorID,DeviceID);
	sprintf(Rev,"%02X",RevisionID);

	wsprintf(FullPath,"%s\\data\\%s",path,FileName);
	fp = fopen(FullPath,"r");
	if(fp == NULL){
		wsprintf(FullPath,"%s\\%s",path,FileName);
		fp = fopen(FullPath,"r");
	}
	if(fp != NULL){
		while( fgets(buf,1024,fp) ){
			if(buf[0] == ';' || buf[0] == '\r' || buf[0] == '\n' || buf[0] == '\t'){continue;}
			token = strtok( buf , ",\t\n" );	id    = token;
			token = strtok( NULL, ",\t\n" );	rev   = token;
			token = strtok( NULL, ",\t\n" );	name  = token;

			if( name != NULL && id[7] == 'X' ){
				if( memcmp(ID,id,7) == 0 ){
					strcpy(DeviceName,name);
					flag = 1;
					break;
				}
			}else if( name != NULL && _stricmp(ID,id) == 0 && _stricmp(Rev,rev) >= 0){
				strcpy(DeviceName,name);
				flag = 1;
				break;
			}
		}
		fclose(fp);
	}
	if(flag == 0){SearchVendorName(VendorID,path,"vendor.pci",DeviceName,FALSE);}
	return flag;
	//MessageBox(NULL,ID,DeviceName,MB_OK);
}

static int SearchChipSetName(DWORD NVendorID,DWORD NDeviceID,DWORD NRevisionID,DWORD SVendorID,DWORD SDeviceID,DWORD SRevisionID,char* path,char* FileName,char* DeviceName)
{
	FILE* fp;
	int flag = 0;
	char NID[16];
	char SID[16];
	char NRev[16];
	char SRev[16];
	char buf[1024];
	char *nid;
	char *sid;
	char *nrev;
	char *srev;
	char *name;
	char *token;
	char FullPath[MAX_PATH];

	sprintf(NID,"%04X%04X",NVendorID,NDeviceID);
	sprintf(SID,"%04X%04X",SVendorID,SDeviceID);
	sprintf(NRev,"%02X",NRevisionID);
	sprintf(SRev,"%02X",SRevisionID);

	wsprintf(FullPath,"%s\\data\\%s",path,FileName);
	fp = fopen(FullPath,"r");
	if(fp == NULL){
		wsprintf(FullPath,"%s\\%s",path,FileName);
		fp = fopen(FullPath,"r");
	}
	if(fp != NULL){
		while( fgets(buf,1024,fp) ){
			if(buf[0] == ';' || buf[0] == '\r' || buf[0] == '\n' || buf[0] == '\t'){continue;}
			token = strtok( buf , ",\t\n" );	nid   = token;
			token = strtok( NULL, ",\t\n" );	nrev  = token;
			token = strtok( NULL, ",\t\n" );	sid   = token;
			token = strtok( NULL, ",\t\n" );	srev  = token;
			token = strtok( NULL, ",\t\n" );	name  = token;

			if( name != NULL && nid[7] == 'X' && memcmp(NID,nid,7) == 0){
				strcpy(DeviceName,name);
				flag = 1;
				break;
			}else if( name != NULL && _stricmp(NID,nid) == 0 && _stricmp(NRev,nrev) >= 0){
				if(sid[0] == 'X'){// XXXXXXXX,XX の場合は North だけで決定できる
					strcpy(DeviceName,name);
					flag = 1;
					break;
				}else if( (sid[7] == 'X' && memcmp(SID,sid,7) == 0 ) || _stricmp(SID,sid) == 0){
					if( name != NULL && _stricmp(NRev,nrev) >= 0 ){
						flag = 1;
						strcpy(DeviceName,name);
						break;
					}
				}
			}
		}
		fclose(fp);
	}
	if(flag == 0){SearchVendorName(NVendorID,path,"vendor.pci",DeviceName,FALSE);}
//	MessageBox(NULL,NID,DeviceName,MB_OK);
	return flag;
}
/*
int CPciInfo::GetEntryDataLength()
{

	int Length=0;
	char t[1024];

	Length += sprintf(t,"&ChipSetNameSI=%s",ChipSetNameSI);
	Length += sprintf(t,"&NorthVendorName=%s",NorthVendorName);
	Length += sprintf(t,"&NorthDeviceName=%s",NorthDeviceName);
	Length += sprintf(t,"&NorthSubName=%s",NorthSubName);
	Length += sprintf(t,"&NorthClassName=%s",NorthClassName);
	Length += sprintf(t,"&SouthVendorName=%s",SouthVendorName);
	Length += sprintf(t,"&SouthDeviceName=%s",SouthDeviceName);
	Length += sprintf(t,"&SouthSubName=%s",SouthSubName);
	Length += sprintf(t,"&SouthClassName=%s",SouthClassName);
	Length += sprintf(t,"&VideoVendorName=%s",VideoVendorName);
	Length += sprintf(t,"&VideoDeviceName=%s",VideoDeviceName);
	Length += sprintf(t,"&VideoSubName=%s",VideoSubName);
	Length += sprintf(t,"&VideoClassName=%s",VideoClassName);

	Length += sprintf(t,"&NorthVendorID=%04X",NorthVendorID);
	Length += sprintf(t,"&NorthDeviceID=%04X",NorthDeviceID);
	Length += sprintf(t,"&NorthRevisionID=%02X",NorthRevisionID);
	Length += sprintf(t,"&NorthSubSystemID=%08X",NorthSubSystemID);
	Length += sprintf(t,"&NorthID=%08X",NorthID);
	Length += sprintf(t,"&SouthVendorID=%04X",SouthVendorID);
	Length += sprintf(t,"&SouthDeviceID=%04X",SouthDeviceID);
	Length += sprintf(t,"&SouthRevisionID=%02X",SouthRevisionID);
	Length += sprintf(t,"&SouthSubSystemID=%08X",SouthSubSystemID);
	Length += sprintf(t,"&SouthID=%08X",SouthID);
	Length += sprintf(t,"&VideoVendorID=%04X",VideoVendorID);
	Length += sprintf(t,"&VideoDeviceID=%04X",VideoDeviceID);
	Length += sprintf(t,"&VideoRevisionID=%02X",VideoRevisionID);
	Length += sprintf(t,"&VideoSubSystemID=%08X",VideoSubSystemID);
	Length += sprintf(t,"&VideoID=%08X",VideoID);

	return Length;

}

int CPciInfo::GetEntryData(char* s)
{
	int Length=0;
	char t[1024];

	Length += sprintf(t,"&ChipSetNameSI=%s",ChipSetNameSI);				strcat(s,t);
	Length += sprintf(t,"&NorthVendorName=%s",NorthVendorName);			strcat(s,t);
	Length += sprintf(t,"&NorthDeviceName=%s",NorthDeviceName);			strcat(s,t);
	Length += sprintf(t,"&NorthSubName=%s",NorthSubName);				strcat(s,t);
	Length += sprintf(t,"&NorthClassName=%s",NorthClassName);			strcat(s,t);
	Length += sprintf(t,"&SouthVendorName=%s",SouthVendorName);			strcat(s,t);
	Length += sprintf(t,"&SouthDeviceName=%s",SouthDeviceName);			strcat(s,t);
	Length += sprintf(t,"&SouthSubName=%s",SouthSubName);				strcat(s,t);
	Length += sprintf(t,"&SouthClassName=%s",SouthClassName);			strcat(s,t);
	Length += sprintf(t,"&VideoVendorName=%s",VideoVendorName);			strcat(s,t);
	Length += sprintf(t,"&VideoDeviceName=%s",VideoDeviceName);			strcat(s,t);
	Length += sprintf(t,"&VideoSubName=%s",VideoSubName);				strcat(s,t);
	Length += sprintf(t,"&VideoClassName=%s",VideoClassName);			strcat(s,t);

	Length += sprintf(t,"&NorthVendorID=%04X",NorthVendorID);			strcat(s,t);
	Length += sprintf(t,"&NorthDeviceID=%04X",NorthDeviceID);			strcat(s,t);
	Length += sprintf(t,"&NorthRevisionID=%02X",NorthRevisionID);		strcat(s,t);
	Length += sprintf(t,"&NorthSubSystemID=%08X",NorthSubSystemID);		strcat(s,t);
	Length += sprintf(t,"&NorthID=%08X",NorthID);						strcat(s,t);
	Length += sprintf(t,"&SouthVendorID=%04X",SouthVendorID);			strcat(s,t);
	Length += sprintf(t,"&SouthDeviceID=%04X",SouthDeviceID);			strcat(s,t);
	Length += sprintf(t,"&SouthRevisionID=%02X",SouthRevisionID);		strcat(s,t);
	Length += sprintf(t,"&SouthSubSystemID=%08X",SouthSubSystemID);		strcat(s,t);
	Length += sprintf(t,"&SouthID=%08X",SouthID);						strcat(s,t);
	Length += sprintf(t,"&VideoVendorID=%04X",VideoVendorID);			strcat(s,t);
	Length += sprintf(t,"&VideoDeviceID=%04X",VideoDeviceID);			strcat(s,t);
	Length += sprintf(t,"&VideoRevisionID=%02X",VideoRevisionID);		strcat(s,t);
	Length += sprintf(t,"&VideoSubSystemID=%08X",VideoSubSystemID);		strcat(s,t);
	Length += sprintf(t,"&VideoID=%08X",VideoID);						strcat(s,t);
	return Length;

}
*/
static void SearchClassName(DWORD ClassID,char* path,char* FileName,char* ClassName)
{
	FILE* fp;
	int flag=0;
	char ID[16];
	char buf[1024];
	char *id;
	char *name;
	char *token;
	char FullPath[MAX_PATH];

	sprintf(ID,"%06X",ClassID);

	wsprintf(FullPath,"%s\\data\\%s",path,FileName);
	fp = fopen(FullPath,"r");
	if(fp == NULL){
		wsprintf(FullPath,"%s\\%s",path,FileName);
		fp = fopen(FullPath,"r");
	}
	if(fp != NULL){
		while( fgets(buf,1024,fp) ){
			if(buf[0] == ';' || buf[0] == '\r' || buf[0] == '\n' || buf[0] == '\t'){continue;}
			token = strtok( buf , ",\t\n" );	id    = token;
			token = strtok( NULL, ",\t\n" );	name  = token;

			if( name != NULL && id[4] == 'X' ){
				if( memcmp(ID,id,4) == 0 ){
					strcpy(ClassName,name);
					flag = 1;
					break;
				}
			}else if( name != NULL && _stricmp(ID,id) == 0){
				strcpy(ClassName,name);
				flag = 1;
				break;
			}
		}
		fclose(fp);
	}
//	MessageBox(NULL,ID,ClassName,MB_OK);
}

void CPciInfo::SearchAllDevice()
{
	char path[MAX_PATH];char* ptrEnd;
	::GetModuleFileName(NULL,path, MAX_PATH);
	if ( (ptrEnd = strrchr(path, '\\')) != NULL ){*ptrEnd = '\0';}

	DetectedDevice = 0;
	ULONG bus, dev, func;
	ULONG pci_address;
	ULONG	id;
	ULONG max_bus;
	BYTE type;
	BYTE CapPtr;
	char Buf[256];
	DWORD SubSystemVendorID;

	// _pciBusNumber が通らないマシンあり!!
	max_bus = NumberOfBus;

	for(bus = 0; bus < max_bus; bus++){
		for(dev = 0; dev < NumberOfDevice; dev++){
			for(func = 0; func < NumberOfFunc; func++){
				pci_address = pciBusDevFunc(bus,dev,func);

				id = _pciConfigReadLong(pci_address, 0);
				int vender_id = id & 0xffff;
				if(vender_id != 0xffff && vender_id != 0) {
					PciBus[DetectedDevice] = bus;
					PciDevice[DetectedDevice] = dev;
					PciFunction[DetectedDevice] = func;
					VendorID[DetectedDevice] = id & 0x0000FFFF;
					DeviceID[DetectedDevice] = (id & 0xFFFF0000) >> 16;
					VideoID = (VideoVendorID << 16) + VideoDeviceID;
					id = _pciConfigReadLong( pci_address, 0x8);
					ClassID[DetectedDevice] = (id & 0xFFFFFF00) >> 8;
					RevisionID[DetectedDevice] = id & 0xFF;
					type = _pciConfigReadChar(pci_address, 0x0e);
					id = 0;
					if((type & 0x7F) == 2){ // Type 2 PCI-CardBus Bridge 
						id = _pciConfigReadLong( pci_address, 0x40);			
					}else if((type & 0x7F) == 1){ // Type 1 : PCI-PCI Bridge
						CapPtr = _pciConfigReadChar(pci_address, 0x34); // CAP_PTR
						pciConfigRead(pci_address, 0, Buf, 256);
						do{
							switch (Buf[CapPtr])
							{
							case 0x0D:
								id = _pciConfigReadLong( pci_address, CapPtr + 4);
								break;
							default:
								break;
							}
							// Jump to next capability
							CapPtr = Buf[CapPtr+1];
						}while (CapPtr != 0);		
					}else{ // Type 0
						id = _pciConfigReadLong( pci_address, 0x2C);
					}
					if(id != 0){
						SubSystemID[DetectedDevice] = ( ( id & 0x0000FFFF ) << 16 ) + ( ( id & 0xFFFF0000 ) >> 16);
						SubSystemVendorID = id & 0x0000FFFF;
					}
					SearchClassName(ClassID[DetectedDevice],path,"class.pci",ClassName[DetectedDevice]);
					SearchVendorName(VendorID[DetectedDevice],path,"vendor.pci",VendorName[DetectedDevice],TRUE);
					SearchVendorName(SubSystemVendorID,path,"vendor.pci",SubVendorName[DetectedDevice],TRUE);
					SearchDeviceNamePciDB(VendorID[DetectedDevice],DeviceID[DetectedDevice],RevisionID[DetectedDevice],path,"pci.db",DeviceName[DetectedDevice]);
				//	MessageBox(NULL,ClassName[DetectedDevice],ClassName[DetectedDevice],MB_OK);
					DetectedDevice++;
				}
				// 単機能デバイスの場合はfunc番号１以降は検索しない
				if(func ==0 && (type & 0x80) ==0) break;
			}
		}
	}
}
