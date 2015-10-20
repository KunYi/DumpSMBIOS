#pragma once

#include "stdafx.h"

class SMBIOS {
private:
	SMBIOS();
	void initialization(void);
	UINT MajorVersion;
	UINT MinorVersion;
	DWORD DMIRevision;
private:
	// for Type 0
	PWCHAR m_wszBIOSVendor;
	PWCHAR m_wszBIOSVersion;
	PWCHAR m_wszBIOSReleaseDate;
	DWORD  m_BIOSSysVersion;
	DWORD  m_BIOSECVersion;
	// for Type 1
	PWCHAR m_wszSysManufactor;
	PWCHAR m_wszSysProductName;
	PWCHAR m_wszSysVersion;
	PWCHAR m_wszSysSerialNumber;
	UUID   m_SysUUID;
	PWCHAR m_wszSysSKU;
	PWCHAR m_wszSysFamily;
	// for Type 2
	PWCHAR m_wszBoardManufactor;
	PWCHAR m_wszBoardProductName;
	PWCHAR m_wszBoardVersion;
	PWCHAR m_wszBoardSerialNumber;
	PWCHAR m_wszBoardAssetTag;
	PWCHAR m_wszBoardLocation;
public:
	virtual ~SMBIOS();
	static const SMBIOS& getInstance(void);
public:
	// Type 0
	const PWCHAR BIOSVendor(void) { return m_wszBIOSVendor; }
	const PWCHAR BIOSVersion(void) { return m_wszBIOSVersion; }
	const PWCHAR BIOSReleaseDate(void) { return m_wszBIOSReleaseDate; }
	const DWORD  BIOSSysVersion(void) { return m_BIOSSysVersion; }
	const DWORD  BIOSECVersion(void) { return m_BIOSECVersion; }
	// Type 1
	const PWCHAR SysManufactor(void) { return m_wszSysManufactor; }
	const PWCHAR SysProductName(void) { return m_wszSysProductName; }
	const PWCHAR SysVersion(void) { return m_wszSysVersion; }
	const PWCHAR SysSerialNumber(void) { return m_wszSysSerialNumber; }
	const UUID&  SysUUID(void) { return m_SysUUID; }
	const PWCHAR SysSKU(void) { return m_wszSysSKU; }
	const PWCHAR SysFamily(void) { return m_wszSysFamily; }
	// Type 2
	const PWCHAR BoardManufactor(void) { return m_wszBoardManufactor; }
	const PWCHAR BoardProductName(void) { return m_wszBoardProductName; }
	const PWCHAR BoardVersion(void) { return m_wszBoardVersion; }
	const PWCHAR BoardSerialNumber(void) { return m_wszBoardSerialNumber; }
	const PWCHAR BoardAssetTag(void) { return m_wszBoardAssetTag; }
	const PWCHAR BoardLocation(void) { return m_wszBoardLocation; }
private:
	// helper function
	static bool ProcBIOSInfo(SMBIOS* T, void* p);
	static bool ProcSysInfo(SMBIOS* T, void* p);
	static bool ProcBoardInfo(SMBIOS* T, void* p);
	void ParseSMBIOSStruct(void* Addr, UINT Len);
	bool DispatchStructType(void* pHdr);
};
