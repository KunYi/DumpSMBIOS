// DumpSMBIOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma pack(push) 
#pragma pack(1)
typedef struct _RawSMBIOSData
{
	BYTE	Used20CallingMethod;
	BYTE	SMBIOSMajorVersion;
	BYTE	SMBIOSMinorVersion;
	BYTE	DmiRevision;
	DWORD	Length;
	PBYTE	SMBIOSTableData;
} RawSMBIOSData, *PRawSMBIOSData;

typedef struct _SMBIOSHEADER_
{
	BYTE Type;
	BYTE Length;
	WORD Handle;
} SMBIOSHEADER, *PSMBIOSHEADER;

typedef struct _TYPE_0_ {
	SMBIOSHEADER	Header;
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


typedef struct _TYPE_1_ {
	SMBIOSHEADER	Header;
	UCHAR	Manufacturer;
	UCHAR	ProductName;
	UCHAR	Version;
	UCHAR	SN;
	UCHAR	UUID[16];
	UCHAR	WakeUpType;
	UCHAR	SKUNumber;
	UCHAR	Family;
} SystemInfo, *PSystemInfo;

typedef struct _TYPE_2_ {
	SMBIOSHEADER	Header;
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

typedef struct _TYPE_3_ {
	SMBIOSHEADER Header;
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

typedef struct _TYPE_4_ {
	SMBIOSHEADER Header;
	UCHAR	SocketDesignation;
	UCHAR	Type;
	UCHAR	Family;
	UCHAR	Manufacturer;
	ULONG64 ID;
	UCHAR	Version;
	UCHAR	Voltage;
	UINT16	ExtClock;
	UINT16	MaxSpeed;
	UINT16	CurrentSpeed;
	// Todo, Here

} ProcessorInfo, *PProcessorInfo;

typedef struct _TYPE_5_ {
	SMBIOSHEADER Header;
	// Todo, Here

} MemCtrlInfo, *PMemCtrlInfo;

typedef struct _TYPE_6_ {
	SMBIOSHEADER Header;
	UCHAR	SocketDesignation;
	UCHAR	BankConnections;
	UCHAR	CurrentSpeed;
	// Todo, Here
} MemModuleInfo, *PMemModuleInfo;

typedef struct _TYPE_7_ {
	SMBIOSHEADER Header;
	UCHAR	SocketDesignation;
	UINT16	Configuration;
	UINT16	MaxSize;
	UINT16	InstalledSize;
	UINT16	SupportSRAMType;
	UINT16	CurrentSRAMType;
	UCHAR	Speed;
	UCHAR	ErrorCorrectionType;
	UCHAR	SystemCacheType;
	UCHAR	Associativity;
} CacheInfo, *PCacheInfo;

typedef struct _TYPE_17_ {
	SMBIOSHEADER Header;
	UINT16	PhysicalArrayHandle;
	UINT16	ErrorInformationHandle;
	UINT16	TotalWidth;
	UINT16	DataWidth;
	UINT16	Size;
	UCHAR	FormFactor;
	UCHAR	DeviceSet;
	UCHAR	DeviceLocator;
	UCHAR	BankLocator;
	UCHAR	MemoryType;
	UINT16	TypeDetail;
	UINT16	Speed;
	UCHAR	Manufacturer;
	UCHAR	SN;
	UCHAR	AssetTag;
	UCHAR	PN;
	UCHAR	Attributes;
} MemoryDevice, *PMemoryDevice;

typedef struct _TYPE_19_ {
	SMBIOSHEADER Header;
	ULONG32	Starting;
	ULONG32	Ending;
	UINT16	Handle;
	UCHAR	PartitionWidth;
} MemoryArrayMappedAddress, *PMemoryArrayMappedAddress;

typedef struct _TYPE_22_ {
	SMBIOSHEADER Header;
	UCHAR	Location;
	UCHAR	Manufacturer;
	UCHAR	Date;
	UCHAR	SN;
	UCHAR	DeviceName;

} PortableBattery, *PPortableBattery;
#pragma pack(pop) 

#ifdef UNICODE
#define getHeaderString  getHeaderStringW
#define LocateString	LocateStringW
#else
#define getHeaderString  getHeaderStringA
#define LocateString	LocateStringA
#endif

const UINT cstrHEADER = 0xFFFF;
const char* getHeaderStringA(const UINT type)
{
	static const char* PRT_HEADER[] =
	{
		"-=======================================================-",
		"==========          BIOS information           ==========",
		"==========         System information          ==========",
		"==========       Base Board information        ==========",
		"==========    System Enclosure information     ==========",
		"==========        Processor information        ==========",
		"==========    Memory Controller information    ==========",
		"==========      Memory Module information      ==========",
		"==========           Cache information         ==========",
		"==========      Port Connector Information     ==========",
		"==========            System Slots             ==========",
		"==========     On Board Devices Information    ==========",
		"==========             OEM String              ==========",
		"==========     System Configuration Options    ==========",
		"==========      BIOS Language Information      ==========",
		"==========         Group Associations          ==========",
		"==========          System Event Log           ==========",
		"==========        Physical Memory Array        ==========",
		"==========            Memory Device            ==========",
		"==========      Memory Error Information       ==========",
		"==========     Memory Array Mapped Address     ==========",
	};

	if (cstrHEADER == type)
		return PRT_HEADER[0];

	return PRT_HEADER[type + 1];
}

const wchar_t* getHeaderStringW(const UINT type)
{
	static wchar_t buff[2048];
	const char* pStr = getHeaderStringA(type);
	SecureZeroMemory(buff, sizeof(buff));
	MultiByteToWideChar(CP_OEMCP, 0, pStr, (int) strlen(pStr), buff, sizeof(buff));
	return buff;
}

const char* LocateStringA(const char* str, UINT i)
{
	static const char strNull[] = "Null String";
	
	if (0 == i || 0 == *str) return strNull;

	while (--i)
	{
		str += strlen((char*)str) + 1;
	}
	return str;
}

const wchar_t* LocateStringW(const char* str, UINT i)
{
	static wchar_t buff[2048];
	const char *pStr = LocateStringA(str, i);
	SecureZeroMemory(buff, sizeof(buff));
	MultiByteToWideChar(CP_OEMCP, 0, pStr, (int) strlen(pStr), buff, sizeof(buff));
	return buff;
}

const char* toPointString(void* p)
{
	return (char*)p + ((PSMBIOSHEADER)p)->Length;
}

bool ProcBIOSInfo(void* p)
{
	PBIOSInfo pBIOS = (PBIOSInfo)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(0));
	_tprintf(TEXT("Vendor: %s\n"), LocateString(str, pBIOS->Vendor));
	_tprintf(TEXT("Version: %s\n"), LocateString(str, pBIOS->Version));
	_tprintf(TEXT("BIOS Starting Segment: 0x%X\n"), pBIOS->StartingAddrSeg);
	_tprintf(TEXT("Release Date: %s\n"), LocateString(str, pBIOS->ReleaseDate));
	_tprintf(TEXT("Image Size: %dK\n"), (pBIOS->ROMSize + 1) * 64);
	if (pBIOS->Header.Length > 0x14)
	{   // for spec v2.4 and later
		_tprintf(TEXT("System BIOS version: %d.%d\n"), pBIOS->MajorRelease, pBIOS->MinorRelease);
		_tprintf(TEXT("EC Firmware version: %d.%d\n"), pBIOS->ECFirmwareMajor, pBIOS->ECFirmwareMinor);
	}
	return true;
}

bool ProcSysInfo(void* p)
{
	PSystemInfo pSystem = (PSystemInfo)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(1));
	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pSystem->Manufacturer));
	_tprintf(TEXT("Product Name: %s\n"), LocateString(str, pSystem->ProductName));
	_tprintf(TEXT("Version: %s\n"), LocateString(str, pSystem->Version));
	_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pSystem->SN));
	// for v2.1 and later
	if (pSystem->Header.Length > 0x08)
	{
		_tprintf(TEXT("UUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n"),
			pSystem->UUID[0], pSystem->UUID[1], pSystem->UUID[2], pSystem->UUID[3],
			pSystem->UUID[4], pSystem->UUID[5], pSystem->UUID[6], pSystem->UUID[7],
			pSystem->UUID[8], pSystem->UUID[9], pSystem->UUID[10], pSystem->UUID[11],
			pSystem->UUID[12], pSystem->UUID[13], pSystem->UUID[14], pSystem->UUID[15]);
	}

	if (pSystem->Header.Length > 0x19)
	{
		// fileds for spec. 2.4
		_tprintf(TEXT("SKU Number: %s\n"), LocateString(str, pSystem->SKUNumber));
		_tprintf(TEXT("Family: %s\n"), LocateString(str, pSystem->Family));
	}
	return true;
}

