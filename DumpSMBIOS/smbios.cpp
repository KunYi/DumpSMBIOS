#include "stdafx.h"
#include "smbios.h"

#include <wbemidl.h>
#include <sstream>
#include <comdef.h>
#pragma comment(lib, "Wbemuuid.lib")

typedef UINT(WINAPI *GET_SYSTEM_FIRMWARE_TABLE) (DWORD, DWORD, PVOID, DWORD);

#pragma pack(push)
#pragma pack(1)
typedef struct _RawSMBIOSData
{
	BYTE	Used20CallingMethod;
	BYTE	MajorVersion;
	BYTE	MinorVersion;
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

#pragma pack(pop)

static const char* LocateStringA(const char* str, UINT i)
{
	static const char strNull[] = "";

	if (0 == i || 0 == *str) return strNull;

	while (--i)
	{
		str += strlen((char*)str) + 1;
	}
	return str;
}

static const char* toPointString(void* p)
{
	return (char*)p + ((PSMBIOSHEADER)p)->Length;
}

static PWCHAR ConvertToWideChar(const char* str)
{
	if (!str || !*str) return NULL;

	const int len = (int)strlen(str);
	PWCHAR result = new WCHAR[len + 1];

	if (result) {
		::SecureZeroMemory(result, sizeof(WCHAR) * (len + 1));
		::MultiByteToWideChar(CP_ACP, 0, str, len, result, len + 1);
	}
	return result;
}


bool SMBIOS::ProcBIOSInfo(SMBIOS* T, void* p)
{
	PBIOSInfo pBIOS = (PBIOSInfo)p;
	const char* str = toPointString(p);
	T->m_wszBIOSVendor = ConvertToWideChar(LocateStringA(str, pBIOS->Vendor));
	T->m_wszBIOSVersion = ConvertToWideChar(LocateStringA(str, pBIOS->Version));
	T->m_wszBIOSReleaseDate = ConvertToWideChar(LocateStringA(str, pBIOS->ReleaseDate));

	if (pBIOS->Header.Length > 0x14)
	{
		T->m_BIOSSysVersion = pBIOS->MajorRelease << 16 | pBIOS->MinorRelease;
		T->m_BIOSECVersion = pBIOS->ECFirmwareMajor << 16 | pBIOS->ECFirmwareMinor;
	}
	return true;
}

bool SMBIOS::ProcSysInfo(SMBIOS* T, void* p)
{
	PSystemInfo pSystem = (PSystemInfo)p;
	const char* str = toPointString(p);
	T->m_wszSysManufactor = ConvertToWideChar(LocateStringA(str, pSystem->Manufacturer));
	T->m_wszSysProductName = ConvertToWideChar(LocateStringA(str, pSystem->ProductName));
	T->m_wszSysVersion = ConvertToWideChar(LocateStringA(str, pSystem->Version));
	T->m_wszSysSerialNumber = ConvertToWideChar(LocateStringA(str, pSystem->SN));

	if (pSystem->Header.Length > 0x08)
	{
		memcpy_s(&(T->m_SysUUID), sizeof(UUID), pSystem->UUID, 16);
	}
	if (pSystem->Header.Length > 0x19)
	{
		T->m_wszSysSKU = ConvertToWideChar(LocateStringA(str, pSystem->SKUNumber));
		T->m_wszSysFamily = ConvertToWideChar(LocateStringA(str, pSystem->Family));
	}
	return true;
}

bool SMBIOS::ProcBoardInfo(SMBIOS* T, void* p)
{
	PBoardInfo pBoard = (PBoardInfo)p;
	const char* str = toPointString(p);
	T->m_wszBoardManufactor = ConvertToWideChar(LocateStringA(str, pBoard->Manufacturer));
	T->m_wszBoardProductName = ConvertToWideChar(LocateStringA(str, pBoard->Product));
	T->m_wszBoardVersion = ConvertToWideChar(LocateStringA(str, pBoard->Version));
	T->m_wszBoardSerialNumber = ConvertToWideChar(LocateStringA(str, pBoard->SN));
	T->m_wszBoardAssetTag = ConvertToWideChar(LocateStringA(str, pBoard->AssetTag));

	if (pBoard->Header.Length > 0x08)
	{
		T->m_wszBoardLocation = ConvertToWideChar(LocateStringA(str, pBoard->LocationInChassis));
	}
	return true;
}


bool SMBIOS::DispatchStructType(void* pHdr)
{
	typedef struct {
		BYTE t;
		bool(*Proc)(SMBIOS* T, void* p);
	} TPFUNC;

	const TPFUNC	tpfunc[] = {
		{ 0, ProcBIOSInfo },
		{ 1, ProcSysInfo },
		{ 2, ProcBoardInfo },
	};

	PSMBIOSHEADER hdr = (PSMBIOSHEADER)pHdr;

	for (UINT i = 0; i < sizeof(tpfunc) / sizeof(TPFUNC); i++)
	{
		if (tpfunc[i].t == hdr->Type)
		{
			tpfunc[i].Proc(this, (void*)hdr);
			return true;
		}
	}
	return false;
}

void SMBIOS::ParseSMBIOSStruct(void* Addr, UINT Len)
{
	LPBYTE p = (LPBYTE)(Addr);
	const LPBYTE lastAddress = p + Len;
	PSMBIOSHEADER pHeader;

	for (;;) {
		pHeader = (PSMBIOSHEADER)p;
		if (pHeader->Type == 127 && pHeader->Length == 4)
			break; // last avaiable table
		DispatchStructType((void*)p);
		LPBYTE nt = p + pHeader->Length; // point to struct end
		while (0 != (*nt | *(nt + 1))) {
		nt++;  // skip string area
			if (nt >= lastAddress) break; // avoid infinite loop when got bad structure of SMBIOS
		}
		nt += 2;
		if (nt >= lastAddress)
			break;
		p = nt;
	}
}

SMBIOS::SMBIOS() :
	m_BIOSSysVersion(0UL),
	m_BIOSECVersion(0UL),
	m_wszBIOSVendor(NULL),
	m_wszBIOSVersion(NULL),
	m_wszBIOSReleaseDate(NULL),
	m_wszSysManufactor(NULL),
	m_wszSysProductName(NULL),
	m_wszSysVersion(NULL),
	m_wszSysSerialNumber(NULL),
	m_SysUUID(GUID_NULL),
	m_wszSysSKU(NULL),
	m_wszSysFamily(NULL),
	m_wszBoardManufactor(NULL),
	m_wszBoardProductName(NULL),
	m_wszBoardVersion(NULL),
	m_wszBoardSerialNumber(NULL),
	m_wszBoardAssetTag(NULL),
	m_wszBoardLocation(NULL)
{
	initialization();
}

SMBIOS::~SMBIOS()
{
	if (m_wszBIOSVendor)
		delete[] m_wszBIOSVendor;
	if (m_wszBIOSVersion)
		delete[] m_wszBIOSVersion;
	if (m_wszBIOSReleaseDate)
		delete[] m_wszBIOSReleaseDate;
	if (m_wszSysManufactor)
		delete[] m_wszSysManufactor;
	if (m_wszSysProductName)
		delete[] m_wszSysProductName;
	if (m_wszSysVersion)
		delete[] m_wszSysVersion;
	if (m_wszSysSerialNumber)
		delete[] m_wszSysSerialNumber;
	if (m_wszSysSKU)
		delete[] m_wszSysSKU;
	if (m_wszSysFamily)
		delete[] m_wszSysFamily;
	if (m_wszBoardManufactor)
		delete[] m_wszBoardManufactor;
	if (m_wszBoardProductName)
		delete[] m_wszBoardProductName;
	if (m_wszBoardVersion)
		delete[] m_wszBoardVersion;
	if (m_wszBoardSerialNumber)
		delete[] m_wszBoardSerialNumber;
	if (m_wszBoardAssetTag)
		delete m_wszBoardAssetTag;
	if (m_wszBoardLocation)
		delete[] m_wszBoardLocation;
}

const SMBIOS& SMBIOS::getInstance(void)
{
    static SMBIOS instance;  // C++11 guarantee thread safety + lazy initialization
    return instance;
}

void SMBIOS::initialization(void)
{
	GET_SYSTEM_FIRMWARE_TABLE pGetSystemFirmwareTable = (GET_SYSTEM_FIRMWARE_TABLE)GetProcAddress(GetModuleHandle(L"kernel32"), "GetSystemFirmwareTable");

	LPBYTE pBuff = NULL;

	PBYTE tableStart = nullptr;
	UINT nTableLength = 0;
	DWORD needBufferSize = 0;

	if (pGetSystemFirmwareTable)
	{
		const DWORD Signature = 'RSMB';
		#if 0
		DWORD Signature = 'R';
		Signature = (Signature << 8) + 'S';
		Signature = (Signature << 8) + 'M';
		Signature = (Signature << 8) + 'B';
		#endif

		needBufferSize = pGetSystemFirmwareTable(Signature, 0, NULL, 0);
		pBuff = new BYTE[needBufferSize];

		needBufferSize = pGetSystemFirmwareTable(Signature, 0,
			pBuff,needBufferSize);
		if (needBufferSize > 0) {
			const PRawSMBIOSData pDMIData = (PRawSMBIOSData)pBuff;
			MajorVersion = pDMIData->MajorVersion;
			MinorVersion = pDMIData->MinorVersion;
			DMIRevision = pDMIData->DmiRevision;

			tableStart = (PBYTE) & (pDMIData->SMBIOSTableData);
			nTableLength = pDMIData->Length;
		}
	}

	if ((0 == needBufferSize) || (nTableLength > needBufferSize))
	{
		if (getWmiSmbios(&pBuff, &nTableLength))
			tableStart = pBuff;
	}

	if (tableStart)
		ParseSMBIOSStruct(tableStart, nTableLength);

	if (pBuff)
		delete[] pBuff;
}

bool SMBIOS::getWmiSmbios(BYTE ** data, UINT * length)
{
	IWbemServices *  pSvc = NULL;
	IWbemServices *  pSvcSmbios = NULL;
	IWbemLocator *   pLoc = NULL;
	HRESULT            result;
	IEnumWbemClassObject *  pEnumerator = NULL;
	std::wostringstream     query;
	std::wstring            q;
	IWbemClassObject *      pInstance = NULL;
	VARIANT                 vtProp;
	ULONG                   uReturn = 0;
	CIMTYPE                 pvtType;

	result = CoInitialize(NULL);

	if (!SUCCEEDED(result))
		return false;

	result = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (!SUCCEEDED(result)) {
		CoUninitialize();
		return false;
	}

	result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
	if (!SUCCEEDED(result)) {
		CoUninitialize();
		return false;
	}

	result = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (!SUCCEEDED(result)) {
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	result = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (!SUCCEEDED(result)) {
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	result = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), NULL, NULL, 0, NULL, 0, 0, &pSvcSmbios);
	if (!SUCCEEDED(result)) {
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	result = CoSetProxyBlanket(pSvcSmbios, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (!SUCCEEDED(result)) {
		pSvcSmbios->Release();
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	result = pSvcSmbios->CreateInstanceEnum(L"MSSMBios_RawSMBiosTables", 0, NULL, &pEnumerator);
	if (SUCCEEDED(result)) {
		while (pEnumerator) {
			result = pEnumerator->Next(WBEM_INFINITE, 1, &pInstance, &uReturn);

			if (!uReturn) {
				break;
			}

			VariantInit(&vtProp);

			result = pInstance->Get(bstr_t("SMBiosData"), 0, &vtProp, &pvtType, NULL);
			if (SUCCEEDED(result)) {
				SAFEARRAY * array = V_ARRAY(&vtProp);

				*length = array->rgsabound[0].cElements;
				*data = new BYTE[*length];
				memcpy(*data, (BYTE*)array->pvData, *length);
				VariantClear(&vtProp);
			}
		}
		pEnumerator->Release();
		if (pInstance)
			pInstance->Release();
	}

	if (pSvcSmbios)
		pSvcSmbios->Release();
	if (pSvc)
		pSvc->Release();
	if (pLoc)
		pLoc->Release();

	CoUninitialize();
	return true;
}

