/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2004 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#ifndef __CPU_INFO_H__
#define __CPU_INFO_H__

#ifdef _WIN32

#include <windows.h>

#else

#define ULONG	unsigned long
#define DWORD	unsigned long
#define BOOL	int
#define BYTE	unsigned char
#define WORD	unsigned short

#define TRUE	1
#define FALSE	0

#endif

#define SLEEP_TIME 1000

class CCpuInfo  
{
public:
	CCpuInfo(
#ifdef _WIN32
	   DWORD mask,
	   DWORD TimerType = 0
#endif
		);
	~CCpuInfo();

	char* GetInfo( DWORD ID , char* pStr );
	int GetString( DWORD ID , char* pStr );
	int GetData( DWORD ID , DWORD* pData );
	int SetData( DWORD ID , DWORD Data );
	void Init(					// Init Class CpuInfo
#ifdef _WIN32
		DWORD mask,
		DWORD TimerType = 0
#endif
		);

protected:
	int FillCpuInfo();
	void InitData();
	void CheckPowerMangement();
	BOOL ModeX86_64;
	BOOL FlagAMDMP;
	int DualWaitTime;

	int CheckEnableCPUID(void);
	double GetClock(DWORD TimerType=0);
	int  SetUpRDMSR();	
	void SetCPUFSB();
	double GetMultiplierByNameString(int bus);
//	double GetMultiplierByModelNumber(int bus);
	double GetOriginalClockByNameString();
	void SetModelNumberK8();
	void SetModelNumberP4();
	void SetModelNumberP6();
	void SetModelNumberCore2();
	void SetSocketK8();
	void SetCoreRevision(double D, double T, char* CR);
	void SetCoreRevisionAMD();
	void SetCoreRevisionVIA();
	void SetCoreRevisionIntel();

	void FillNameString();
	int FillCacheInfo();
	void FillCacheInfoIntel();
	void CacheInfoIntel(int);
	void FillCacheInfoAMD();
	void SetCPUName();	
	void SetCPUNameIntel();	
	void SetCPUNameAMD();
	void SetCPUNameTMx86();	
	void SetCPUNameVIA();
	void SetCPUNameOthers();
	void FillTMx86Ex();	
	int SetCacheSpeed();		// L2 Cache Speed Info
	int CheckAMDMobile();		// AMD Mobile CPU Check
	void EistCorrect();

	int FlagQPC;
	int FlagCPUID;
	int FlagRDTSC;
	int FlagRDMSR;
	int FlagBrand;				// Intel AMD etc..
	int FlagMultiTable;			// Multiplier Table 
	int FlagHalfSpeedCache;		// L2 Cache Speed for Intel
	int FlagAMDMobile;			// AMD Mobile CPU
	int FlagK7Desktop;			// K7 Desktop Voltage
	int FlagK8LowVoltage;		// K8 Low Voltage
	int MaxCPUID;
	DWORD MaxCPUIDEx;
	DWORD MsrEAX1;
	DWORD MsrEAX2;
	DWORD MsrEDX1;
	DWORD MsrEDX2;

#ifdef _WIN32
	DWORD m_mask;				// Processor Mask for Win32
#endif

protected:

// GetString or GetData

