/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2004 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#ifndef __ITEM_ID_H__
#define __ITEM_ID_H__

#include "CpuInfoID.h"

#define	ERROR_MESSAGE "LoadDriver failure!"
#define	ERROR_CAPTION "Error"

////////////
// Version Information
//////////////////////////////////////////

#define		CRYSTAL_SYS_INFO_VERSION	"1.0.0"
#define		CRYSTAL_SYS_INFO_DATE		"2005/6/5"

#define		CRYSTAL_MARK_VERSION_09		"0.9.126"
#define		CRYSTAL_MARK_DATE_09		"2008/7/6"

#define		CRYSTAL_MARK_VERSION_08		"0.8.96"
#define		CRYSTAL_MARK_DATE_08		"2004/7/18"

#define		CRYSTAL_MARK_VERSION_06		"0.6.32.%s"
#define		CRYSTAL_MARK_DATE_06		"2004/7/18"

////////////
// SysInfo.dll
//////////////////////////////////////////

#define		SYS_INFO_VERSION			"410"			// Max 32
#define		SYS_INFO_DATE				"2008/7/6"		// Max 32
#define		SYS_INFO_AUTHOR				"hiyohiyo"		// Max 32

////////////
// SysInfo.dll / CpuInfo.dll Status
/////////////////////////////////////////

#define		SI_VERSION					0x0001
#define		SI_DATE						0x0002
#define		SI_AUTHOR					0x0003

#define		SI_STATUS					0x0010

#define		DIRECT_X_VERSION			0x0300

////////////
// ChipSet
//////////////////////////////////////////

#define		PCI_BASE					0x2000
// String //
#define		PCI_CHIP_SET_NAME_SI		0x2001	// SysInfo.dll Set ChipSet Name
#define		PCI_NORTH_VENDOR_NAME		0x2002	// NorthBridge or MCH Vendor Name
#define		PCI_NORTH_DEVICE_NAME		0x2003	// NorthBridge or MCH Device Name
#define		PCI_NORTH_SUB_NAME			0x2004	// SubSystem Vendor Name
#define		PCI_NORTH_CLASS_NAME		0x2005	// Class Name
#define		PCI_SOUTH_VENDOR_NAME		0x2006	// SouthBridge or ICH Vendor Name
#define		PCI_SOUTH_DEVICE_NAME		0x2007	// SouthBridge or ICH Device Name
#define		PCI_SOUTH_SUB_NAME			0x2008	// SubSystem Vendor Name
#define		PCI_SOUTH_CLASS_NAME		0x2009	// Class Name
#define		PCI_VIDEO_VENDOR_NAME		0x200A	// VideoChip Vendor Name
#define		PCI_VIDEO_DEVICE_NAME		0x200B	// VideoChip Device Name
#define		PCI_VIDEO_SUB_NAME			0x200C	// SubSystem Vendor Name
#define		PCI_VIDEO_CLASS_NAME		0x200D	// Class Name
/*
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
*/
// integer //

#define		PCI_UNKNOWN					0x2010

#define		PCI_NORTH_VENDOR_ID			0x2020
#define		PCI_NORTH_DEVICE_ID			0x2021
#define		PCI_NORTH_REVISION_ID		0x2022
#define		PCI_NORTH_SUSSYSTEM_ID		0x2023
#define		PCI_NORTH_ID				0x2024

#define		PCI_SOUTH_VENDOR_ID			0x2030
#define		PCI_SOUTH_DEVICE_ID			0x2031
#define		PCI_SOUTH_REVISION_ID		0x2032
#define		PCI_SOUTH_SUSSYSTEM_ID		0x2033
#define		PCI_SOUTH_ID				0x2034

#define		PCI_VIDEO_VENDOR_ID			0x2040
#define		PCI_VIDEO_DEVICE_ID			0x2041
#define		PCI_VIDEO_REVISION_ID		0x2042
#define		PCI_VIDEO_SUSSYSTEM_ID		0x2043
#define		PCI_VIDEO_ID				0x2044

