// DumpSMBIOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include ".\SysInfo\ISysInfo.h"
#include ".\SysInfo\ItemID.h"
#include "smheader.h"
#include "biosinfo.h"
#include "sysinfo.h"
#include "boardinfo.h"
#include "sysenclosure.h"

typedef ISysInfo* (*_CreateSysInfo) (DWORD);
typedef void (*_DestroySysInfo) (ISysInfo*);
typedef ULONG	(*_MemReadBlock) (ULONG address, UCHAR* data, ULONG count, ULONG unitsize);
_MemReadBlock pfMemReadBlock = NULL;

const UINT cstrHEADER = 0xFFFF;
const char* getHeaderString(const UINT type)
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

	return PRT_HEADER[type+1];
}

#pragma pack(push)
#pragma pack(1)

typedef struct _TYPE_4_ {
	SMBIOS_STRUCT_HEADER Header;
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
	SMBIOS_STRUCT_HEADER Header;
	// Todo, Here

} MemCtrlInfo, *PMemCtrlInfo;

typedef struct _TYPE_6_ {
	SMBIOS_STRUCT_HEADER Header;
	UCHAR	SocketDesignation;
	UCHAR	BankConnections;
	UCHAR	CurrentSpeed;
	// Todo, Here
} MemModuleInfo, *PMemModuleInfo;

typedef struct _TYPE_7_ {
	SMBIOS_STRUCT_HEADER Header;
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
	SMBIOS_STRUCT_HEADER Header;
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
	SMBIOS_STRUCT_HEADER Header;
	ULONG32	Starting;
	ULONG32	Ending;
	UINT16	Handle;
	UCHAR	PartitionWidth;
} MemoryArrayMappedAddress, *PMemoryArrayMappedAddress;

typedef struct _TYPE_22_ {
	SMBIOS_STRUCT_HEADER Header;
	UCHAR	Location;
	UCHAR	Manufacturer;
	UCHAR	Date;
	UCHAR	SN;
	UCHAR	DeviceName;

} PortableBattery, *PPortableBattery;

#pragma pack(pop)

UCHAR* toString(const UCHAR* src, int len)
{
	static UCHAR buff[256];
	memcpy(buff, src, len);
	buff[len] = 0;
	return buff;
}

void printSMBIOSEPS(SMBIOS_EPS& eps, ULONG addr)
{
	printf("SMBIOS Signature: %s, at 0x%X\n", toString(eps.Signature, 4), addr);
	printf("Checksum: 0x%X\n", eps.Chksum);
	printf("Length: %d(0x%X)\n", eps.Length, eps.Length);
	printf("Version: %d.%d\n", eps.VerMajor, eps.VerMinor);
	printf("Maximum Structure Size: %d(0x%X)\n", eps.MaxSize, eps.MaxSize);
	printf("Entry Point Structure Revision: %d\n", eps.EPSRevision);
	printf("Formatter Area: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n", 
		eps.Formatter[0],eps.Formatter[1],eps.Formatter[2],
		eps.Formatter[3],eps.Formatter[4]);
	printf("Intermediate anchor string: %s\n", toString(eps.DMISignature, 5));
	printf("Intermediate Checksum: 0x%X\n", eps.DMIChksum);
	printf("Structure Table Length: %d\n", eps.StructLength);
	printf("Structure Table Address: 0x%08X\n", eps.StructAddress);
	printf("Number of SMBIOS Structures: %d\n", eps.NumStruct);
	printf("BCD Revision: 0x%02X\n", eps.BCDRevision);
}

bool LoadSysInfo(HMODULE &hSysInfoLib, ISysInfo* pISysInfo)
{
	hSysInfoLib = LoadLibrary(_T("SysInfo.dll"));

	if (NULL != hSysInfoLib)
	{
		_CreateSysInfo pCreateSysInfo = (_CreateSysInfo)GetProcAddress(hSysInfoLib, "CreateSysInfo");

		if (NULL != pCreateSysInfo)
		{
			pISysInfo = pCreateSysInfo(MODE_PCI); // request access memory spaces, MODE_PCI
		}
		else
		{
			printf("ERR: Failed in GetProcAddress(\"CreateSysInfo\")\n");
			exit(-1);
			return false;
		}
	}
	else 
	{
		printf("Can't find Sysinfo.dll\n");
		exit(-1);
		return false;
	}
	return true;
}

