#ifndef _SYTEM_ENCLOSURE_HEADER_
#define _SYTEM_ENCLOSURE_HEADER_

#include "smheader.h"

typedef struct _TYPE_3_ {
	SMBIOS_STRUCT_HEADER Header;
	UCHAR	Manufacturer;
	UCHAR	Type;
	UCHAR	Version;
	UCHAR	SN;
	UCHAR	AssetTag;
	UCHAR	BootupState;
	UCHAR	PowerSupplyState;
	UCHAR	ThermalState;
	UCHAR	SecurityStatus;
	ULONG32	OEMDefine;
	UCHAR	Height;
	UCHAR	NumPowerCord;
	UCHAR	ElementCount;
	UCHAR	ElementRecordLength;
	UCHAR	pElements;
} SystemEnclosure, *PSystemEnclosure;

#endif
/* end of _SYTEM_ENCLOSURE_HEADER_ */