bool ProcBoardInfo(void* p)
{
	PBoardInfo pBoard = (PBoardInfo)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(2));
	_tprintf(TEXT("Length: 0x%X\n"), pBoard->Header.Length);
	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pBoard->Manufacturer));
	_tprintf(TEXT("Product Name: %s\n"), LocateString(str, pBoard->Product));
	_tprintf(TEXT("Version: %s\n"), LocateString(str, pBoard->Version));
	_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pBoard->SN));
	_tprintf(TEXT("Asset Tag Number: %s\n"), LocateString(str, pBoard->AssetTag));
	if (pBoard->Header.Length > 0x08)
	{
		_tprintf(TEXT("Location in Chassis: %s\n"), LocateString(str, pBoard->LocationInChassis));
	}

	return true;
}

bool ProcSystemEnclosure(void* p)
{
	PSystemEnclosure pSysEnclosure = (PSystemEnclosure)p;
	const char *str = toPointString(p);
	_tprintf(TEXT("%s\n"), getHeaderString(3));
	_tprintf(TEXT("Length: 0x%X\n"), pSysEnclosure->Header.Length);
	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pSysEnclosure->Manufacturer));
	_tprintf(TEXT("Version: %s\n"), LocateString(str, pSysEnclosure->Version));
	_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pSysEnclosure->SN));
	_tprintf(TEXT("Asset Tag Number: %s\n"), LocateString(str, pSysEnclosure->AssetTag));

	return true;
}

