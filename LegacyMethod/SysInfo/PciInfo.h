/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include <windows.h>
#include "../SysInfo/ItemID.h"

class CPciInfo
{
public:

	CPciInfo();
	~CPciInfo();
	int GetString(DWORD ID,char* pStr);
	int GetData(DWORD ID,DWORD* pData);

//	int GetEntryDataLength();
//	int GetEntryData(char* pStr);

protected:

	void InitData();
	void GetDeviceID();
	void SetDeviceName();
	void Exception();
	void SearchAllDevice();


protected:

// String
	char		ChipSetNameSI[65];
	char		NorthVendorName[33];
	char		NorthDeviceName[129];
	char		NorthSubName[33];
	char		NorthClassName[33];
	char		SouthVendorName[33];
	char		SouthDeviceName[129];
	char		SouthSubName[33];
	char		SouthClassName[33];
	char		VideoVendorName[33];
	char		VideoDeviceName[129];
	char		VideoSubName[33];
	char		VideoClassName[33];

// DWORD
	DWORD		PciUnknown;
	DWORD		NorthVendorID;
	DWORD		NorthDeviceID;
	DWORD		NorthRevisionID;
	DWORD		NorthSubSystemID;
	DWORD		NorthID;
	DWORD		SouthVendorID;
	DWORD		SouthDeviceID;
	DWORD		SouthRevisionID;
	DWORD		SouthSubSystemID;
	DWORD		SouthID;
	DWORD		VideoVendorID;
	DWORD		VideoDeviceID;
	DWORD		VideoRevisionID;
	DWORD		VideoSubSystemID;
	DWORD		VideoID;

// PCI_DEVICE_BASE 
	DWORD		DetectedDevice;
	DWORD		VendorID[MAX_SUPPORT_PCI_DEVICE];
	DWORD		DeviceID[MAX_SUPPORT_PCI_DEVICE];
	DWORD		RevisionID[MAX_SUPPORT_PCI_DEVICE];
	DWORD		SubSystemID[MAX_SUPPORT_PCI_DEVICE];
	DWORD		ClassID[MAX_SUPPORT_PCI_DEVICE];
	DWORD		PciBus[MAX_SUPPORT_PCI_DEVICE];
	DWORD		PciDevice[MAX_SUPPORT_PCI_DEVICE];
	DWORD		PciFunction[MAX_SUPPORT_PCI_DEVICE];
	char		ClassName[MAX_SUPPORT_PCI_DEVICE][64];
	char		VendorName[MAX_SUPPORT_PCI_DEVICE][64];
	char		DeviceName[MAX_SUPPORT_PCI_DEVICE][64];
	char		SubVendorName[MAX_SUPPORT_PCI_DEVICE][64];
};