bool UnloadSysInfo(HMODULE hSysInfoLib, ISysInfo* pISysInfo)
{
	_DestroySysInfo pDestroySysInfo = (_DestroySysInfo)GetProcAddress(hSysInfoLib, "DestroySysInfo");
	if (pDestroySysInfo)
	{
		pDestroySysInfo(pISysInfo);
		pISysInfo = NULL;
	}
	else
	{
		printf("ERR: Failed in GetProcAddress(\"DestroySysInfo\")\n");
		exit(-1);
		return false;
	}

	FreeLibrary(hSysInfoLib);
	hSysInfoLib = NULL;
	return true;
}


bool FindSMBIOS(PSMBIOS_EPS& eps, UCHAR* buff, UINT size)
{
	UCHAR *p = buff;

	for (UINT i = 0; i < size; i+=16)
	{
		if (0 == memcmp(p, "_SM_", 4))
		{
			PSMBIOS_EPS psm = (PSMBIOS_EPS)p;

			if (0 == memcmp(psm->DMISignature, "_DMI_",5))
			{
				UCHAR chk = 0;

				// verify checksum
				for(UINT i=0; i < psm->Length; i++)
				{
					chk+=*(p+i);
				}

				if (0 == chk)
				{
					eps = psm;
					return true;
				}
			}
		}
		p+=16;
	}
	return false;
}

const UCHAR* LocateString(UCHAR* str, UINT i)
{
	static const UCHAR strNull[] = "Null String";
	
	if (0 == i || 0 == *str) return strNull;

	while(--i)
	{
		str += strlen((char*)str) + 1;
	}
	return str;
}

UCHAR* toPointString(void* p)
{
	return (UCHAR*)p+((PSMBIOS_STRUCT_HEADER)p)->Length;
}

bool ProcBIOSInfo(void* p)
{
	PBIOSInfo pBIOS = (PBIOSInfo)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(0));
	printf("Vendor: %s\n", LocateString(str, pBIOS->Vendor));
	printf("Version: %s\n", LocateString(str, pBIOS->Version));
	printf("BIOS Starting Segment: 0x%X\n", pBIOS->StartingAddrSeg);
	printf("Release Date: %s\n", LocateString(str, pBIOS->ReleaseDate));
	printf("Image Size: %dK\n", (pBIOS->ROMSize+1)*64);
	if (pBIOS->Header.Length > 0x14)
	{   // for spec v2.4 and later
		printf("System BIOS version: %d.%d\n", pBIOS->MajorRelease, pBIOS->MinorRelease);
		printf("EC Firmware version: %d.%d\n", pBIOS->ECFirmwareMajor, pBIOS->ECFirmwareMinor);
	}
	return true;
}

bool ProcSysInfo(void* p)
{
	PSystemInfo pSystem = (PSystemInfo)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(1));
	printf("Manufacturer: %s\n", LocateString(str, pSystem->Manufacturer));
	printf("Product Name: %s\n", LocateString(str, pSystem->ProductName));
	printf("Version: %s\n", LocateString(str, pSystem->Version));
	printf("Serial Number: %s\n", LocateString(str, pSystem->SN));
	// for v2.1 and later
	if (pSystem->Header.Length > 0x08)
	{
		printf("UUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
		pSystem->UUID[3], pSystem->UUID[2], pSystem->UUID[1], pSystem->UUID[0],
		pSystem->UUID[5], pSystem->UUID[4], pSystem->UUID[7], pSystem->UUID[6],
		pSystem->UUID[8], pSystem->UUID[9], pSystem->UUID[10], pSystem->UUID[11],
		pSystem->UUID[12], pSystem->UUID[13], pSystem->UUID[14], pSystem->UUID[15]);
	}

	if (pSystem->Header.Length > 0x19)
	{
		// fileds for spec. 2.4
		printf("SKU Number: %s\n", LocateString(str, pSystem->SKUNumber));
		printf("Family: %s\n", LocateString(str, pSystem->Family));
	}
	return true;
}