bool ProcProcessorInfo(void* p)
{
	PProcessorInfo	pProcessor = (PProcessorInfo)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(4));
	_tprintf(TEXT("Length: 0x%X\n"), pProcessor->Header.Length);
	_tprintf(TEXT("Socket Designation: %s\n"), LocateString(str, pProcessor->SocketDesignation));
	_tprintf(TEXT("Processor Manufacturer: %s\n"), LocateString(str, pProcessor->Manufacturer));
	_tprintf(TEXT("Processor Version: %s\n"), LocateString(str, pProcessor->Version));
	_tprintf(TEXT("External Clock: %dMHz, 0MHz is unknown clock\n"), pProcessor->ExtClock);
	_tprintf(TEXT("Max Speed: %dMHz\n"), pProcessor->MaxSpeed);
	_tprintf(TEXT("Current Speed: %dMHz\n"), pProcessor->CurrentSpeed);
	return true;
}

bool ProcMemModuleInfo(void* p)
{
	PMemModuleInfo	pMemModule = (PMemModuleInfo)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(6));
	_tprintf(TEXT("Length: 0x%X\n"), pMemModule->Header.Length);
	_tprintf(TEXT("Socket Designation: %s\n"), LocateString(str, pMemModule->SocketDesignation));
	_tprintf(TEXT("Current Speed: %dns\n"), pMemModule->CurrentSpeed);

	return true;
}

bool ProcCacheInfo(void *p)
{
	PCacheInfo	pCache = (PCacheInfo)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(7));
	_tprintf(TEXT("Length: 0x%X\n"), pCache->Header.Length);
	_tprintf(TEXT("Socket Designation: %s\n"), LocateString(str, pCache->SocketDesignation));

	return true;
}

bool ProcOEMString(void* p)
{
	PSMBIOSHEADER pHdr = (PSMBIOSHEADER)p;
	const char *str = toPointString(p);
	_tprintf(TEXT("%s\n"), getHeaderString(11));
	int OemStringCount = *(((char*)p) + 4);
	for(int i = 1; i <= OemStringCount; i++) {
		_tprintf(TEXT("OEM String%d: %s\n"), LocateString(str, i));
	}
	
	return true;
}

bool ProcMemoryDevice(void* p)
{
	PMemoryDevice pMD = (PMemoryDevice)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(17));
	_tprintf(TEXT("Length: 0x%X\n"), pMD->Header.Length);
	_tprintf(TEXT("Total Width: %dbits\n"), pMD->TotalWidth);
	_tprintf(TEXT("Data Width: %dbits\n"), pMD->DataWidth);
	_tprintf(TEXT("Device Locator: %s\n"), LocateString(str, pMD->DeviceLocator));
	_tprintf(TEXT("Bank Locator: %s\n"), LocateString(str, pMD->BankLocator));
	if (pMD->Header.Length > 0x15)
	{
		_tprintf(TEXT("Speed: %d\n"), pMD->Speed);
		_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str,pMD->Manufacturer));
		_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pMD->SN));
		_tprintf(TEXT("Asset Tag Number: %s\n"), LocateString(str, pMD->AssetTag));
		_tprintf(TEXT("Part Number: %s\n"), LocateString(str, pMD->PN));
	}

	return true;
}

bool ProcMemoryArrayMappedAddress(void*	p)
{
	PMemoryArrayMappedAddress pMAMA = (PMemoryArrayMappedAddress)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(19));
	_tprintf(TEXT("Length: 0x%X\n"), pMAMA->Header.Length);
	_tprintf(TEXT("Starting Address: 0x%08X\n"), pMAMA->Starting);
	_tprintf(TEXT("Ending Address: 0x%08X\n"), pMAMA->Ending);
	_tprintf(TEXT("Memory Array Handle: 0x%X\n"), pMAMA->Handle);
	_tprintf(TEXT("Partition Width: 0x%X\n"), pMAMA->PartitionWidth);
	return true;
}

