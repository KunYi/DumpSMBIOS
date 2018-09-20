#ifndef _SYSTEM_INFO_HEADER_
#define _SYSTEM_INFO_HEADER_

#include "smheader.h"

#pragma pack(push)
#pragma pack(1)

typedef struct _TYPE_1_ {
	SMBIOS_STRUCT_HEADER	Header;
	UCHAR	Manufacturer;
	UCHAR	ProductName;
	UCHAR	Version;
	UCHAR	SN;
	UCHAR	UUID[16];
	UCHAR	WakeUpType;
	UCHAR	SKUNumber;
	UCHAR	Family;
} SystemInfo, *PSystemInfo;

#pragma pack(pop)

#endif
/* end of _SYSTEM_INFO_HEADER_ */