/*
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
*/
#define		MAX_SUPPORT_PCI_DEVICE		128
#define		NUMBER_OF_BUS				256
#define		NUMBER_OF_FUNCTION			  8
#define		NUMBER_OF_DEVICE			 32

#define		PCI_DEVICE_BASE				0x5000
#define		PCI_NUMBER_OF_DEVICE		0x5001

#define		PCI_VENDOR_ID_BASE			0x5100
#define		PCI_DEVICE_ID_BASE			0x5200
#define		PCI_REVISION_ID_BASE		0x5300
#define		PCI_SUBSYSTEM_ID_BASE		0x5400
#define		PCI_CLASS_ID_BASE			0x5500
#define		PCI_CLASS_NAME_BASE			0x5600
#define		PCI_PCI_BUS_BASE			0x5700
#define		PCI_PCI_DEVICE_BASE			0x5800
#define		PCI_PCI_FUNCTION_BASE		0x5900
#define		PCI_VENDOR_NAME_BASE		0x5A00
#define		PCI_DEVICE_NAME_BASE		0x5B00
#define		PCI_SUB_VENDOR_NAME_BASE	0x5C00

////////////
// DMI Information
//////////////////////////////////////////
#define		DMI_BASE					0x6000
#define		DMI_STATUS					0x6001
#define		DMI_VERSION					0x6002
#define		DMI_BIOS_VENDOR				0x6100
#define		DMI_BIOS_VERSION			0x6101
#define		DMI_BIOS_RELEASE_DATE		0x6102
#define		DMI_BIOS_ROM_SIZE			0x6103
#define		DMI_MOTHER_MANUFACTURER		0x6200
#define		DMI_MOTHER_PRODUCT			0x6201
#define		DMI_MOTHER_VERSION			0x6202
#define		DMI_CPU_SOCKET				0x6300
#define		DMI_CPU_MANUFACTURER		0x6301
#define		DMI_CPU_VERSION				0x6302
#define		DMI_CPU_CURRENT_CLOCK		0x6303
#define		DMI_CPU_EXTERNAL_CLOCK		0x6304
#define		DMI_CPU_MAX_CLOCK			0x6305
/*
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
*/

////////////
// AGP Information
//////////////////////////////////////////
#define		AGP_BASE					0x7000
#define		AGP_REVISION				0x7001


////////////
// MODE List
//////////////////////////////////////////
#define		MODE_SAFE				0x0001
#define		MODE_PC98				0x0002
#define		MODE_WMI_OFF			0x0004

#define		MODE_CPU				0x0100
#define		MODE_PCI				0x0200
#define		MODE_WIN				0x0400
#define		MODE_WMI				0x0800
#define		MODE_DMI				0x1000
#define		MODE_ALL				0xFF00

////////////
// Debug Information for CrystalMark08
//////////////////////////////////////////
#define		CRYSTAL_MARK_EXEC_START	"CrystalMark Execute Start"
#define		CRYSTAL_MARK_INIT_START	"CrystalMark Initialize Start"
#define		WMI_INIT_START			"WMI Initialize Start"
#define		WMI_INIT_END			"WMI Initialize OK!"
#define		IDE_INIT_START			"Get HDD Infomation Start"
#define		IDE_INIT_END			"Get HDD Infomation OK!"
#define		CRYSTAL_MARK_INIT_END	"CrystalMark Initialize OK!!"
#define		CRYSTAL_MARK_EXEC_END	"CrystalMark Execute End"

#define		SYS_INFO_INIT_START		"SysInfo.dll Init Start"
#define		SYS_INFO_CPU_INIT		"Get CPU Infomation OK!"
#define		SYS_INFO_PCI_INIT		"Get PCI Infomation OK!"
#define		SYS_INFO_INIT_END		"SysInfo.dll Init OK!!"

#endif