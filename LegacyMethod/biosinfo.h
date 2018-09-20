#ifndef _BIOS_INFO_HEADER_
#define _BIOS_INFO_HEADER_

#include "smheader.h"

#pragma pack(push)
#pragma pack(1)
typedef struct _TYPE_0_ {
	SMBIOS_STRUCT_HEADER	Header;
	UCHAR	Vendor;
	UCHAR	Version;
	UINT16	StartingAddrSeg;
	UCHAR	ReleaseDate;
	UCHAR	ROMSize;
	ULONG64 Characteristics;
	UCHAR	Extension[2]; // spec. 2.3
	UCHAR	MajorRelease;
	UCHAR	MinorRelease;
	UCHAR	ECFirmwareMajor;
	UCHAR	ECFirmwareMinor;
} BIOSInfo, *PBIOSInfo;

#pragma pack(pop)

#endif
/* end of _BIOS_INFO_HEADER_ */