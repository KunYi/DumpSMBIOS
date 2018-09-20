#ifndef _BOARD_INFO_HEADER_
#define _BOARD_INFO_HEADER_

#include "smheader.h"

typedef struct _TYPE_2_ {
	SMBIOS_STRUCT_HEADER	Header;
	UCHAR	Manufacturer;
	UCHAR	Product;
	UCHAR	Version;
	UCHAR	SN;
	UCHAR	AssetTag;
	UCHAR	FeatureFlags;
	UCHAR	LocationInChassis;
	UINT16	ChassisHandle;
	UCHAR	Type;
	UCHAR	NumObjHandle;
	UINT16	*pObjHandle;
} BoardInfo, *PBoardInfo;

#endif
/* end of BOARD_INFO_HEADER_ */