bool ProcPortableBattery(void* p)
{
	PPortableBattery pPB = (PPortableBattery)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("============= Portable Battery =============\n"));
	_tprintf(TEXT("Length: 0x%X\n"), pPB->Header.Length);
	_tprintf(TEXT("Location: %s\n"), LocateString(str, pPB->Location));
	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pPB->Manufacturer));
	_tprintf(TEXT("Manufacturer Date: %s\n"), LocateString(str, pPB->Date));
	_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pPB->SN));

	return true;
}


bool DispatchStructType(PSMBIOSHEADER hdr)
{
	typedef struct {
		BYTE t;
		bool(*Proc)(void* p);
	} TPFUNC;

	const TPFUNC	tpfunc[] = {
		{ 0, ProcBIOSInfo },
		{ 1, ProcSysInfo },
		{ 2, ProcBoardInfo },
		{ 3, ProcSystemEnclosure },
		{ 4, ProcProcessorInfo },
		{ 6, ProcMemModuleInfo },
		{ 7, ProcCacheInfo },
		{ 11, ProcOEMString },
		{ 17, ProcMemoryDevice },
		{ 19, ProcMemoryArrayMappedAddress },
		{ 22, ProcPortableBattery },

	};

	for (UINT i = 0; i < sizeof(tpfunc) / sizeof(TPFUNC); i++)
	{
		if (tpfunc[i].t == hdr->Type)
		{
			_tprintf(TEXT("%s\n"), getHeaderString(cstrHEADER));
			tpfunc[i].Proc((void*)hdr);
			return true;
		}
	}

	return false;
}

void DumpSMBIOSStruct(void *Addr, UINT Len)
{
	LPBYTE p = (LPBYTE)(Addr);
	const LPBYTE lastAddress = p + Len;
	PSMBIOSHEADER pHeader;

	for (;;) {
		pHeader = (PSMBIOSHEADER)p;
		DispatchStructType(pHeader);
		if ((pHeader->Type == 127) && (pHeader->Length == 4))
			break; // last avaiable tables
		LPBYTE nt = p + pHeader->Length; // point to struct end
		while (0 != (*nt | *(nt + 1))) nt++; // skip string area
		nt += 2;
		if (nt >= lastAddress)
			break;
		p = nt;
	}
}


#include "smbios.h"

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD needBufferSize = 0;
	// the seqence just for x86, but don't worry we know SMBIOS/DMI only exist on x86 platform
	// const BYTE byteSignature[] = { 'B', 'M', 'S', 'R' };
	const DWORD Signature = 'RSMB';
	LPBYTE pBuff = NULL;

	needBufferSize = GetSystemFirmwareTable(Signature, 0, NULL, 0);

	_tprintf(TEXT("We need prepare %d bytes for recevie SMBIOS/DMI Table\n"), needBufferSize);
	pBuff = (LPBYTE) malloc(needBufferSize);
	if (pBuff)
	{
		GetSystemFirmwareTable(Signature, 0, pBuff, needBufferSize);

		const PRawSMBIOSData pDMIData = (PRawSMBIOSData)pBuff;
		_tprintf(TEXT("SMBIOS version:%d.%d\n"), pDMIData->SMBIOSMajorVersion, pDMIData->SMBIOSMinorVersion);
		_tprintf(TEXT("DMI Revision:%x\n"), pDMIData->DmiRevision);
		_tprintf(TEXT("Total length: %d\n"), pDMIData->Length);
		_tprintf(TEXT("DMI at address %p\n"), &pDMIData->SMBIOSTableData);
		DumpSMBIOSStruct(&(pDMIData->SMBIOSTableData), pDMIData->Length);
	}
	else
		_tprintf(TEXT("Can not allocate memory for recevice SMBIOS/DMI table\n"));

	if (pBuff)
		free(pBuff);

	/* for SMBIOS singleton verify*/
	wprintf(L"\r\n///////////////////////////////////////////////////////////\r\n");
	const SMBIOS &SmBios = SMBIOS::getInstance();
	wprintf(L"BIOS Vendor: %s\r\n", SmBios.BIOSVendor());
	wprintf(L"BIOS Version: %s\r\n", SmBios.BIOSVersion());
	wprintf(L"BIOS Release Data: %s\r\n", SmBios.BIOSReleaseDate());
	wprintf(L"Board Manaufactor: %s\r\n", SmBios.BoardManufactor());
	wprintf(L"Board Product Name: %s\r\n", SmBios.BoardProductName());
	wprintf(L"System Manufactor: %s\r\n", SmBios.SysManufactor());
	wprintf(L"System Product Name: %s\r\n", SmBios.SysProductName());
	return 0;
}