bool ProcBoardInfo(void* p)
{
	PBoardInfo pBoard = (PBoardInfo)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(2));
	printf("Length: 0x%X\n", pBoard->Header.Length);
	printf("Manufacturer: %s\n", LocateString(str, pBoard->Manufacturer));
	printf("Product Name: %s\n", LocateString(str, pBoard->Product));
	printf("Version: %s\n", LocateString(str, pBoard->Version));
	printf("Serial Number: %s\n", LocateString(str, pBoard->SN));
	printf("Asset Tag Number: %s\n", LocateString(str, pBoard->AssetTag));
	if (pBoard->Header.Length > 0x08)
	{
		printf("Location in Chassis: %s\n", LocateString(str, pBoard->LocationInChassis));
	}

	return true;
}

bool ProcSystemEnclosure(void* p)
{
	PSystemEnclosure pSysEnclosure = (PSystemEnclosure)p;
	UCHAR *str = toPointString(p);
	printf("%s\n", getHeaderString(3));
	printf("Length: 0x%X\n", pSysEnclosure->Header.Length);
	printf("Manufacturer: %s\n", LocateString(str, pSysEnclosure->Manufacturer));
	printf("Version: %s\n", LocateString(str, pSysEnclosure->Version));
	printf("Serial Number: %s\n", LocateString(str, pSysEnclosure->SN));
	printf("Asset Tag Number: %s\n", LocateString(str, pSysEnclosure->AssetTag));

	return true;
}

bool ProcProcessorInfo(void* p)
{
	PProcessorInfo	pProcessor = (PProcessorInfo)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(4));
	printf("Length: 0x%X\n", pProcessor->Header.Length);
	printf("Socket Designation: %s\n", LocateString(str, pProcessor->SocketDesignation));
	printf("Processor Manufacturer: %s\n", LocateString(str, pProcessor->Manufacturer));
	printf("Processor Version: %s\n", LocateString(str, pProcessor->Version));
	printf("External Clock: %dMHz, 0MHz is unknown clock\n", pProcessor->ExtClock);
	printf("Max Speed: %dMHz\n", pProcessor->MaxSpeed);
	printf("Current Speed: %dMHz\n", pProcessor->CurrentSpeed);
	return true;
}

bool ProcMemModuleInfo(void* p)
{
	PMemModuleInfo	pMemModule = (PMemModuleInfo)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(6));
	printf("Length: 0x%X\n", pMemModule->Header.Length);
	printf("Socket Designation: %s\n", LocateString(str, pMemModule->SocketDesignation));
	printf("Current Speed: %dns\n", pMemModule->CurrentSpeed);

	return true;
}

bool ProcCacheInfo(void *p)
{
	PCacheInfo	pCache = (PCacheInfo)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(7));
	printf("Length: 0x%X\n", pCache->Header.Length);
	printf("Socket Designation: %s\n", LocateString(str, pCache->SocketDesignation));

	return true;
}

bool ProcOEMString(void* p)
{
	PSMBIOS_STRUCT_HEADER pHdr = (PSMBIOS_STRUCT_HEADER)p;
	UCHAR *str = toPointString(p);
	printf("%s\n", getHeaderString(11));
	printf("OEM String: %s\n", LocateString(str,*(((UCHAR*)p)+4) ));

	return true;
}

bool ProcMemoryDevice(void* p)
{
	PMemoryDevice pMD = (PMemoryDevice)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(17));
	printf("Length: 0x%X\n", pMD->Header.Length);
	printf("Total Width: %dbits\n", pMD->TotalWidth);
	printf("Data Width: %dbits\n", pMD->DataWidth);
	printf("Device Locator: %s\n", LocateString(str, pMD->DeviceLocator));
	printf("Bank Locator: %s\n", LocateString(str, pMD->BankLocator));
	if (pMD->Header.Length > 0x15)
	{
		printf("Speed: %d\n", pMD->Speed);
		printf("Manufacturer: %s\n", (char *)pMD->Manufacturer);
		printf("Serial Number: %s\n", LocateString(str, pMD->SN));
		printf("Asset Tag Number: %s\n", LocateString(str, pMD->AssetTag));
		printf("Part Number: %s\n", LocateString(str, pMD->PN));
	}

	return true;
}