	// String //
	char		NameSysInfo[65];
	char		FSBMode[4];					// ""=SDR "DDR" "QDR"
	char		VendorString[13];			// OriginalVendorString
	char		VendorName[13];				// VendorName
	char		NameString[49];
	char		PlatformName[65];
	char		TypeName[33];
	char		FMS[4];
	char		CacheSpeedS[9];
	char		Name[65];
	char		CodeName[65];
	char		FullName[65];
	char		ProcessRule[65];
	char		ProcessorSerial[30];
	char		Architecture[13];
	char		MeasureMode[4];				// QPC or MMT or WT
	char		ModelNumber[32];			// ModelNumber
	char		CoreRevision[32];			// CoreRevision
	char		Logo[32];

// dobule //
	double		Clock;
	double		SystemClock;
	double		SystemBus;
	double		Multiplier;
	double		ClockOri;
	double		SystemClockOri;
	double		SystemBusOri;
	double		CorrectedClock;
	double		CorrectedSystemClock;
	double		CorrectedSystemBus;
	double		CacheSpeed;
	double		OverClock;
	double		MultiplierOri;
	double		DieSize;
	double		Transister;
	double		K8HyperTransport;

// integer //
	int			Number;
	int			Family;
	int			Model;
	int			Stepping;
	int			FamilyEx;
	int			ModelEx;
	int			FamilyX;
	int			ModelX;
	int			ExFamily;	// for AMD
	int			ExModel;	// for AMD
	int			ExStepping;	// for AMD
	int			ExFamilyX;	// for AMD
	int			ExModelX;	// for AMD
	int			BrandID;
	int			BrandIDEx;
	int			BrandIDNN;
	int			PwrLmt;
	int			CmpCap;
	int			SocketID;
	int			Apic;
	int			CacheL1I;
	int			CacheL1T;
	int			CacheL1U;
	int			CacheL1D;
	int			CacheL2;
	int			CacheL3;
	int			HyperThreadNum;
	int			PhysicalCoreNum;
	int			Type;
	int			FSBMultiplier;
	int			PlatformID;
	int			MicrocodeID;

	char		L1ITUWaysS[8];
	char		L1ITULinesS[8];
	char		L1DWaysS[8];
	char		L1DLinesS[8];
	char		L2WaysS[8];
	char		L2LinesS[8];
	char		L3WaysS[8];
	char		L3LinesS[8];

	int			L1ITUWays;
	int			L1ITULines;
	int			L1DWays;
	int			L1DLines;
	int			L2Ways;
	int			L2Lines;
	int			L3Ways;
	int			L3Lines;

	DWORD		Feature;
	DWORD		FeatureEcx;
	DWORD		FeatureEx;
	DWORD		FeatureExEcx;
	DWORD		FeatureVia;
	DWORD		FeatureTransmeta;
	DWORD		FeaturePM;
	DWORD		MiscInfo;
	DWORD		MiscInfoEx;
	DWORD		Version;
	DWORD		VersionEx;

// flag //
	int			FlagMMX;
	int			FlagMMXEx;
	int			FlagSSE;
	int			FlagSSE2;
	int			FlagSSE3;
	int			FlagSSSE3;
	int			FlagSSE4;
	int			FlagSSE41;
	int			FlagSSE42;
	int			FlagSSE4A;
	int			FlagSSE5;
	int			FlagAVX;
	int			Flag3DNow;
	int			Flag3DNowEx;
	int			FlagHT;
	int			FlagVT;
	int			FlagAmdV;
	int			FlagAA64;
	int			FlagIA64;
	int			FlagSpeedStep;
	int			FlagEIST;
	int			FlagPowerNow;
	int			FlagLongHaul;
	int			FlagLongRun;
	int			FlagClockModulation;
	int			FlagProcessorSerial;
	int			FlagIA32e;
	int			FlagNX;
	int			FlagMSR;
	int			FlagK7Sempron;
	int			FlagDualCore;
	int			FlagEistCorrect;
	int			FlagK8100MHzSteps;
	int			FlagK8Under1100V;

	// for Transmeta // 
	char		TmNameString[65];
	int			TmClock;
	int			TmNominalClock;
	int			TmCurrentVoltage;
	int			TmCurrentPerformance;
	int			TmCurrentGateDelay;
	char		TmHardwareVersion[33];
	char		TmSoftwareVersion[33];

// Love Hammer & LongHaul
	DWORD		FlagLH;
				/*
				1 = LongHaul Level 1,
				2 = LongHaul Level 2,
				5 = GeodeLX,
				6 = K6,
				7 = K7,
				8 = K8
				*/
	DWORD		LhCurrentFID;
	DWORD		LhCurrentVID;
	DWORD		LhStartupFID;
	DWORD		LhStartupVID;
	DWORD		LhMaxFID;
	DWORD		LhMinVID;
	DWORD		LhMaxVID;
	char		LhCurrentMultiplier[16];
	char		LhStartupMultiplier[16];
	char		LhMaxMultiplier[16];
	char		LhCurrentVoltage[16];
	char		LhStartupVoltage[16];
	char		LhMinVoltage[16];
	char		LhMaxVoltage[16];
};

#endif // __CPU_INFO_H__
