#ifndef _SMBIOS_HEADER_
#define _SMBIOS_HEADER_

#pragma pack(push)
#pragma pack(1)

typedef struct {
	UCHAR	Signature[4];
	UCHAR	Chksum;
	UCHAR	Length;
	UCHAR	VerMajor;
	UCHAR	VerMinor;
	UINT16	MaxSize;
	UCHAR	EPSRevision;
	UCHAR	Formatter[5];
	UCHAR	DMISignature[5];
	UCHAR	DMIChksum;
	UINT16	StructLength;
	ULONG	StructAddress;
	UINT16	NumStruct;
	UCHAR	BCDRevision;
} SMBIOS_EPS, *PSMBIOS_EPS;

// ref. section 3.1.2 in spec.
typedef struct {
	UCHAR	Type;
	UCHAR	Length;
	UINT16	Handle;
} SMBIOS_STRUCT_HEADER, *PSMBIOS_STRUCT_HEADER;

#pragma pack(pop)

#endif
/* end of _SMBIOS_HEADER_ */