bool ProcMemoryArrayMappedAddress(void*	p)
{
	PMemoryArrayMappedAddress pMAMA = (PMemoryArrayMappedAddress)p;
	UCHAR *str = toPointString(p);

	printf("%s\n", getHeaderString(19));
	printf("Length: 0x%X\n", pMAMA->Header.Length);
	printf("Starting Address: 0x%08X\n", pMAMA->Starting);
	printf("Ending Address: 0x%08X\n", pMAMA->Ending);
	printf("Memory Array Handle: 0x%X\n", pMAMA->Handle);
	printf("Partition Width: 0x%X\n", pMAMA->PartitionWidth);
	return true;
}

bool ProcPortableBattery(void* p)
{
	PPortableBattery pPB = (PPortableBattery)p;
	UCHAR *str = toPointString(p);

	printf("============= Portable Battery =============\n");
	printf("Length: 0x%X\n", pPB->Header.Length);
	printf("Location: %s\n", LocateString(str, pPB->Location));
	printf("Manufacturer: %s\n", LocateString(str, pPB->Manufacturer));
	printf("Manufacturer Date: %s\n", LocateString(str, pPB->Date));
	printf("Serial Number: %s\n", LocateString(str, pPB->SN));

	return true;
}

bool DispatchStructType(PSMBIOS_STRUCT_HEADER hdr)
{
	typedef struct {
		UCHAR	t;
		bool (*Proc)(void* p);
	} TPFUNC;

	const TPFUNC	tpfunc[] = {
		{	0, ProcBIOSInfo		},
		{   1, ProcSysInfo		},
		{   2, ProcBoardInfo	},
		{	3, ProcSystemEnclosure	},
		{	4, ProcProcessorInfo },
		{	6, ProcMemModuleInfo },
		{	7, ProcCacheInfo	},
		{	11, ProcOEMString	},
		{	17, ProcMemoryDevice	},
		{	19, ProcMemoryArrayMappedAddress },
		{	22, ProcPortableBattery	},

	};

	for (UINT i = 0; i < sizeof(tpfunc)/sizeof(TPFUNC); i++)
	{
		if (tpfunc[i].t == hdr->Type)
		{
			printf("%s\n", getHeaderString(cstrHEADER));
			tpfunc[i].Proc((void*)hdr);
			return true;
		}
	}
	
	return false;
}

void DumpSMBIOSStruct(ULONG Addr,UINT Len, UINT Num)
{	
	UCHAR *pData;
	UCHAR *p = NULL;
		
	pData = (UCHAR*)malloc(Len);
	if (NULL != pData)
	{
			pfMemReadBlock(Addr, pData, Len, 1);
			p = pData;
	}
		
	PSMBIOS_STRUCT_HEADER pHeader;
	for (UINT n = 0; n < Num; n++) {
		pHeader = (PSMBIOS_STRUCT_HEADER)p;

		DispatchStructType(pHeader);

		UCHAR *nt = p + pHeader->Length; // point to struct end
		
		while (0 != (*nt | *(nt+1))) nt++;
		nt+=2;
		if (nt > pData+Len) break;
		p = nt;
	}

	free(pData);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ISysInfo* pISysInfo = NULL;
	HMODULE hSysInfoLib = NULL;
	const ULONG MEM_RANGE = 64*1024; // 0xF0000 ~ 0xFFFFF, 64K
	const ULONG MEM_START = 0xF0000;
	UCHAR buff[MEM_RANGE];
	PSMBIOS_EPS pSMBIOS = NULL;

	if (false == LoadSysInfo(hSysInfoLib, pISysInfo))
		return -1;

	pfMemReadBlock = (_MemReadBlock) GetProcAddress(hSysInfoLib, "_MemReadBlock");
	if (pfMemReadBlock)
	{
		pfMemReadBlock(MEM_START, buff, MEM_RANGE, 1);
		FindSMBIOS(pSMBIOS, buff, MEM_RANGE);
	}
	else
	{
		printf("ERR: failed in GetProcAddress(\"_MemReadBlock\")\n");
	}

	if (pSMBIOS)
	{
		printf("Find out the SMBIOS Entry Point Structure \n");
		printSMBIOSEPS(*pSMBIOS, ((UCHAR*)pSMBIOS - buff) + 0xF0000);
		DumpSMBIOSStruct(pSMBIOS->StructAddress, pSMBIOS->StructLength, pSMBIOS->NumStruct);
	}
	else
	{
		printf("Could not find SMBIOS Entry Point Structure \n");
	}
 	UnloadSysInfo(hSysInfoLib, pISysInfo);
	return 0;
}
