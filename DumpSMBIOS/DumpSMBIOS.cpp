// DumpSMBIOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma pack(push)
#pragma pack(1)

#define WAKEUP_TYPE_COUNT   9
#define BOARD_TYPE_COUNT    13
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
	UCHAR   Status;
	UCHAR   ProcessorUpgrade;
	UINT16  L1CacheHandle;
	UINT16  L2CacheHandle;
	UINT16  L3CacheHandle;
	UCHAR   SerialNumber;
	UCHAR   AssertTag;
	UCHAR   PartNumber;
	UCHAR   CoreCount;
	UCHAR   CoreEnabled;
	UCHAR   ThreadCount;
	UINT16  ProcessorCharacteristics;
	UINT16  ProcessorFamily2;

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

typedef struct _TYPE_11_ {
	SMBIOSHEADER Header;
	UCHAR	Count;
} OemString, *POemString;

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

typedef struct _TYPE_21_ {
	SMBIOSHEADER Header;
	UCHAR Type;
	UCHAR Interface;
	UCHAR NumOfButton;
} BuiltinPointDevice, *PBuiltinPointDevice;

typedef struct _TYPE_22_ {
	SMBIOSHEADER Header;
	UCHAR	Location;
	UCHAR	Manufacturer;
	UCHAR	Date;
	UCHAR	SN;
	UCHAR	DeviceName;
	UCHAR   Chemistry;
	UINT16  DesignCapacity;
	UINT16  DesignVoltage;
	UCHAR   SBDSVersionNumber;
	UCHAR   MaximumErrorInBatteryData;
	UINT16  SBDSSerialNumber;
	UINT16	SBDSManufactureDate;
	UCHAR   SBDSDeviceChemistry;
	UCHAR   DesignCapacityMultiplie;
	UINT32  OEM;
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
		"-=======================================================-", /*   0  */
		"==========          BIOS information           ==========", /*   1  */
		"==========         System information          ==========", /*   2  */
		"==========       Base Board information        ==========", /*   3  */
		"==========    System Enclosure information     ==========", /*   4  */
		"==========        Processor information        ==========", /*   5  */
		"==========    Memory Controller information    ==========", /*   6  */
		"==========      Memory Module information      ==========", /*   7  */
		"==========           Cache information         ==========", /*   8  */
		"==========      Port Connector Information     ==========", /*   9  */
		"==========            System Slots             ==========", /*  10  */
		"==========     On Board Devices Information    ==========", /*  11  */
		"==========             OEM String              ==========", /*  12  */
		"==========     System Configuration Options    ==========", /*  13  */
		"==========      BIOS Language Information      ==========", /*  14  */
		"==========         Group Associations          ==========", /*  15  */
		"==========          System Event Log           ==========", /*  16  */
		"==========        Physical Memory Array        ==========", /*  17  */
		"==========            Memory Device            ==========", /*  18  */
		"==========      Memory Error Information       ==========", /*  19  */
		"==========     Memory Array Mapped Address     ==========", /*  20  */
		"==========    Memory Device Mapped Address     ==========", /*  21  */
		"==========       Built-in Pointing Device      ==========", /*  22  */
		"==========          Portable Battery           ==========", /*  23  */
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
	const int convSize = MultiByteToWideChar(CP_OEMCP, 0, pStr, (int) strlen(pStr), buff, sizeof(buff) / 2);
	if (convSize > 0)
		return buff;
	return NULL;
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
	const int convSize = MultiByteToWideChar(CP_OEMCP, 0, pStr, (int) strlen(pStr), buff, sizeof(buff) / 2);
	if (convSize > 0)
		return buff;
	return NULL;
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

	// process wake-up type, for spec2.1 or later
	if (pSystem->WakeUpType >= 0 && pSystem->WakeUpType < WAKEUP_TYPE_COUNT) 
	{	
		const TCHAR* WakeupTypeStrings[] =
		{
			TEXT("Reserved"), TEXT("Other"), TEXT("Unknown"), TEXT("APM Timer"), TEXT("Modem Ring"),
			TEXT("LAN Remote"),TEXT("Power Switch") , TEXT("PCI PME#"), TEXT("AC Power Restored")
		};
		_tprintf(TEXT("Wake-up Type: %s\n"), WakeupTypeStrings[pSystem->WakeUpType]);
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
	const char HandleCount = pBoard->NumObjHandle;

	_tprintf(TEXT("%s\n"), getHeaderString(2));
	_tprintf(TEXT("Length: 0x%X\n"), pBoard->Header.Length);
	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pBoard->Manufacturer));
	_tprintf(TEXT("Product Name: %s\n"), LocateString(str, pBoard->Product));
	_tprintf(TEXT("Version: %s\n"), LocateString(str, pBoard->Version));
	_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pBoard->SN));
	_tprintf(TEXT("Asset Tag Number: %s\n"), LocateString(str, pBoard->AssetTag));
	_tprintf(TEXT("Feature Flag: 0x%x\n"), pBoard->FeatureFlags);

	if (pBoard->Header.Length > 0x08)
	{
		_tprintf(TEXT("Location in Chassis: %s\n"), LocateString(str, pBoard->LocationInChassis));
	}

	if (pBoard->Header.Length > 0x0B)
	{
		const TCHAR* BoardType[] =
		{
			TEXT("Unknown"), TEXT("Other"), TEXT("Server Blade"), TEXT("Connectivity Switch"), TEXT("System Management Module"),
			TEXT("Processor Module"), TEXT("I/O Module"), TEXT("Memory Module"), TEXT("Daughter board"), TEXT("Motherboard (includes processor, memory, and I/O)"),
			TEXT("Processor/Memory Module"), TEXT("Processor/IO Module"), TEXT("Interconnect board"),
		};
		if (pBoard->Type >= 0 && pBoard->Type < BOARD_TYPE_COUNT) 
		{
			_tprintf(TEXT("Board Type: %s\n"), BoardType[pBoard->Type]);
		}
	}

	if (pBoard->Header.Length > 0x0D) 
	{
		_tprintf(TEXT("Number of Contained Object Handles: %d\n"), HandleCount);
	}
	_tprintf(TEXT("Object Handles: \n"));

	for (int i = 0; i < HandleCount; i++) 
	{
		_tprintf(TEXT("%02x "), pBoard->pObjHandle[i]);
	}
	_tprintf(TEXT("\n"));

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
	const TCHAR *ProcessTypeStrings[] = 
	{
		TEXT("Other"), TEXT("Unknown"), TEXT("Central Processor"),
		TEXT("Math Processor"), TEXT("DSP Processor"), TEXT("Video Processor"),
	};

	_tprintf(TEXT("%s\n"), getHeaderString(4));
	_tprintf(TEXT("Length: 0x%X\n"), pProcessor->Header.Length);
	_tprintf(TEXT("Socket Designation: %s\n"), LocateString(str, pProcessor->SocketDesignation));
	_tprintf(TEXT("Processor Type: %s\n"), ProcessTypeStrings[pProcessor->Type]);
	_tprintf(TEXT("Processor Family: \n"));
	_tprintf(TEXT("Processor ID: \n"));
	_tprintf(TEXT("Processor Manufacturer: %s\n"), LocateString(str, pProcessor->Manufacturer));
	_tprintf(TEXT("Processor Version: %s\n"), LocateString(str, pProcessor->Version));
	_tprintf(TEXT("External Clock: %dMHz, 0MHz is unknown clock\n"), pProcessor->ExtClock);
	_tprintf(TEXT("Max Speed: %dMHz\n"), pProcessor->MaxSpeed);
	_tprintf(TEXT("Current Speed: %dMHz\n"), pProcessor->CurrentSpeed);
	_tprintf(TEXT("Status: 0x%02x\n"), pProcessor->Status);
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
	POemString pOemString = (POemString)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(11));
	_tprintf(TEXT("Count: %d\n"), pOemString->Count);
	for(int i = 1; i <= pOemString->Count; i++)
	{
		_tprintf(TEXT("OEM String%d: %s\n"), i, LocateString(str, i));
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

static const TCHAR* getBuiltinPointDeviceTypeString(const UCHAR type)
{
	const TCHAR* typeString[10] = {
		TEXT("Unsupport Type"),
		TEXT("Other"),
		TEXT("Unknown"),
		TEXT("Mouse"),
		TEXT("Track Ball"),
		TEXT("Track Point"),
		TEXT("Glide Point"),
		TEXT("Touch Pad"),
		TEXT("Touch Screen"),
		TEXT("Optical Sensor"),
	};

	if ((type >= 1) && (type <= 9))
		return typeString[type];

	return typeString[0];
}

static const TCHAR* getBuiltinPointDeviceInterfaceString(const UCHAR Interface)
{
	const TCHAR* interfaceString[12] = {
		TEXT("Unsupport Interface"),
		TEXT("Other"),
		TEXT("Unknown"),
		TEXT("Serial"),
		TEXT("PS/2"),
		TEXT("Infrared"),
		TEXT("HP-HIL"),
		TEXT("Bus mouse"),
		TEXT("Apple Desktop Bus"),
		TEXT("Bus mouse DB-9"),
		TEXT("Bus mouse micro-DIN"),
		TEXT("USB"),
	};

	if ((Interface >= 1) && Interface <= 8)
		return interfaceString[Interface];
	else if ((Interface >= 0xA0) && (Interface <= 0xA2))
		return interfaceString[Interface - 0xA0 + 9];

	return interfaceString[0];
}

bool ProcBuiltinPointDevice(void* p)
{
	PBuiltinPointDevice pBPD = (PBuiltinPointDevice)p;

	_tprintf(TEXT("%s\n"), getHeaderString(21));
	_tprintf(TEXT("Length: 0x%X\n"), pBPD->Header.Length);
	_tprintf(TEXT("Type: %s(0x%X)\n"),
		getBuiltinPointDeviceTypeString(pBPD->Type), pBPD->Type);
	_tprintf(TEXT("Interface: %s(0x%X)\n"),
		getBuiltinPointDeviceInterfaceString(pBPD->Interface), pBPD->Interface);
	_tprintf(TEXT("Number of Button: %d\n"), pBPD->NumOfButton);
	return true;
}

static const TCHAR* getBatteryChemistry(const UCHAR chemistry)
{
	const TCHAR* typeString[9] = {
		TEXT("Unsupport type"),
		TEXT("Other"),
		TEXT("Unknown"),
		TEXT("Lead Acid"),
		TEXT("Nickel Cadmium"),
		TEXT("Nickel metal hydride"),
		TEXT("Lithium-ion"),
		TEXT("Zinc air"),
		TEXT("Lithium Polyme")
	};

	if ((chemistry >= 1) && (chemistry <= 8))
	{
		return typeString[chemistry];
	}

	return typeString[0];
}

bool ProcPortableBattery(void* p)
{
	PPortableBattery pPB = (PPortableBattery)p;
	const char *str = toPointString(p);

	_tprintf(TEXT("%s\n"), getHeaderString(22));
	_tprintf(TEXT("Length: 0x%X\n"), pPB->Header.Length);
	_tprintf(TEXT("Location: %s\n"), LocateString(str, pPB->Location));
	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pPB->Manufacturer));

	if (pPB->Date != 0)
		_tprintf(TEXT("Manufacturer Date: %s\n"), LocateString(str, pPB->Date));
	else {
		// TODO:
	}

	if (pPB->SN != 0)
		_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pPB->SN));
	else {
		// TODO:
	}

	_tprintf(TEXT("Device Name: %s\n"), LocateString(str, pPB->DeviceName));

	if (pPB->Chemistry != 2) {
		_tprintf(TEXT("Chemistry: %s(0x%X)\n"), getBatteryChemistry(pPB->Chemistry), pPB->Chemistry);
	}
	else {
		_tprintf(TEXT("Chemistry: %s\n"), LocateString(str, pPB->SBDSDeviceChemistry));
	}
	if (pPB->DesignCapacity == 0) {
		_tprintf(TEXT("Design Capacity: Unkonw\n"));
	}
	else {
		_tprintf(TEXT("Design Capacity: %dmWH\n"), ((pPB->DesignCapacity * pPB->DesignCapacityMultiplie)));
	}
	_tprintf(TEXT("Design Voltage: %dmV\n"), pPB->DesignVoltage);
	_tprintf(TEXT("SBDS Version Number: %s\n"), LocateString(str, pPB->SBDSVersionNumber));
	_tprintf(TEXT("Maximum Error in Battery Data: %d\n"), pPB->MaximumErrorInBatteryData);
	_tprintf(TEXT("SBDS Serial Number: %d\n"), pPB->SBDSSerialNumber);
	_tprintf(TEXT("SBDS Manufacture Date: \n")); // to be done
	_tprintf(TEXT("SBDS Device Chemistry: %s\n"), LocateString(str, pPB->SBDSDeviceChemistry));
	_tprintf(TEXT("Design Capacity Multiplier: 0x%02x\n"), pPB->DesignCapacityMultiplie);
	_tprintf(TEXT("OEM-specify: \n")); // to be done

	
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
		{ 21, ProcBuiltinPointDevice },
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
