/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

//When you build CpuInfoX86-64.exe, please define _X86_64 & _CPU_INFO.
//#define _X86_64
//When you build CpuInfo.dll, please define _CPU_INFO.
//#define _CPU_INFO

//#define EXEC_CPUID "CpuInfoX86-64.exe"

//#define HIGH_PRIORITY

#define LOOP_MAX 100000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "CpuInfo.h"
#include "CpuInfoID.h"
#include "MultiplierTable.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

extern "C"
{
extern  __declspec (dllexport) BOOL ReadMSR(ULONG ulECX,ULONG* pulEAX,ULONG* pulEDX);
extern  __declspec (dllexport) BOOL WriteMSR(ULONG ulECX,ULONG* pulEAX,ULONG* pulEDX);
}

void Hlt();
void SetPcr();
void UnsetPcr();
void EnableK8FVID();
double GetK8HTMulti();

#ifdef WIN32

#include <process.h>

#include "msrnt.h"
#include "InstDrv.h"
#include "../common/IsNT.h"
#include "../common/ExecAndWait.h"

//HANDLE handle = NULL;
extern DWORD gFlagStatus;
extern BOOL gK7FVID;
extern BOOL gK8FVID;

#define	ERROR_MESSAGE "LoadDriver failure!"
#define	ERROR_CAPTION "Error"

#ifndef _X86_64

int CallRing0(PVOID pvRing0FuncAddr,ULONG ulECX,ULONG* ulEAX,ULONG* ulEDX);
void Ring0ReadMSR();
void Ring0WriteMSR();
void Ring0HLT();
#endif // _X86_64

union{
	DWORD d;
	struct {
		unsigned CurrentFID:6;
		unsigned StartupFID:6;
		unsigned MaxFID:6;
		unsigned Reserved:12;
		unsigned LoveHammer:1;		// Enable LoveHammer:1
		unsigned Success:1;			// Success;1
	}b;
}f;

// for Voltage
union{
	DWORD d;
	struct {
		unsigned CurrentVID:5;
		unsigned StartupVID:5;
		unsigned MaxVID:5;
		unsigned Reserved:15;
		unsigned LoveHammer:1;		// Enable LoveHammer:1
		unsigned Success:1;			// Success;1
	}b;
}v;

#ifdef _X86_64
extern "C" {
void __fastcall _cpuid(DWORD dwOP, DWORD *lpAX, DWORD *lpBX, DWORD *lpCX, DWORD *lpDX);
}
#endif // _X86_64
#else // for UNIX //
/*
//#include <unistd.h>
__inline__ unsigned long long int rdtsc()
{
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
*/
#endif

static void cpuid(DWORD op, DWORD *EAX, DWORD *EBX, DWORD *ECX, DWORD *EDX)
{
#ifdef _WIN32

#ifndef _X86_64
	DWORD A, B, C, D;
	_asm{
		mov eax, op
		mov ecx, 0
		cpuid
		mov A, eax
		mov B, ebx
		mov C, ecx
		mov D, edx
	}
	*EAX = A;
	*EBX = B;
	*ECX = C;
	*EDX = D;
#else
	_cpuid(op,EAX,EBX,ECX,EDX);
#endif

#else
	__asm__("cpuid"
		:	"=a" (*EAX),
			"=b" (*EBX),
			"=c" (*ECX),
			"=d" (*EDX)
		:	"a" (op));
#endif

}


/*
void DebugInfo(char* mes);

void DebugInfo(char* mes)
{
	static int flag = TRUE;
	static char file[MAX_PATH];

//	MessageBox(NULL,mes,mes,MB_OK);

	if(flag){
		char* ptrEnd;
		::GetModuleFileName(NULL,file, MAX_PATH);
		if ( (ptrEnd = strrchr(file, '.')) != NULL ){*ptrEnd = '\0';strcat(file, ".log");}
		DeleteFile(file);
		flag = FALSE;
	}

	FILE *fp;
	fp = fopen(file,"a");
	fprintf(fp,"%s\n",mes);
	fclose(fp);
}
*/

CCpuInfo::CCpuInfo(
#ifdef _WIN32
				   DWORD mask,
				   DWORD TimerType
#endif
				   )
{

#ifdef _CPU_INFO
	if( IsNT() && handle == NULL ){
		BOOL Flag = FALSE;
		if( LoadDriver(DRIVER_FILE_NAME,DRIVER_NAME) == TRUE ){
			Flag = TRUE;
			gFlagStatus = 0;/* DLL_NOERROR */
		}
		if( Flag == FALSE ){
			handle = NULL;
			gFlagStatus = 3;/* DLLSTATUS_OTHERERROR */
			MessageBox(NULL,ERROR_MESSAGE,ERROR_CAPTION,MB_OK);
		}
	}
#endif // _CPU_INFO
	
	FlagK7Desktop = FALSE;
	FlagK8LowVoltage = FALSE;
	FlagEistCorrect = FALSE;
	FlagK8100MHzSteps = FALSE;
	FlagK8Under1100V = FALSE;

	K8HyperTransport = -1.0;

	Init(
#ifdef _WIN32
		mask,
		TimerType
#endif
		);
}

CCpuInfo::~CCpuInfo()
{
#ifdef _CPU_INFO

#ifdef _WIN32
	if( IsNT() ){
		UnloadDriver(DRIVER_NAME);
	}
#endif // _WIN32

#endif // _CPU_INFO
}

void CCpuInfo::Init(
#ifdef _WIN32
				   DWORD mask,
				   DWORD TimerType
#endif
				   )
{

#ifdef _WIN32
	m_mask = mask;
	if( IsNT() ){
		mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), m_mask);
	}
#endif // _WIN32

	InitData();

	if( CheckEnableCPUID() ){
		FillCpuInfo();
		FillCacheInfo();

		if( MaxCPUIDEx >= 0x80000004 ){
			FillNameString();
		}
		if( FlagBrand == AMD && MaxCPUIDEx >= 0x80000007 ){
			FlagAMDMobile = CheckAMDMobile();
		}
		if( FlagBrand == TMx86 ){
			FillTMx86Ex();
		}
		SetCPUName();

		if( FlagRDTSC ){
			Clock = GetClock(TimerType);
		}

		if( FlagRDMSR ){
			SetUpRDMSR();
			SetCPUName();
		}
	}


/* for AMD64 Debug
		FlagRDTSC = TRUE;
		FlagRDMSR = TRUE;

		FlagBrand = AMD;
		Family = 0xF;
		Model = 0xF;
		Stepping = 0xF;
		strcpy(FMS,"FFF");
		BrandID = 7;
		
		Clock = GetClock(TimerType);
	//	SetUpRDMSR();
		SetCPUName();
		SetCPUFSB();
*/
#ifdef _WIN32
	if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
#endif
}

void CCpuInfo::EistCorrect()
{
	if(Multiplier < 1.0 && MultiplierOri < 1.0)
	{
		CorrectedClock = Clock;
		CorrectedSystemClock = -1;
		CorrectedSystemBus = -1;
	}
	else if(FlagEistCorrect && MultiplierOri >= 1.0 && Multiplier < MultiplierOri)
	{
		CorrectedClock = Clock * (Multiplier / MultiplierOri);
		CorrectedSystemClock = SystemClock * (Multiplier / MultiplierOri); 
		CorrectedSystemBus = SystemBus * (Multiplier / MultiplierOri); 
	}
	else
	{
		CorrectedClock = SystemClock * Multiplier;
		CorrectedSystemClock = SystemClock;
		CorrectedSystemBus = SystemBus;
	}
}

int CCpuInfo::CheckAMDMobile()
{
	DWORD EAX, EBX, ECX, EDX;
	cpuid(0x80000007, &EAX, &EBX, &ECX, &EDX);
	// EDX
	// bit 2 = voltage ID control
	// bit 1 = frequency ID control

	return ( ((EDX >> 1) & 0x3) == 3 );
}

int CCpuInfo::SetUpRDMSR()
{
	if(	FlagBrand == INTEL && ( Family == 0xF || (Family == 6 && ModelX >= 7)) ){ /* Pentium III & P4 */
		ReadMSR(0x17,&MsrEAX1,&MsrEDX1);
		PlatformID = ( MsrEDX1 >> 18 ) & 0x7;
		ReadMSR(0x8B,&MsrEAX1,&MsrEDX1);
		MicrocodeID = MsrEDX1;
	}

	if(	FlagBrand == INTEL && Family == 6 ){
		ReadMSR(0x2A,&MsrEAX1,&MsrEDX1);
		if( Model == 9 || ModelX >= 0xC ){// Banias & Dothan for Multiplier
			ReadMSR(0x17,&MsrEAX2,&MsrEDX2);
		}
	}else if(  FlagMultiTable == CYRIX3
			|| FlagMultiTable == SAMUEL2
			|| FlagMultiTable == EZRA
			|| FlagMultiTable == EZRA_T
			|| FlagMultiTable == NEHEMIAH
			|| FlagMultiTable == ESTHER
			){
		ReadMSR(0x2A,&MsrEAX1,&MsrEDX1);
	}else if( FlagBrand == INTEL && Family == 0xF && FamilyEx == 0x0 ){ // Pentium4
/*
		ReadMSR(0x2D,&MsrEAX2,&MsrEDX2);
		char str[256];
		sprintf(str,"%08X",MsrEAX2);
		MessageBox(NULL,str,str,MB_OK);
		MsrEAX2 &= 0x00000000;
		MsrEAX2 |= 0xFFFFFFFF;
		sprintf(str,"%08X",MsrEAX2);
		MessageBox(NULL,str,str,MB_OK);
		WriteMSR(0x2D,&MsrEAX2,&MsrEDX2);
		sprintf(str,"%08X",MsrEAX2);
		MessageBox(NULL,str,str,MB_OK);
		ReadMSR(0x2D,&MsrEAX2,&MsrEDX2);
		sprintf(str,"%08X",MsrEAX2);
		MessageBox(NULL,str,str,MB_OK);
*/
		ReadMSR(0x2A,&MsrEAX1,&MsrEDX1);
		ReadMSR(0x2C,&MsrEAX2,&MsrEDX2);

	}else if( FlagBrand == AMD && FamilyX == 0x10 ){// Opteron / Athlon 64
		ReadMSR(0xC0010071,&MsrEAX1,&MsrEDX1);
		FSBMultiplier = 0;
	}else if( FlagBrand == AMD && Family == 0xF ){// Opteron / Athlon 64
	//	ReadMSR(0x8B,&MsrEAX1,&MsrEDX1);
	//	MicrocodeID = MsrEAX1;
		ReadMSR(0xC0010015,&MsrEAX1,&MsrEDX1);
		if( FlagAMDMobile ){
			ReadMSR(0xC0010042,&MsrEAX2,&MsrEDX2);
		}
		K8HyperTransport = GetK8HTMulti();
		if(K8HyperTransport > 0.5){
			FSBMultiplier = (int)K8HyperTransport;
//			sprintf(FSBMode, "%.1fx", K8HyperTransport);
		}
	}else if( FlagBrand == AMD && Family == 5 && (Model >= 8 && Model != 0xA) ){// K6-2 & K6-III
		ReadMSR(0xC0000087,&MsrEAX1,&MsrEDX1);
	}else if( FlagBrand == AMD && Family == 5 && Model == 0xA ){// GeodeLX
		ReadMSR(0x4C000014,&MsrEAX1,&MsrEDX1);
	}else if( FlagBrand == AMD && Family == 6 ){// Athlon
		ReadMSR(0xC0010015,&MsrEAX1,&MsrEDX1);
		if( Model <= 2 ){ // Athlon K7/K75
			ReadMSR(0xC0010011,&MsrEAX2,&MsrEDX2);
		}else if( FlagAMDMobile ){
			ReadMSR(0xC0010042,&MsrEAX2,&MsrEDX2);
		}
	}else if( FlagBrand == IDT && Family == 5 && Model >= 8 ){ // WinChip 2/3
		ReadMSR(0x10A,&MsrEAX1,&MsrEDX1);
		if( (Model == 8 && 7 <= Stepping && Stepping <= 9 ) // WinChip 2A
		||	(Model == 9) // WinChip 3
		){
			ReadMSR(0x147,&MsrEAX2,&MsrEDX2);
		}
	}

	SetCacheSpeed();
	if( MsrEAX1 != 0 || MsrEAX2 != 0 ){
		SetCPUFSB();
	}

	if( FlagBrand == TMx86 ){
		ClockOri = (double)TmNominalClock;
	}

	CheckPowerMangement();
	EistCorrect();
	if( ClockOri > 0.0 ){
		OverClock = CorrectedClock / ClockOri * 100.0 - 100.0;
	}

	return 0;
}

int CCpuInfo::SetCacheSpeed()
{
	if( CacheL2 <= 0 ){
		return -1;
	}

	if( FlagBrand == AMD && Family == 6 && Model <= 2 ){ // Athlon K7/K75
		DWORD CacheInfo = (MsrEAX2>>4) & 0x3;
		switch(CacheInfo){
		case 0:strcpy(CacheSpeedS,"1/2");CacheSpeed = 0.50;break;
		case 1:strcpy(CacheSpeedS,"2/5");CacheSpeed = 0.40;break;
		case 2:strcpy(CacheSpeedS,"1/3");CacheSpeed = 0.33;break;
		}
	}else if( FlagBrand == INTEL && FlagHalfSpeedCache ){
		strcpy(CacheSpeedS,"1/2");
		CacheSpeed = 0.50;
	}else{
		strcpy(CacheSpeedS,"Full");
		CacheSpeed = 1.00;
	}
	return 0;
}

extern  __declspec (dllexport) BOOL ReadMSR(ULONG ulECX,ULONG* pulEAX,ULONG* pulEDX)
{
#ifdef _WIN32
	if( IsNT() ){	
		if( ! _ReadMSR(ulECX,pulEAX,pulEDX) ){
			return FALSE;
		}else{
			return TRUE;
		}
	}else{
#ifndef _X86_64
		CallRing0(Ring0ReadMSR,ulECX,pulEAX,pulEDX);
#endif
		return TRUE;
	}
#else
	// for UNIX
	return FALSE;
#endif
}

extern  __declspec (dllexport) BOOL WriteMSR(ULONG ulECX,ULONG* pulEAX,ULONG* pulEDX)
{
#ifdef _WIN32
	if( IsNT() ){	
		if( ! _WriteMSR(ulECX,pulEAX,pulEDX) ){
			return FALSE;
		}else{
			return TRUE;
		}
	}else{
#ifndef _X86_64
		CallRing0(Ring0WriteMSR,ulECX,pulEAX,pulEDX);
#endif
		return TRUE;
	}
#else
	// for UNIX
	return FALSE;
#endif
}

void Hlt()
{
#ifdef _WIN32
	if( IsNT() ){
		_Hlt();
	}else{
#ifndef _X86_64
		CallRing0(Ring0HLT,0,NULL,NULL);
#endif
	}
#else
	// for UNIX
#endif
}

char* CCpuInfo::GetInfo( DWORD ID , char* s )
{
	GetString(ID, s);
	if( atof(s) < 0 ){
		sprintf(s, "");
	}
	return s;
}

extern "C"
{
ULONG	WINAPI _IoReadLong(ULONG address);
void WINAPI _IoWriteLong(ULONG address, ULONG data);
}

int	CCpuInfo::SetData( DWORD ID , DWORD data )
{

#ifdef HIGH_PRIORITY
	SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
#endif

	DWORD EAX, EDX;
	EAX = EDX = 0x00000000;
	int i;

	switch( ID )
	{
/*
	case LH_SET_EIST_CORRECT:
		FlagEistCorrect = data; // TRUE or FALSE
		if( FlagEistCorrect && MultiplierOri >= 1.0){
			Clock = Clock * (Multiplier / MultiplierOri);
			SystemClock = SystemClock * (Multiplier / MultiplierOri); 
			SystemBus = SystemBus * (Multiplier / MultiplierOri); 
		}else if(Multiplier >= 1.0){
			Clock = Clock * (MultiplierOri / Multiplier);
			SystemClock = SystemClock * (MultiplierOri / Multiplier); 
			SystemBus = SystemBus * (MultiplierOri / Multiplier); 
		}
		break;
*/
	case SET_VIA_VT_310DP_PIPELINE:
		if(FlagBrand == IDT && Family == 6){
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
				}
				ReadMSR(0x1143, &EAX, &EDX);
				EAX &= ~1;
				WriteMSR(0x1143, &EAX, &EDX);
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
		}
		break;
	case LH_SET_K8_LOW_VOLTAGE:
		FlagK8LowVoltage = TRUE;
		break;
	case LH_SET_K7_DESKTOP:
		FlagK7Desktop = TRUE;
		break;
	case LH_SET_FID:
		switch ( FlagLH ){
		// LongHaul
		case LONG_HAUL_LEVEL_1:
			ReadMSR(0x1147,&EAX,&EDX);

			EAX = ~(1<<23|1<<24|1<<25|1<<26);
			EAX |= (1<<19);		// Enable Software Multiplier Change
			EAX |= ( ( data & 0xF ) << 23 );
			WriteMSR(0x1147,&EAX,&EDX);

			//Sleep(100);
			Hlt();

			ReadMSR(0x1147,&EAX,&EDX);
			EAX &= ~(1<<19);
			WriteMSR(0x1147,&EAX,&EDX);
			break;
		case LONG_HAUL_LEVEL_2:
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0x110A,&EAX,&EDX);

				EAX = ~( 1<<4|1<<5|1<<6|1<<7| 1<<14 |1<<16|1<<17|1<<18|1<<19 );
				EAX |= ( 1<<8 );	// Enable Software Multiplier Change
				EAX |= ( (data & 0xF) << 16 );
				EAX |= ( ( (data & 0x10) >> 4 ) << 14 );
				EAX |= 0x00;		// Revision Key
				WriteMSR(0x110A,&EAX,&EDX);

				//Sleep(100);
				Hlt();

				ReadMSR(0x110A,&EAX,&EDX);
				EAX = ~( 1<<4|1<<5|1<<6|1<<7 );
				EAX &= ~( 1<<8 );
				EAX |= 0xF0;		// Revision Key
				WriteMSR(0x110A,&EAX,&EDX);
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
			break;

		// Love Hammer
		case LOVE_HAMMER_GEODE_LX: // GeodeLX
			ReadMSR(0x4C000014, &EAX, &EDX);
			EDX &= 0xFFFFFFC1;
			EDX |= (data << 1);
			WriteMSR(0x4C000014,&EAX,&EDX);
			break;

		case LOVE_HAMMER_K6://K6 support!?
			EAX = 0xFFF1;EDX = 0;
			WriteMSR(0xC0000086, &EAX, &EDX); // Enable the PowerNow port
			_IoWriteLong(0xFFF8, (_IoReadLong(0xFFF8) & 0xF)  // current setting
								 | (1<<12) | (1<<10) | (1<<9) // required flag 
								 | (data<<5) // FID
								 );
			EAX = 0xFFF0;EDX = 0;
			WriteMSR(0xC0000086, &EAX, &EDX); // Disable the PowerNow port
			break;
		
		case LOVE_HAMMER_K7:
			SetPcr();
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0xC0010041,&EAX,&EDX);
				EDX = 0x7D0;		// Allow minimum 10 us setting time
				EAX &= 0xFFFFFFC0;
				EAX |= (1<<16);	// Set bit 16
				EAX |= data;
				WriteMSR(0xC0010041,&EAX,&EDX);
				int loop = 0;
				do{
					ReadMSR(0xC0010042,&EAX,&EDX);
					loop++;
				}while( (EAX >> 31) & 0x1 && loop < LOOP_MAX );
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
			break;
		case LOVE_HAMMER_K8:
			EnableK8FVID();
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0xC0010041,&EAX,&EDX);
				EDX = 0x7D0;		// Allow minimum 10 us setting time
				EAX &= 0xFFFFFFC0;
				EAX |= (1<<16);	// Set bit 16
				EAX |= data;
				WriteMSR(0xC0010041,&EAX,&EDX);
				int loop = 0;
				do{
					ReadMSR(0xC0010042,&EAX,&EDX);
					loop++;
				}while( (EAX >> 31) & 0x1 && loop < LOOP_MAX );
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
				// for Athlon 64 X2
				if(Number == 2 && PhysicalCoreNum == 2){break;}
			}
			break;
		case SPEED_STEP_PM:
		case SPEED_STEP_P4:
		case SPEED_STEP_CORE_MA:
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0x00000199,&EAX,&EDX);
				EAX &= 0xFFFF00FF;
				EAX |= (data + 6) << 8; // default 6.0x
				WriteMSR(0x00000199,&EAX,&EDX);
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
			break;
		case SPEED_STEP_PENRYN:
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0x00000199,&EAX,&EDX);
				EAX &= 0xFFFF00FF;
				EAX |= (((data / 2) + 6) << 8) + ((data % 2) << 14); // default 6.0x
				WriteMSR(0x00000199,&EAX,&EDX);
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
			break;
		default:
			break;
		}
		break;
	case LH_SET_VID:
		switch ( FlagLH ){
		// LongHaul
		case LONG_HAUL_LEVEL_2:
			break;
		case LOVE_HAMMER_K6:
			break;
		case LOVE_HAMMER_K7:
			SetPcr();
			if((! FlagK7Desktop && data == 15 ) || data == 31){
				break;
			}
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				DWORD count = 32;
				while( count-- ){
					DWORD CurrentVID = 0;
					DWORD NextVID = 0;

					ReadMSR(0xC0010042,&EAX,&EDX);
					CurrentVID = EDX & 0x1F;
					if(data > CurrentVID){
						NextVID = CurrentVID + 1;
						if( NextVID == 15 && ! FlagK7Desktop ){
							NextVID++;
						}
					}else if(data < CurrentVID){
						NextVID = CurrentVID - 1;
						if( NextVID == 15 && ! FlagK7Desktop ){
							NextVID--;
						}
					}else{
						break;
					}

					if( NextVID == 31 || NextVID < 0 || NextVID > 31 ){
						break;
					}

					ReadMSR(0xC0010041,&EAX,&EDX);
					EAX &= 0xFFFFE0FF;
					EAX |= 0x00020000; // Set bit 17    (VID)
					EAX &= 0xFFFEFFFF; // Clear bit 16  (FID)
					EDX = 0x64;        // Allow minimum 100 ms settling time
					EAX |= ( NextVID << 8 );
					WriteMSR(0xC0010041,&EAX,&EDX);
					int loop = 0;
					do{
						ReadMSR(0xC0010042,&EAX,&EDX);
						loop++;
					}while( (EAX >> 31) & 0x1 && loop < LOOP_MAX );
				}
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
			break;
		case LOVE_HAMMER_K8:
			EnableK8FVID();
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				DWORD count = 32;
				while( count-- ){
					DWORD CurrentFID = 0;
					DWORD CurrentVID = 0;
					DWORD NextVID = 0;

					ReadMSR(0xC0010042,&EAX,&EDX);
					CurrentVID = EDX & 0x1F;
					CurrentFID = EAX & 0x3F;

					if(data > CurrentVID){NextVID = CurrentVID + 1;}
					else if(data < CurrentVID){NextVID = CurrentVID - 1;}
					else{break;}

					if( NextVID < 0 || NextVID > 31 ){
						break;
					}

					ReadMSR(0xC0010041,&EAX,&EDX);
					EDX = 0x16;			// Allow minimum 100 us setting time
					EAX &= 0xFFFFE0C0;
					EAX |= ( 1<<16);// Set bit 16
					EAX |= ( NextVID << 8 );
					EAX |= ( CurrentFID );
					WriteMSR(0xC0010041,&EAX,&EDX);
					int loop = 0;
					do{
						ReadMSR(0xC0010042,&EAX,&EDX);
						loop++;
					}while( (EAX >> 31) & 0x1 && loop < LOOP_MAX );
				}
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
				// for Athlon 64 X2
				if(Number == 2 && PhysicalCoreNum == 2){break;}
			}
			break;
		case SPEED_STEP_PM:
		case SPEED_STEP_P4:
		case SPEED_STEP_CORE_MA:
		case SPEED_STEP_PENRYN:
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetThreadAffinityMask(GetCurrentThread(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0x00000199,&EAX,&EDX);
				EAX &= 0xFFFFFF00;
				EAX |= data;
				WriteMSR(0x00000199,&EAX,&EDX);
				if( IsNT() ){SetThreadAffinityMask(GetCurrentThread(), mask);}
			}
			break;
		default:
			break;
		}
		break;
	case LH_SET_FVID:
		switch ( FlagLH ){
		case SPEED_STEP_PM:
		case SPEED_STEP_P4:
		case SPEED_STEP_CORE_MA:
		case SPEED_STEP_PENRYN:
			for( i = 1; i <= Number; i++ ){
				DWORD mask;
				if( IsNT() ){
					mask = (DWORD)SetProcessAffinityMask(GetCurrentProcess(), 1<<(i-1));
					if( i >= 2 ){
						Sleep(DualWaitTime);
					}
				}
				ReadMSR(0x00000199,&EAX,&EDX);
				EAX &= 0xFFFF0000;
				EAX |= data;
				WriteMSR(0x00000199,&EAX,&EDX);
				if( IsNT() ){SetProcessAffinityMask(GetCurrentProcess(), mask);}
			}
			break;
		default:
			break;
		}
		break;
	case LH_RESET_FVID_FLAG:
		gK7FVID = FALSE;
		gK8FVID = FALSE;
		break;
	default:
		break;
	}

#ifdef HIGH_PRIORITY
	SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
#endif

	return 0;
}

int CCpuInfo::GetString( DWORD ID , char* s)
{
	strcpy(s,"");
	switch( ID )
	{
// String
	case	CPU_NAME_SYS_INFO:		strcpy(s,NameSysInfo);				break;
	case	CPU_FSB_MODE:			strcpy(s,FSBMode);					break;
	case	CPU_VENDOR_STRING:		strcpy(s,VendorString);				break;
	case	CPU_VENDOR_NAME:		strcpy(s,VendorName);				break;
	case	CPU_NAME_STRING:		strcpy(s,NameString);				break;
	case	CPU_PLATFORM_NAME:		strcpy(s,PlatformName);				break;
	case	CPU_TYPE_NAME:			strcpy(s,TypeName);					break;
	case	CPU_FMS:				strcpy(s,FMS);						break;
	case	CPU_TM_NAME_STRING:		strcpy(s,TmNameString);				break;
	case	CPU_CACHE_SPEED_STR:	strcpy(s,CacheSpeedS);				break;
	case	CPU_NAME:				strcpy(s,Name);						break;
	case	CPU_CODE_NAME:			strcpy(s,CodeName);					break;
	case	CPU_FULL_NAME:			strcpy(s,FullName);					break;
	case	CPU_PROCESS_RULE:		strcpy(s,ProcessRule);				break;
	case	CPU_PROCESSOR_SERIAL:	strcpy(s,ProcessorSerial);			break;
	case	CPU_ARCHITECTURE:		strcpy(s,Architecture);				break;
	case	CPU_MEASURE_MODE:		strcpy(s,MeasureMode);				break;
	case	CPU_MODEL_NUMBER:		strcpy(s,ModelNumber);				break;
	case	CPU_CORE_REVISION:		strcpy(s,CoreRevision);				break;
	case	CPU_LOGO:				strcpy(s,Logo);						break;

	case	CPU_CACHE_L1ITU_WAYS:	strcpy(s,L1ITUWaysS);				break;
	case	CPU_CACHE_L1ITU_LINES:	strcpy(s,L1ITULinesS);				break;
	case	CPU_CACHE_L1D_WAYS:		strcpy(s,L1DWaysS);					break;
	case	CPU_CACHE_L1D_LINES:	strcpy(s,L1DLinesS);				break;
	case	CPU_CACHE_L2_WAYS:		strcpy(s,L2WaysS);					break;
	case	CPU_CACHE_L2_LINES:		strcpy(s,L2LinesS);					break;
	case	CPU_CACHE_L3_WAYS:		strcpy(s,L3WaysS);					break;
	case	CPU_CACHE_L3_LINES:		strcpy(s,L3LinesS);					break;

// double
	case	CPU_CLOCK:				sprintf(s,"%4.2f",CorrectedClock);		break;
	case	CPU_SYSTEM_CLOCK:		sprintf(s,"%4.2f",CorrectedSystemClock);break;
	case	CPU_SYSTEM_BUS:			sprintf(s,"%4.2f",CorrectedSystemBus);	break;
	case	CPU_MULTIPLIER:			sprintf(s,"%4.2f",Multiplier);			break;
	case	CPU_CLOCK_ORI:			sprintf(s,"%4.2f",ClockOri);			break;
	case	CPU_SYSTEM_CLOCK_ORI:	sprintf(s,"%4.2f",SystemClockOri);		break;
	case	CPU_SYSTEM_BUS_ORI:		sprintf(s,"%4.2f",SystemBusOri);		break;
	case	CPU_CACHE_SPEED:		sprintf(s,"%4.2f",CacheSpeed);			break;
	case	CPU_OVER_CLOCK:			sprintf(s,"%4.2f",OverClock);			break;
	case	CPU_MULTIPLIER_ORI:		sprintf(s,"%4.2f",MultiplierOri);		break;
	case	CPU_DIE_SIZE:			sprintf(s,"%4.1f",DieSize);				break;
	case	CPU_TRANSISTER:			sprintf(s,"%4.1f",Transister);			break;
	case	CPU_K8_HYPER_TRANSPORT:	sprintf(s,"%4.1f",K8HyperTransport);	break;
	case	CPU_CLOCK_UPDATE:		if(FlagRDTSC){
										CorrectedClock = Clock = GetClock();
									}
									if(FlagRDMSR){SetUpRDMSR();}
									sprintf(s,"%4.2f",CorrectedClock);
									break;
	case	CPU_MULTIPLIER_UPDATE:	if(FlagRDMSR){SetUpRDMSR();}
									sprintf(s,"%4.2f",Multiplier);
									break;
	case	CPU_CLOCK_UPDATE_WT:	if(FlagRDTSC){
										CorrectedClock = Clock = GetClock( 1 /* WT */ );
									}
									if(FlagRDMSR){SetUpRDMSR();}
									sprintf(s,"%4.2f",CorrectedClock);
									break;
	case	CPU_CLOCK_UPDATE_MMT:	if(FlagRDTSC){
										CorrectedClock = Clock = GetClock( 2 /* MMT */ );
									}
									if(FlagRDMSR){SetUpRDMSR();}
									sprintf(s,"%4.2f",CorrectedClock);
									break;
	case	CPU_CLOCK_UPDATE_QPC:	if(FlagRDTSC){
										CorrectedClock = Clock = GetClock( 3 /* QPC */ );
									}
									if(FlagRDMSR){SetUpRDMSR();}
									sprintf(s,"%4.2f",CorrectedClock);
									break;
	case	CPU_CLOCK_UPDATE_NOLOAD:if(FlagRDTSC){
										CorrectedClock = Clock = GetClock( 4 /* QPC + No Load */ );
									}
									if(FlagRDMSR){SetUpRDMSR();}
									sprintf(s,"%4.2f",CorrectedClock);
									break;
// dword
	case	CPU_BRAND_ID:			sprintf(s,"%02X",BrandID);	break;
	case	CPU_APIC:				sprintf(s,"%02X",Apic);		break;
	case	CPU_FAMILY:				sprintf(s,"%X",Family);		break;
	case	CPU_MODEL:				sprintf(s,"%X",Model);		break;
	case	CPU_STEPPING:			sprintf(s,"%X",Stepping);	break;
	case	CPU_FAMILY_X:			sprintf(s,"%X",FamilyX);	break;
	case	CPU_MODEL_X:			sprintf(s,"%X",ModelX);		break;
	case	CPU_FAMILY_EX:			sprintf(s,"%02X",FamilyEx);	break;
	case	CPU_MODEL_EX:			sprintf(s,"%X",ModelEx);	break;
	case	CPU_EX_FAMILY:			sprintf(s,"%X",ExFamily);	break;
	case	CPU_EX_MODEL:			sprintf(s,"%X",ExModel);	break;
	case	CPU_EX_STEPPING:		sprintf(s,"%X",ExStepping);	break;
	case	CPU_EX_FAMILY_X:		sprintf(s,"%X",ExFamilyX);	break;
	case	CPU_EX_MODEL_X:			sprintf(s,"%X",ExModelX);	break;

// Transmeta
	case	CPU_TM_CLOCK:
		if( TmClock > -1 ){
			sprintf(s,"%d",TmClock);
		}
		break;
	case	CPU_TM_NOMINAL_CLOCK:
		if( TmNominalClock > -1 ){
			sprintf(s,"%d",TmNominalClock);
		}
		break;
	case	CPU_TM_CURRENT_VOLTAGE:
		if( TmCurrentVoltage > -1 ){
			sprintf(s,"%d",TmCurrentVoltage);
		}
		break;
	case	CPU_TM_CURRENT_PERFORMANCE:
		if( TmCurrentPerformance > -1 ){
			sprintf(s,"%d",TmCurrentPerformance);
		}
		break;
	case	CPU_TM_CURRENT_GATE_DELAY:
		if( TmCurrentGateDelay > -1 ){
			sprintf(s,"%d",TmCurrentGateDelay);
		}
		break;
	case	CPU_TM_HARDWARE_VERSION:
		strcpy(s,TmHardwareVersion);
		break;
	case	CPU_TM_SOFTWARE_VERSION:
		strcpy(s,TmSoftwareVersion);
		break;

// Love Hammer
	case	LH_GET_CURRENT_MULTIPLIER:	sprintf(s,LhCurrentMultiplier);	break;
	case	LH_GET_STARTUP_MULTIPLIER:	sprintf(s,LhStartupMultiplier);	break;
	case	LH_GET_MAX_MULTIPLIER:		sprintf(s,LhMaxMultiplier);		break;
	case	LH_GET_CURRENT_VOLTAGE:		sprintf(s,LhCurrentVoltage);	break;
	case	LH_GET_STARTUP_VOLTAGE:		sprintf(s,LhStartupVoltage);	break;
	case	LH_GET_MIN_VOLTAGE:			sprintf(s,LhMinVoltage);		break;
	case	LH_GET_MAX_VOLTAGE:			sprintf(s,LhMaxVoltage);		break;

	default:
		strcpy(s,"");
		return -1;
		break;
	}
	return 0;
}

int CCpuInfo::GetData( DWORD ID , DWORD* d)
{
	switch(ID)
	{
	case	CPU_NUMBER:					*d = Number;				break;
	case	CPU_FAMILY:					*d = Family;				break;
	case	CPU_MODEL:					*d = Model;					break;
	case	CPU_STEPPING:				*d = Stepping;				break;
	case	CPU_FAMILY_EX:				*d = FamilyEx;				break;
	case	CPU_MODEL_EX:				*d = ModelEx;				break;
	case	CPU_EX_FAMILY:				*d = ExFamily;				break;
	case	CPU_EX_MODEL:				*d = ExModel;				break;
	case	CPU_EX_STEPPING:			*d = ExStepping;			break;
	case	CPU_BRAND_ID:				*d = BrandID;				break;
	case	CPU_FEATURE:				*d = Feature; 				break;
	case	CPU_FEATURE_ECX:			*d = FeatureEcx;			break;
	case	CPU_FEATURE_EX:				*d = FeatureEx;				break;
	case	CPU_FEATURE_EX_ECX:			*d = FeatureExEcx;			break;
	case	CPU_FEATURE_VIA:			*d = FeatureVia;			break;
	case	CPU_FEATURE_TRANSMETA:		*d = FeatureTransmeta;		break;
	case	CPU_FEATURE_PM:				*d = FeaturePM;				break;
	case	CPU_CACHE_L1I:				*d = CacheL1I;				break;
	case	CPU_CACHE_L1T:				*d = CacheL1T;				break;
	case	CPU_CACHE_L1U:				*d = CacheL1U;				break;
	case	CPU_CACHE_L1D:				*d = CacheL1D;				break;
	case	CPU_CACHE_L2:				*d = CacheL2;				break;
	case	CPU_CACHE_L3:				*d = CacheL3;				break;
	case	CPU_HYPER_THREAD_NUM:		*d = HyperThreadNum;		break;
	case	CPU_PHYSICAL_CORE_NUM:		*d = PhysicalCoreNum;		break;
	case	CPU_TYPE:					*d = Type;					break;
	case	CPU_RDMSR_EAX_1:			*d = MsrEAX1;				break;
	case	CPU_RDMSR_EAX_2:			*d = MsrEAX2;				break;
	case	CPU_FSB_MULTIPLIER:			*d = FSBMultiplier;			break;
	case	CPU_VERSION:				*d = Version;				break;
	case	CPU_VERSION_EX:				*d = VersionEx;				break;
	case	CPU_FLAG_BRAND:				*d = FlagBrand;				break;
	case	CPU_APIC:					*d = Apic;					break;
	case	CPU_PLATFORM_ID:			*d = PlatformID;			break;
	case	CPU_MICROCODE_ID:			*d = MicrocodeID;			break;
	case	CPU_TYPE_ID:				*d = Type;					break;

	case	CPU_FLAG_MMX:				*d = FlagMMX;				break;
	case	CPU_FLAG_MMX_EX:			*d = FlagMMXEx;				break;
	case	CPU_FLAG_SSE:				*d = FlagSSE;				break;
	case	CPU_FLAG_SSE2:				*d = FlagSSE2;				break;
	case	CPU_FLAG_SSE3:				*d = FlagSSE3;				break;
	case	CPU_FLAG_SSSE3:				*d = FlagSSSE3;				break;
	case	CPU_FLAG_SSE4:				*d = FlagSSE4;				break;
	case	CPU_FLAG_SSE41:				*d = FlagSSE41;				break;
	case	CPU_FLAG_SSE42:				*d = FlagSSE42;				break;
	case	CPU_FLAG_SSE4A:				*d = FlagSSE4A;				break;
	case	CPU_FLAG_SSE5:				*d = FlagSSE5;				break;
	case	CPU_FLAG_AVX:				*d = FlagAVX;				break;
	case	CPU_FLAG_3DNOW:				*d = Flag3DNow;				break;
	case	CPU_FLAG_3DNOW_EX:			*d = Flag3DNowEx;			break;
	case	CPU_FLAG_HYPER_THREAD:		*d = FlagHT;				break;
	case	CPU_FLAG_VT:				*d = FlagVT;				break;
	case	CPU_FLAG_AMD_V:				*d = FlagAmdV;				break;
	case	CPU_FLAG_AA64:				*d = FlagAA64;				break;
	case	CPU_FLAG_IA64:				*d = FlagIA64;				break;
	case	CPU_FLAG_PROCESSOR_SERIAL:	*d = FlagProcessorSerial;	break;
	case	CPU_FLAG_SPEED_STEP:		*d = FlagSpeedStep;			break;
	case	CPU_FLAG_EIST:				*d = FlagEIST;				break;
	case	CPU_FLAG_POWER_NOW:			*d = FlagPowerNow;			break;
	case	CPU_FLAG_LONG_HAUL:			*d = FlagLongHaul;			break;
	case	CPU_FLAG_LONG_RUN:			*d = FlagLongRun;			break;
	case	CPU_FLAG_IA32E:				*d = FlagIA32e;				break;
	case	CPU_FLAG_NX:				*d = FlagNX;				break;
	case	CPU_FLAG_DUAL_CORE:			*d = FlagDualCore;			break;
	case	CPU_FLAG_MSR:				*d = FlagMSR;				break;
	case	CPU_FLAG_EIST_CORRECT:		*d = FlagEistCorrect;		break;
	case	CPU_FLAG_K8_100MHZ_STEPS:	*d = FlagK8100MHzSteps;		break;
	case	CPU_FLAG_K8_UNDER_1100V:	*d = FlagK8Under1100V;		break;

	case	CPU_CACHE_L1ITU_WAYS:		*d = L1ITUWays;				break;
	case	CPU_CACHE_L1ITU_LINES:		*d = L1ITULines;			break;
	case	CPU_CACHE_L1D_WAYS:			*d = L1DWays;				break;
	case	CPU_CACHE_L1D_LINES:		*d = L1DLines;				break;
	case	CPU_CACHE_L2_WAYS:			*d = L2Ways;				break;
	case	CPU_CACHE_L2_LINES:			*d = L2Lines;				break;
	case	CPU_CACHE_L3_WAYS:			*d = L3Ways;				break;
	case	CPU_CACHE_L3_LINES:			*d = L3Lines;				break;

	case	CPU_RDMSR_EDX_1:			*d = MsrEDX1;				break;
	case	CPU_RDMSR_EDX_2:			*d = MsrEDX2;				break;
	case	CPU_TM_CLOCK:				*d = TmClock;				break;
	case	CPU_TM_NOMINAL_CLOCK:		*d = TmNominalClock;		break;
	case	CPU_TM_CURRENT_VOLTAGE:		*d = TmCurrentVoltage;		break;
	case	CPU_TM_CURRENT_PERFORMANCE:	*d = TmCurrentPerformance;	break;
	case	CPU_TM_CURRENT_GATE_DELAY:	*d = TmCurrentGateDelay;	break;
	case	CPU_TM_UPDATE:				*d = 0;	FillTMx86Ex();		break;

// Love Hammer
	case	LH_GET_CURRENT_STATUS:
		DWORD EAX, EDX;
		switch ( FlagLH ){
		case LONG_HAUL_LEVEL_1:
		case LONG_HAUL_LEVEL_2:
			if(FlagRDMSR){SetUpRDMSR();}
			sprintf(LhCurrentMultiplier,"%4.2f",Multiplier);
			break;
		case LOVE_HAMMER_GEODE_LX:
			ReadMSR(0x4C000014, &EAX, &EDX);
			LhCurrentFID = (EDX >> 1) & 0x1F;
			sprintf(LhCurrentMultiplier,"%d.0", LhCurrentFID + 1);
			break;
		case LOVE_HAMMER_K6:
			if(FlagRDMSR){SetUpRDMSR();}
			sprintf(LhCurrentMultiplier,"%4.2f",Multiplier);
			/* LhCurrentFID is prepared by SetCPUFSB() */
/*
			EAX = 0xFFF1;EDX = 0;
			WriteMSR(0xC0000086, &EAX, &EDX); // Enable the PowerNow port
			LhCurrentFID = (_IoReadLong(0xFFF8) & 0x70) >> 4;
			sprintf(LhCurrentMultiplier, "%.1f", MultiTableK6M13[LhCurrentFID] / 2.0);
			strcpy(LhMaxMultiplier, "6.0");
			EAX = 0xFFF0;EDX = 0;
			WriteMSR(0xC0000086, &EAX, &EDX); // Disable the PowerNow port
*/
			break;
		case LOVE_HAMMER_K7:
			ReadMSR(0xC0010042,&EAX,&EDX);
			LhCurrentFID = EAX & 0x1F;
			LhCurrentVID = EDX & 0x1F;
			LhStartupFID = (EAX >> 8) & 0x1F;
			LhStartupVID = (EDX >> 8) & 0x1F;
			LhMaxFID = (EAX >> 16) & 0x1F;
			LhMaxVID = (EDX >> 16) & 0x1F;
			// Multiplier
			sprintf(LhCurrentMultiplier,"%.1f", MultiTableMobileAthlon[LhCurrentFID] / 2.0);
			sprintf(LhStartupMultiplier,"%.1f", MultiTableMobileAthlon[LhStartupFID] / 2.0);
			sprintf(LhMaxMultiplier,	"%.1f", MultiTableMobileAthlon[LhMaxFID] / 2.0);
			// Voltage
			if( FlagK7Desktop ){
				sprintf(LhCurrentVoltage,	"%.3f", VoltageTableDesktopAthlon[LhCurrentVID] / 1000.0);
				sprintf(LhStartupVoltage,	"%.3f", VoltageTableDesktopAthlon[LhStartupVID] / 1000.0);
				sprintf(LhMaxVoltage,		"%.3f", VoltageTableDesktopAthlon[LhMaxVID] / 1000.0);
			}else{
				sprintf(LhCurrentVoltage,	"%.3f", VoltageTableMobileAthlon[LhCurrentVID] / 1000.0);
				sprintf(LhStartupVoltage,	"%.3f", VoltageTableMobileAthlon[LhStartupVID] / 1000.0);
				sprintf(LhMaxVoltage,		"%.3f", VoltageTableMobileAthlon[LhMaxVID] / 1000.0);
			}
			break;
		case LOVE_HAMMER_K8:
			ReadMSR(0xC0010042,&EAX,&EDX);
			LhCurrentFID = EAX & 0x3F;
			LhCurrentVID = EDX & 0x1F;
			LhStartupFID = (EAX >> 8) & 0x3F;
			LhStartupVID = (EDX >> 8) & 0x1F;
			LhMaxFID = (EAX >> 16) & 0x3F;
			LhMaxVID = (EDX >> 16) & 0x1F;

			// Multiplier
			sprintf(LhCurrentMultiplier,"%.1f", LhCurrentFID / 2.0 + 4.0);
			sprintf(LhStartupMultiplier,"%.1f", LhStartupFID / 2.0 + 4.0);
			sprintf(LhMaxMultiplier,	"%.1f", LhMaxFID / 2.0 + 4.0);
			// Voltage
			if( FlagK8LowVoltage ){
				sprintf(LhCurrentVoltage,	"%.3f", 1.450 - LhCurrentVID * 0.025);
				sprintf(LhStartupVoltage,	"%.3f", 1.450 - LhStartupVID * 0.025);
				sprintf(LhMaxVoltage,		"%.3f", 1.450 - LhMaxVID * 0.025);
			}else{
				sprintf(LhCurrentVoltage,	"%.3f", 1.550 - LhCurrentVID * 0.025);
				sprintf(LhStartupVoltage,	"%.3f", 1.550 - LhStartupVID * 0.025);
				sprintf(LhMaxVoltage,		"%.3f", 1.550 - LhMaxVID * 0.025);
			}
			break;
		case SPEED_STEP_PM:
			ReadMSR(0x00000198,&EAX,&EDX);
			LhCurrentFID = ((EAX >> 8) & 0xFF) - 6;
			LhCurrentVID = EAX & 0x3F;
			LhMaxFID = ((EDX >> 8) & 0xFF) - 6;
			LhMaxVID = EDX & 0x3F;
			// Multiplier
			sprintf(LhCurrentMultiplier,"%.1f", (double)LhCurrentFID + 6.0);
			sprintf(LhMaxMultiplier,	"%.1f", (double)LhMaxFID + 6.0);
			// Voltage
			sprintf(LhCurrentVoltage,	"%.3f", 0.700 + LhCurrentVID * 0.016);
			sprintf(LhMaxVoltage,		"%.3f", 0.700 + LhMaxVID * 0.016);
			break;
		case SPEED_STEP_P4:
			ReadMSR(0x00000198,&EAX,&EDX);
			LhCurrentFID = ((EAX >> 8) & 0xFF) - 6;
			LhCurrentVID = EAX & 0x3F;
			LhMaxFID = ((EDX >> 8) & 0xFF) - 6;
			LhMaxVID = EDX & 0x3F;
			// Multiplier
			sprintf(LhCurrentMultiplier,"%.1f", (double)LhCurrentFID + 6.0);
			sprintf(LhMaxMultiplier,	"%.1f", (double)LhMaxFID + 6.0);
			// Voltage
			sprintf(LhCurrentVoltage,	"%.4f", VoltageTablePentium4[LhCurrentVID] / 10000.0);
			sprintf(LhMaxVoltage,		"%.4f", VoltageTablePentium4[LhCurrentVID] / 10000.0);
			break;
		case SPEED_STEP_CORE_MA:
			ReadMSR(0x00000198,&EAX,&EDX);
			LhCurrentFID = ((EAX >> 8) & 0xFF) - 6;
			LhCurrentVID = EAX & 0x3F;
			LhMaxFID = ((EDX >> 8) & 0xFF) - 6;
			// Multiplier
			sprintf(LhCurrentMultiplier,"%.1f", (double)LhCurrentFID + 6.0);
			sprintf(LhMaxMultiplier,	"%.1f", (double)LhMaxFID + 6.0);
			// Voltage
//			if (ReadMSR(0x000000CE,&EAX,&EDX)) {
			if (ModelX >= 0xF) {
				// Core2Duo or later
				ReadMSR(0x000000CE,&EAX,&EDX);
				LhMinVID = (EDX >> 16) & 0x3F;
				LhMaxVID = EDX & 0x3F;
				sprintf(LhMinVoltage, "%.3f", PentiumD_Voltage_Base + (double)LhMinVID * PentiumD_Voltage_Step);
			} else {
				LhMaxVID = EDX & 0x3F;
			}
			sprintf(LhCurrentVoltage,	"%.3f", PentiumD_Voltage_Base + (double)LhCurrentVID * PentiumD_Voltage_Step);
			sprintf(LhMaxVoltage, "%.3f", PentiumD_Voltage_Base + (double)LhMaxVID * PentiumD_Voltage_Step);
			break;
		case SPEED_STEP_PENRYN:
			ReadMSR(0x00000198,&EAX,&EDX);
			LhCurrentFID = (((EAX >> 7) & 0x3E) | ((EAX >> 14) & 1)) - 12;
			LhCurrentVID = EAX & 0x3F;
			LhMaxFID = (((EDX >> 7) & 0x3E) | ((EDX >> 14) & 1)) - 12;
			ReadMSR(0x000000CE,&EAX,&EDX);
			LhMinVID = (EDX >> 16) & 0x3F;
			LhMaxVID = EDX & 0x3F;
			// Multiplier
			sprintf(LhCurrentMultiplier,"%.1f", (double)LhCurrentFID / 2.0 + 6.0);
			sprintf(LhMaxMultiplier,	"%.1f", (double)LhMaxFID / 2.0 + 6.0);
			// Voltage
			sprintf(LhCurrentVoltage,	"%.3f", PentiumD_Voltage_Base + (double)LhCurrentVID * PentiumD_Voltage_Step);
			sprintf(LhMinVoltage, "%.3f", PentiumD_Voltage_Base + (double)LhMinVID * PentiumD_Voltage_Step);
			sprintf(LhMaxVoltage, "%.3f", PentiumD_Voltage_Base + (double)LhMaxVID * PentiumD_Voltage_Step);
			break;
		default:
			break;
		}
		break;
	case	LH_GET_TYPE:
		*d = FlagLH;
		break;
	case	LH_GET_TABLE_TYPE:
		*d = FlagMultiTable;
		break;
	case	LH_GET_CURRENT_FID:
		*d = LhCurrentFID;
		break;
	case	LH_GET_STARTUP_FID:
		*d = LhStartupFID;
		break;
	case	LH_GET_MAX_FID:
		*d = LhMaxFID;
		break;
	case	LH_GET_CURRENT_VID:
		*d = LhCurrentVID;
		break;
	case	LH_GET_STARTUP_VID:
		*d = LhStartupVID;
		break;
	case	LH_GET_MAX_VID:
		*d = LhMaxVID;
		break;
	default:
		*d = -1;
		return -1;
	}
	return 0;
}

void CCpuInfo::SetCPUName()
{
	switch( FlagBrand ){
	case INTEL:
		SetCPUNameIntel();
		break;
	case AMD:
		SetCPUNameAMD();
		break;
	case TMx86:
		SetCPUNameTMx86();
		break;
	case CYRIX:
	case IDT:
		SetCPUNameVIA();
		break;
	default:
		SetCPUNameOthers();
		break;
	}

// CPUName & CodeName
	if( strlen(NameSysInfo) != 0 && strchr( NameSysInfo,'(' ) ){
		char str[65];
		char* token;
		strcpy(str,NameSysInfo);
		token = strtok( str , "(" );	strcpy(Name,str);
		token = strtok( NULL , ")" );	strcpy(CodeName,token);
		Name[strlen(Name) - 1] = '\0';
	}else{
		strcpy(Name,NameSysInfo);
	}

	sprintf(FullName,"%s %s",VendorName,Name);
}

void CCpuInfo::SetCPUNameIntel()
{
	int F = Family,M = Model,S = Stepping,FE = FamilyEx;
	char *n,*p,*r;
	n = "";
	p = "";
	r = "";

	SetCoreRevisionIntel();

	switch( Family ){
	case 0xF:
		//////////////
		// Itanuim 2
		//////////////
		if(FE > 0x01){
			n = "Itanium 2 (Madison)";
		}else if(FE == 0x01){
			n = "Itanium 2 (McKinley)";
		}else{
		//////////////
		// Pentium 4
		//////////////
			switch( Model ){
			case 6:
				FlagMultiTable = NORTHWOOD;
				r = "0.065";
				if( strstr(NameString,"Xeon") ){
					p = "LGA771";
					n = "Xeon (Dempsey)";
				}else{
					p = "LGA775";
					if(PhysicalCoreNum == 2 && HyperThreadNum == 2){
						n = "Pentium XE (Presler)";
					}else if(PhysicalCoreNum == 2){
						n = "Pentium D (Presler)";
					}else if(CacheL2 <= 512){
						n = "Celeron D (Cedar Mill-V)";
					}else{
						n = "Pentium 4 (Cedar Mill)";
					}
				}
				break;
			case 5:
				FlagMultiTable = NORTHWOOD;
				r = "0.09";
				p = "LGA775";
				n = "Pentium 4 (Tejas)";
				break;
			case 4:
			case 3:
				FlagMultiTable = NORTHWOOD;
				r = "0.09";
				if(PlatformID == 2 || PlatformID == 3){
					p = "Socket 478";
				}else if(PlatformID == 4){
					p = "LGA775";
				}else{
					p = "Socket 478/LGA775";
				}

				if( ! strstr(NameString,"Pentium") && CacheL2 >= 1024 && BrandID != 0xC &&
					(PlatformID <= 0 || BrandID == 0xB || strstr(NameString,"Xeon")) ){// || BrandID <= 0 || 
					if(PhysicalCoreNum == 2){
						n = "Xeon (Paxville DP)";
					}else if( CacheL2 == 2048 ){
						n = "Xeon (Irwindale)";
					}else{
						n = "Xeon (Nocona)";
					}
					p = "Socket 604";
				}else if(BrandID == 0xC || strstr(NameString,"Xeon")){
					if(PhysicalCoreNum == 2){
						n = "Xeon (Paxville)";
					}else if( CacheL3 == 0 ){
						n = "Xeon MP (Cranford)";
					}else{
						n = "Xeon MP (Potomac)";
					}
					p = "Socket 604";
				}else if(PhysicalCoreNum == 2 && HyperThreadNum == 2){
					n = "Pentium XE (Smithfield)";
					FlagHT = TRUE;
					FlagDualCore = TRUE;
				}else if(PhysicalCoreNum == 2){
					n = "Pentium D (Smithfield)";
					FlagHT = FALSE;
					FlagDualCore = TRUE;
				}else if( CacheL2 == 2048 && SystemClockOri >= 266.6 ){
					n = "Pentium 4 XE (Prescott-2M)";
				}else if( CacheL2 == 2048 ){
					n = "Pentium 4 HT (Prescott-2M)";
				}else if( strstr(NameString,"obile Celeron") && CacheL2 == 256 ){
					n = "Mobile Celeron (Prescott-256K)";
				}else if( strstr(NameString,"obile Celeron") && CacheL2 == 512 ){
					n = "Mobile Celeron (Prescott-512K)";
				}else if( strstr(NameString,"Celeron") && CacheL2 == 256 ){
					n = "Celeron D (Prescott-256K)";
				}else if( strstr(NameString,"Celeron") && CacheL2 == 128 ){
					n = "Celeron D (Prescott-128K)";
				}else if( strstr(NameString,"obile")){
					if( HyperThreadNum >= 2 ){
						n = "Mobile Pentium 4 HT (Prescott)";
					}else{
						n = "Mobile Pentium 4 (Prescott)";
					}
				}else if( HyperThreadNum >= 2 ){
					n = "Pentium 4 HT (Prescott)";
				}else{
					n = "Pentium 4 (Prescott)";
				}
				break;
			case 2:
				FlagMultiTable = NORTHWOOD;
				r = "0.13";
				if(PlatformID == 2 || PlatformID == 3){
					p = "Socket 478";
				}else if(PlatformID == 4){
					p = "LGA775";
				}else{
					p = "Socket 478/LGA775";
				}
				if(BrandID == 0xC){
					n = "Xeon MP (Gallatin)";
					p = "Socket 603/604";
				}else if(BrandID == 0xB){
					n = "Xeon (Prestonia)";
					p = "Socket 603/604";
				}else if(BrandID == 0x9 && CacheL3 == 2048){
					n = "Pentium 4 XE (Gallatin)";
				}else if( CacheL2 == 256 ){
					n = "Mobile Celeron (Northwood-256K)";
					p = "Socket 478";
				}else if( CacheL2 == 128 && strstr(NameString,"Pentium") ){
					n = "Pentium 4 (Northwood-128K)";
				}else if( CacheL2 == 128){
					n = "Celeron (Northwood-128K)";
				}else if( (BrandID == 0xE || BrandID == 0xF) && HyperThreadNum >= 2 ){
					n = "Mobile Pentium 4 HT (Northwood)";
					FlagClockModulation = TRUE;
				}else if( (BrandID == 0xE || BrandID == 0xF) && SystemClockOri >= 125.0 ){
					n = "Mobile Pentium 4 (Northwood)";
					FlagClockModulation = TRUE;
				}else if( BrandID == 0xE || BrandID == 0xF ){
					n = "Mobile Pentium 4-M (Northwood)";
					FlagClockModulation = TRUE;
				}else if( HyperThreadNum >= 2 ){
					n = "Pentium 4 HT (Northwood)";
				}else{
					n = "Pentium 4 (Northwood)";
				}
				break;
			case 1:
			case 0:
				r = "0.18";
				FlagMultiTable = WILLAMETTE;
				
				if( ( BrandID == 0xB && Model <= 1 && Stepping < 3 ) || BrandID == 0xC ){
					n = "Xeon MP (Foster MP)";
					p = "Socket 603";
				}else if( BrandID == 0xE || BrandID == 0xB ){
					n = "Xeon (Foster)";
					p = "Socket 603";
				}else if( CacheL2 == 128 ){
					n = "Celeron (Willamette-128K)";
					p = "Socket 478";
				}else{
					n = "Pentium 4 (Willamette)";
					if( Model == 1 ){
						p = "Socket 478";
					}else{
						p = "Socket 423";
					}
				}
				break;
			default:
				n = "Unknown CPU";
				break;
			}
			break;
		}
		break;
	case 0x7:
		n = "Itanium (Merced)";
		break;
	case 0x6:
		if(ModelX >= 0x10){
			switch(ModelX){
		/////////////
		// Atom
		/////////////
			case 0x1C:
				r = "0.045";
				n = "Atom";
				break;
		/////////////
		// Penryn
		/////////////
			case 0x17:
				r = "0.045";
				FlagMultiTable = PENRYN;
				//FlagClockModulation = TRUE;
				if(PhysicalCoreNum == 4){
					if(strstr(NameString, "Xeon") || PlatformID == 6){
						n = "Xeon (Harpertown)";
						p = "LGA771";
					}else if(strstr(NameString, "3.00")){
						n = "Core 2 Extreme (Yorkfield)";
						p = "LGA775";
					}else{
						n = "Core 2 Quad (Yorkfield)";
						p = "LGA775";
					}
				}else{
					n = "Core 2 Duo (Wolfdale)";
					p = "LGA775";
				}
				break;
			case 0x16:
				r = "0.065";
				FlagMultiTable = DOTHAN;
				FlagClockModulation = TRUE;
				if(PhysicalCoreNum == 4){
					if(strstr(NameString, "Xeon")){
						n = "Xeon (Clovertown)";
						p = "LGA771";
					}else if(strstr(NameString, "2.66") || strstr(NameString, "2.93")){ //
						n = "Core 2 Extreme (Kentsfield)";
						p = "LGA775";
					}else{
						n = "Core 2 Quad (Kentsfield)";
						p = "LGA775";
					}
				}else if(strstr(NameString, "Xeon") && strstr(NameString, " 30")){
					n = "Xeon (Conroe)";
					p = "LGA775";
				}else if(PlatformID == 2 || Number == 4 || strstr(NameString, "Xeon")){
					n = "Xeon (Woodcrest)";
					p = "LGA771";
				}else if(strstr(NameString, "2.93")){ // 
					n = "Core 2 Extreme (Conroe)";
					p = "LGA775";
				}else if(PhysicalCoreNum == 2){
					if(PlatformID == 5){
						p = "Socket 479";
						n = "Core 2 Duo (Merom)";
					}else{
						p = "LGA775";
						if(CacheL2 <= 1024){
							n = "Pentium DC (Conroe-1M)";
						}else if(CacheL2 <= 2048){
							n = "Core 2 Duo (Allendale)";
						}else{
							n = "Core 2 Duo (Conroe)";
						}
					}
				}else{
					if(PlatformID == 5){
						p = "Socket 479";
						n = "Celeron (Merom-L)";
					}else{
						p = "LGA775";
						n = "Celeron (Conroe-L)";
					}
				}
				break; // switch
			}

		}else{
			switch( Model ){
			/////////////
			// Conroe/Merom
			/////////////
			case 0xF:
				r = "0.065";
				FlagMultiTable = DOTHAN;
				FlagClockModulation = TRUE;
				if(PhysicalCoreNum == 4){
					if(strstr(NameString, "Xeon")){
						n = "Xeon (Clovertown)";
						p = "LGA771";
					}else if(strstr(NameString, "2.93")){ //
						n = "Core 2 Extreme QX6800 (Kentsfield)";
						p = "LGA775";
					}else if(strstr(NameString, "Q6700")){
						n = "Core 2 Quad Q6700 (Kentsfield)";
						p = "LGA775";
					}else if(strstr(NameString, "Q6600")){
						n = "Core 2 Quad Q6600 (Kentsfield)";
						p = "LGA775";
					}else if(strstr(NameString, "2.66")){
						n = "Core 2 Extreme QX6700 (Kentsfield)";
						p = "LGA775";
					}else{
						n = "Core 2 Quad (Kentsfield)";
						p = "LGA775";
					}
				}else if(strstr(NameString, "Xeon") && strstr(NameString, " 30")){
					n = "Xeon (Conroe)";
					p = "LGA775";
				}else if(PlatformID == 2 || Number == 4 || strstr(NameString, "Xeon")){
					n = "Xeon (Woodcrest)";
					p = "LGA771";
				}else if(strstr(NameString, "2.93")){ // 
					n = "Core 2 Extreme (Conroe)";
					p = "LGA775";
				}else if(PhysicalCoreNum == 2){
					if(PlatformID == 5 || PlatformID == 7){
						p = "Socket 479";
						n = "Core 2 Duo (Merom)";
					}else{
						p = "LGA775";
						if(CacheL2 <= 1024){
							n = "Pentium DC (Conroe-1M)";
						}else if(CacheL2 <= 2048){
							n = "Core 2 Duo (Allendale)";
						}else{
							n = "Core 2 Duo (Conroe)";
						}
					}
				}else{
					if(PlatformID == 5 || PlatformID == 7){
						p = "Socket 479";
						n = "Celeron (Merom-L)";
					}else{
						p = "LGA775";
						n = "Celeron (Conroe-L)";
					}
				}
				break;
			/////////////
			// Yonah
			/////////////
			case 0xE:
				r = "0.065";
				FlagMultiTable = DOTHAN;
				FlagClockModulation = TRUE;	
				p = "Socket 479";
				if(Number == 4 || strstr(NameString, "Xeon")){
					n = "Xeon (Sossaman)";
				}else if(CacheL2 == 1024 && PhysicalCoreNum == 2){
					n = "Pentium Dual Core (Yonah)";
				}else if(CacheL2 == 1024){
					n = "Celeron M (Yonah)";
				}else if(PhysicalCoreNum == 2){
					n = "Core Duo (Yonah DC)";
				}else{
					n = "Core Solo (Yonah SC)";
				}
				break;
			/////////////
			// Dothan
			/////////////
			case 0xD:
			case 0xC:
				r = "0.09";
				FlagMultiTable = DOTHAN;
				p = "Socket 479";
				if( CacheL2 == 1024 ){
					n = "Celeron M (Dothan-1024K)";
				}else{
					n = "Pentium M (Dothan)";
					FlagClockModulation = TRUE;	
				}
				break;
			/////////////
			// Tualatin
			/////////////
			case 0xB:
				r = "0.13";
				FlagMultiTable = TUALATIN;
				if( BrandID == 0x7 ){
					n = "Mobile Celeron (Tualatin)";
					p = "Mobile Module";
					FlagClockModulation = TRUE;
				}else if( BrandID == 0x6 ){
					n = "Mobile Pentium III-M (Tualatin)";
					p = "Mobile Module";
					FlagClockModulation = TRUE;
				}else if( BrandID == 0x3 && S >= 1 ){
					n = "Celeron (Tualatin)";
					p = "Socket 370";
				}else if( CacheL2 == 512 ){
					n = "Pentium III-S (Tualatin)";
					p = "Socket 370";
				}else if( BrandID == 0x4 || BrandID == 0x2 ){
					n = "Pentium III (Tualatin)";
					p = "Socket 370";
				}else{
					n = "Celeron (Tualatin)";
					p = "Socket 370";
				}
				break;
			/////////////
			// Cascades
			/////////////
			case 0xA:
				r = "0.18";
				FlagMultiTable = COPPERMINE;
				n = "Pentium III Xeon (Cascades)";
				break;
			///////////
			// Banias
			///////////
			case 9:
				r = "0.13";
				p = "Socket 479";
				FlagMultiTable = BANIAS;
				if( CacheL2 == 1024 ){
					n = "Pentium M (Banias)";
					FlagClockModulation = TRUE;
				}else{
					n = "Celeron M (Banias-512K)";
				}
				break;
			///////////////
			// Coppermine
			///////////////
			case 8:
				r = "0.18";
				FlagMultiTable = COPPERMINE;
				if( BrandID == 0x3 ){
					n = "Pentium III Xeon (Cascades)";
					p = "Slot 2";
				}else if( CacheL2 == 256 && PlatformID % 2){
					n = "Mobile Pentium III (Coppermine)";
					p = "Mobile Module";
				}else if( CacheL2 == 256 && PlatformID == 0){
					n = "Pentium III (Coppermine)";
					p = "Slot 1";
				}else if( CacheL2 == 256 ){
					n = "Pentium III (Coppermine)";
					p = "Socket 370";
				}else if( CacheL2 <= 128 && PlatformID % 2){
					n = "Mobile Celeron (Coppermine-128K)";
					p = "Mobile Module";
				}else if( CacheL2 <= 128 ){
					n = "Celeron (Coppermine-128K)";
					p = "Socket 370";
				}
				break;
			///////////
			// Katmai
			///////////
			case 7:
				r = "0.25";
				FlagMultiTable = P6;
				if( CacheL2 == 1024 ){
					n = "Pentium III Xeon (Tanner)";
					p = "Slot 2";
				}else{
					n = "Pentium III (Katmai)";
					p = "Slot 1";
					FlagHalfSpeedCache = TRUE;
				}
				break;
			//////////////////////
			// Dixon & Mendocino
			//////////////////////
			case 6:
				r = "0.25";
				FlagMultiTable = P6;
				if( (S == 0xA || S == 0xD ) && CacheL2 == 256){
					n = "Mobile Pentium II (Dixon)";
					p = "Mobile Module";
				}else if( (S == 0xA || S == 0xD ) && CacheL2 == 128){
					n = "Mobile Celeron (Dixon-128K)";
					p = "Mobile Module";
				}else{
					n = "Celeron (Mendocino)";
					p = "Socket 370";
				}
				break;
			//////////////
			// Deschutes
			//////////////
			case 5:
				r = "0.25";
				FlagMultiTable = P6;
				if( CacheL2 > 512 ){
					n = "Pentium II Xeon (Deschutes)";
					p = "Slot 2";
				}else if( CacheL2 == 512 && Type == 0x1 ){
					n = "Pentium II OverDrive";
					FlagHalfSpeedCache = TRUE;
				}else if( CacheL2 == 512 ){
					n = "Pentium II (Deschutes)";
					p = "Slot 1";
					FlagHalfSpeedCache = TRUE;
				}else if( CacheL2 == 0 ){
					n = "Celeron (Covinton)";
					p = "Slot 1";
				}
				break;
			case 4:
				n = "OverDrive";
				break;
			case 3:
				FlagMultiTable = P6;
				if( Type == 0x1 ){
					r = "0.25";
					n = "Pentium II OverDrive";
				}else{
					n = "Pentium II (Klamath)";
					p = "Slot 1";
					r = "0.28";
					FlagHalfSpeedCache = TRUE;
				}
				break;
			///////
			// P6
			///////
			case 2:
				n = "Pentium Pro (P6)";
				p = "Socket 8";
				FlagMultiTable = P6;
				r = "0.35";
				break;
			case 1:
				n = "Pentium Pro (P6)";
				p = "Socket 8";
				FlagMultiTable = P6;
				if(S<=2){
					r = "0.60";
				}else{
					r = "0.35";
				}
				break;
			case 0:
				n = "Pentium Pro (P6)";
				p = "Socket 8";
				r = "0.60";
				FlagMultiTable = P6;
				break;
			}
		}
		break;
	////////////
	// Pentium 
	////////////
	case 5:
		switch( Model ){
		case 8:
			n = "Pentium MMX (P55)";
			r = "0.25";
			CacheL1I=16;
			CacheL1D=16;
			break;
		case 7:
			n = "Pentium (P54C)";
			r = "0.35";
			CacheL1I=8;
			CacheL1D=8;
			break;
		case 6:
			n = "Pentium OverDrive";
			CacheL1I=8;
			CacheL1D=8;
			break;
		case 5:
			n = "Pentium OverDrive";
			CacheL1I=8;
			CacheL1D=8;
			break;
		case 4:
			if( Type == 1 ){
				n = "Pentium MMX OverDrive";
			}else{
				n = "Pentium MMX (P55C)";
			}
			r = "0.35";
			CacheL1I=16;
			CacheL1D=16;
			break;
		case 3:
			n = "Pentium OverDrive (P24T)";
			r = "0.35";
			CacheL1I=8;
			CacheL1D=8;
			break;
		case 2:
			if( Type == 1){
				n = "Pentium OverDrive";
				r = "0.35";
			}else{
				n = "Pentium (P54C)";
				r = "0.50/0.35";
			}
			CacheL1I=8;
			CacheL1D=8;
			break;
		case 1:
		case 0:
			if( Type == 1 ){
				n = "Pentium OverDrive";
			}else{
				n = "Pentium (P5)";
			}
			r = "0.80";
			CacheL1I=8;
			CacheL1D=8;
			break;
		default:
			n = "Pentium";
			break;		
		}
		break;
	////////
	// 486
	////////
	case 4:
		switch( Model ){
		case 9:
			n = "486DX4 WB";
			CacheL1U = 8;
			break;
		case 8:
			n = "486DX4";
			CacheL1U=8;
			break;
		case 7:
			n = "486DX2 WB";
			CacheL1U=8;
			break;
		case 5:
			n = "486SX2";
			CacheL1U=8;
			break;
		case 4:
			n = "486SL";
			CacheL1U=8;
			break;
		case 3:
			n = "486DX2";
			CacheL1U=8;
			break;
		case 2:
			n = "486SX";
			CacheL1U=8;
			break;
		case 1:
		case 0:
			n = "486DX";
			CacheL1U=8;
			break;
		default:
			n = "486";
			break;
		}
		break;
	case 3:
		n = "386";
		break;
	default:
		n = "Unknown CPU";
		break;
	}
	strcpy(NameSysInfo,n);
	strcpy(PlatformName,p);
	strcpy(ProcessRule,r);
}

void CCpuInfo::SetCPUNameAMD()
{
	int F = Family,M = Model,S = Stepping;
	char *n, *p, *r;
	n = "";
	p = "";
	r = "";		
	
	int FlagSempron19 = ( FeatureEx >> 19 ) & 0x1;
	SetCoreRevisionAMD();

	if(ExFamilyX == 0x10)
	{
		r = "0.065";

		FlagClockModulation = TRUE;
		FlagMultiTable = K10;

		if(strstr(NameString, "Opteron")){		
			if(strstr(NameString, "23") || strstr(NameString, "83")){
				strcpy(NameSysInfo, "Opteron");
				strcat(NameSysInfo, " (Barcelona)");
				wsprintf(PlatformName, "Socket F/F+");
			}else{
				strcpy(NameSysInfo, "Opteron");
				strcat(NameSysInfo, " (Budapest)");
				wsprintf(PlatformName, "Socket AM2/AM2+");
			}
		}else{
			wsprintf(PlatformName, "Socket AM2/AM2+");

			if(CacheL3 > 0 && PhysicalCoreNum == 4 && Number == 8){
				strcpy(NameSysInfo, "Phenom FX");
				strcat(NameSysInfo, " (Agena FX)");
				wsprintf(PlatformName, "Socket F/F+");
			}
			/*else if(CacheL3 > 0 && PhysicalCoreNum == 4){
				strcpy(NameSysInfo, "Phenom FX");
				strcat(NameSysInfo, " (Agena FX)");
			}
			*/
			else if(CacheL3 > 0 && PhysicalCoreNum == 4){
				strcpy(NameSysInfo, "Phenom X4");
				strcat(NameSysInfo, " (Agena)");
			}else if(CacheL3 > 0 && PhysicalCoreNum == 3){
				strcpy(NameSysInfo, "Phenom X3");
				strcat(NameSysInfo, " (Toliman)");
			}else if(CacheL3 > 0 && PhysicalCoreNum == 2){
				strcpy(NameSysInfo, "Athlon");
				strcat(NameSysInfo, " (Kuma)");
			}else if(PhysicalCoreNum == 2){
				strcpy(NameSysInfo, "Athlon X2");
				strcat(NameSysInfo, " (Rana)");
			}else{
				strcpy(NameSysInfo, "Sempron");
				strcat(NameSysInfo, " (Spica)");
			}
		}
		strcpy(ProcessRule, r);
		return;
	}
	else
	{

	switch( FamilyX ){
	///////////
	// Hammer
	///////////
	case 0xF:
		SetModelNumberK8();
		SetSocketK8();

		if( ModelEx >= 6 ){
			r = "0.065";
		}else if( ModelEx > 0 ){
			r = "0.09";
		}else{
			r = "0.13";
		}

		if(ModelEx <= 1){
			FlagK8Under1100V = TRUE;
		}
	
		if( FlagAMDMobile ){
			FlagClockModulation = TRUE;
			FlagMultiTable = MOBILE_ATHLON_64;
			FlagLH = LOVE_HAMMER_K8;
		}else{
			FlagMultiTable = ATHLON_64;
		}

		if(ModelEx > 6){
			strcpy(NameSysInfo, "Hammer Family");
		}else if(ModelEx == 6){
			if(SocketID == 0x0){ // Socket S1g1
				switch(BrandID)
				{
				case 0x2:
					strcpy(NameSysInfo, "Turion 64 X2");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Tyler)");
					FlagK8Under1100V = TRUE;
					break;
				case 0x3:
					strcpy(NameSysInfo, "Athlon 64 X2");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Tyler)");
					FlagK8Under1100V = TRUE;
					break;
				default:
					strcpy(NameSysInfo, "Unknown");
					break;
				}
			}else if(SocketID == 0x1){ // Socket F
				switch(BrandID)
				{
				case 0x1:
					strcpy(NameSysInfo, "Opteron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Hound)");
					break;
				case 0x4:
					strcpy(NameSysInfo, "Opteron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Hound)");
					break;
				default:
					strcpy(NameSysInfo, "Unknown");
					break;
				}
			}else{ //  if(SocketID == 0x3) Socket AM2
				switch(BrandID)
				{
				case 0x1:
					strcpy(NameSysInfo, "Opteron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Hound)");
					break;
				case 0x3:
					if(PhysicalCoreNum == 2){
						strcpy(NameSysInfo, "Athlon X2");
					//	strcat(NameSysInfo, ModelNumber);
						strcat(NameSysInfo, " (Brisbane)");
					}
					break;
				case 0x4:
					if(PhysicalCoreNum == 2){
						strcpy(NameSysInfo, "Athlon 64 X2");
						strcat(NameSysInfo, ModelNumber);
						strcat(NameSysInfo, " (Brisbane)");
					}else if(PhysicalCoreNum == 1){
						strcpy(NameSysInfo, "Athlon 64");
						strcat(NameSysInfo, ModelNumber);
						strcat(NameSysInfo, " (Lima)");
					}
					break;
				case 0x5:
					strcpy(NameSysInfo, "Athlon 64 FX");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Brisbane)");
					break;
				case 0x6:
					strcpy(NameSysInfo, "Sempron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Sparta)");
					break;
				case 0x7:
					strcpy(NameSysInfo, "Athlon X2");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Brisbane)");
					break;
				default:
					strcpy(NameSysInfo, "Unknown");
					break;
				}
			}
		}else if(ModelEx >=4){
			if(SocketID == 0x0){ // Socket S1g1
				switch(BrandID)
				{
				case 0x2:
					if(PhysicalCoreNum == 2){
						strcpy(NameSysInfo, "Turion 64 X2");
						strcat(NameSysInfo, ModelNumber);
						if(CacheL2 == 512){
							strcat(NameSysInfo, " (Trinidad)");
						}else{
							strcat(NameSysInfo, " (Taylor)");
						}
						FlagK8Under1100V = TRUE;
						break;
					}else{
						strcpy(NameSysInfo, "Turion 64");
						strcat(NameSysInfo, " (Richmond)");
						FlagK8Under1100V = TRUE;
						break;
					}
				case 0x3:
					strcpy(NameSysInfo, "Mobile Sempron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Keene)");
					FlagK8Under1100V = TRUE;
					break;
				default:
					strcpy(NameSysInfo, "Engineering Sample");
					break;
				}
			}else if(SocketID == 0x1){ // Socket F
				switch(BrandID)
				{
				case 0x1: // 2xxx
					strcpy(NameSysInfo, "Opteron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Santa Rosa)");
					break;
				case 0x4: // 8xxx
					strcpy(NameSysInfo, "Opteron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Santa Rosa)");
					break;
				default:
					strcpy(NameSysInfo, "Engineering Sample");
					break;
				}
			}else{ //  if(SocketID == 0x3) Socket AM2
				switch(BrandID)
				{
				case 0x1: // 1xxx
					strcpy(NameSysInfo, "Opteron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Santa Ana)");
					break;
				case 0x4:
					if(PhysicalCoreNum == 2){
						strcpy(NameSysInfo, "Athlon 64 X2");
						strcat(NameSysInfo, ModelNumber);
						strcat(NameSysInfo, " (Windsor)");
					}else if(PhysicalCoreNum == 1){
						strcpy(NameSysInfo, "Athlon 64");
						strcat(NameSysInfo, ModelNumber);
						strcat(NameSysInfo, " (Orleans)");
					}
					break;
				case 0x5:
					strcpy(NameSysInfo, "Athlon 64 FX");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Windsor)");
					break;
				case 0x6:
					strcpy(NameSysInfo, "Sempron");
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " (Manila)");
					break;
				default:
					strcpy(NameSysInfo, "Engineering Sample");
					break;
				}
			}
		}else{ // ModelEx <= 3
			switch(BrandID){
			case 0:
				strcpy(NameSysInfo, "Hammer Engineering Sample");
				break;
			case 0x4:
			case 0x18:
				strcpy(NameSysInfo, "Athlon 64");
				strcat(NameSysInfo, ModelNumber);
				if( CacheL2 == 512 ){
					if( ModelEx == 3 ){
						// Unknown
					}else if( ModelEx == 2 ){
						strcat(NameSysInfo, " (Venice)");
					}else if( ModelEx == 1 ){
						strcat(NameSysInfo, " (Winchester)");
					}else if( Model == 4 || Model == 7 ){
						strcat(NameSysInfo, " (ClawHammer)");
					}else if( Model == 8 || Model == 0xB || Model == 0xC
							|| Model == 0xE || Model == 0xF){
						strcat(NameSysInfo, " (NewCastle)");
					}
				}else{
					if( ModelEx > 0 ){
						strcat(NameSysInfo, " (San Diego)");
					}else{
						if( Model == 4 || Model == 0xC || Model == 0xE ){
							strcat(NameSysInfo, " (ClawHammer)");
						}else{
							strcat(NameSysInfo, " (SledgeHammer)");
						}
					}
				}
				break;
			case 0x5:
				strcpy(NameSysInfo, "Athlon 64 X2");
				strcat(NameSysInfo, ModelNumber);
				if(CacheL2 == 512 && ExModelX == 0x23){
					strcat(NameSysInfo, " (Toledo-512K)");
				}else if( CacheL2 == 512 ){
					strcat(NameSysInfo, " (Manchester)");
				}else{
					strcat(NameSysInfo, " (Toledo)");
				}
				break;
			case 0x6:
				strcpy(NameSysInfo, "Athlon 64 FX");
				strcat(NameSysInfo, ModelNumber);
				strcat(NameSysInfo, " (Toledo)");
				break;
			case 0x8:
			case 0x9:
			case 0xA:
			case 0xB:
				FlagK8Under1100V = TRUE;
				strcpy(NameSysInfo, "Mobile Athlon 64"); 
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x8:
					if( ModelEx > 0 ){
						strcat(NameSysInfo, " (Newark)");
					}else if( Model == 4 ){
						strcat(NameSysInfo, " (ClawHammer)");
					}else{
						strcat(NameSysInfo, " (Odessa)");
					}
					break;
				case 0x9:
					strcat(NameSysInfo, " [LV]");
					if( ModelEx > 0 && CacheL2 == 1024 ){
						strcat(NameSysInfo, " (Lancaster)");
					}else if( ModelEx > 0 ){
						strcat(NameSysInfo,  " (Oakville)");
					}else if( Model == 4 ){
						strcat(NameSysInfo, " (ClawHammer)");
					}else{
						strcat(NameSysInfo, " (Odessa)");
					}
					break;
				case 0xA:
					strcpy(NameSysInfo, "Turion 64 ML");
					strcat(NameSysInfo, ModelNumber);
					if(FlagAmdV){
						strcat(NameSysInfo, " (Richmond)");
					}else{
						strcat(NameSysInfo, " (Lancaster)");
					}
					break;
				case 0xB:
					strcpy(NameSysInfo, "Turion 64 MT"); 
					strcat(NameSysInfo, ModelNumber);
					if(FlagAmdV){
						strcat(NameSysInfo, " (Richmond)");
					}else{
						strcat(NameSysInfo, " (Lancaster)");
					}
					break;
				default:
					break;
				}
				break;
			case 0xC:
			case 0xD:
			case 0xE:
			case 0xF:
				strcpy(NameSysInfo, "Opteron UP");
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0xE:	strcat(NameSysInfo, " HE");	break;
				case 0xF:	strcat(NameSysInfo, " EE");	break;
				default:	break;
				}
				if( ModelEx >= 1 ){
					strcat(NameSysInfo, " (Venus)");
				}else if( ModelEx == 0 ){
					strcat(NameSysInfo, " (SledgeHammer)");
				}
				break;
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
				strcpy(NameSysInfo, "Opteron DP"); 
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x12:	strcat(NameSysInfo, " HE");	break;
				case 0x13:	strcat(NameSysInfo, " EE");	break;
				default:	break;
				}
				if( ModelEx >= 1 ){
					strcat(NameSysInfo, " (Troy)");
				}else if( ModelEx == 0 ){
					strcat(NameSysInfo, " (SledgeHammer)");
				}
				break;
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
				strcpy(NameSysInfo, "Opteron MP"); 
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x16:	strcat(NameSysInfo, " HE");	break;
				case 0x17:	strcat(NameSysInfo, " EE");	break;
				default:	break;
				}
				if( ModelEx >= 1 ){
					strcat(NameSysInfo, " (Athens)");
				}else if( ModelEx == 0 ){
					strcat(NameSysInfo, " (SledgeHammer)");
				}
				break;
			case 0x1D:
			case 0x1E:
				FlagK8Under1100V = TRUE;
				strcpy(NameSysInfo, "Mobile Athlon XP-M"); 
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x1D:
					strcat(NameSysInfo, " (Dublin)");
					break;
				case 0x1E:
					strcat(NameSysInfo, " [LV]");
					strcat(NameSysInfo, " (Dublin)");
					break;
				default:
					break;
				}

				break;
			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
				switch (BrandID){
				case 0x20:
					strcpy(NameSysInfo, "Athlon XP"); 
					strcat(NameSysInfo, ModelNumber);
					if( ModelEx > 0 ){
						strcat(NameSysInfo, " (Palermo)");
					}else{
						strcat(NameSysInfo, " (Paris)");
					}
					break;
				case 0x21:
					strcpy(NameSysInfo, "Mobile Sempron"); 
					strcat(NameSysInfo, ModelNumber);
					if( ModelEx == 2 ){
						strcat(NameSysInfo, " (Albany)");
					}else if( ModelEx == 1 ){
						strcat(NameSysInfo, " (Georgetown)");
					}else{
						strcat(NameSysInfo, " (Dublin)");
					}
					break;
				case 0x22:
					strcpy(NameSysInfo, "Sempron");
					strcat(NameSysInfo, ModelNumber);
					switch(ExModelX){
					case 0xC:
						strcat(NameSysInfo, " (Paris)");
						break;
					case 0x1F:
						if(CacheL2 == 128){
							strcat(NameSysInfo, " (Winchester-128K)");
						}else if(CacheL2 == 256){
							strcat(NameSysInfo, " (Winchester-256K)");
						}else{
							strcat(NameSysInfo, " (Winchester)");
						}
						break;
					case 0x1C:
					case 0x2C:
						strcat(NameSysInfo, " (Palermo)");
						break;
					default:
						break;
					}
					break;
				case 0x23:
					strcpy(NameSysInfo, "Mobile Sempron"); 
					strcat(NameSysInfo, ModelNumber);
					strcat(NameSysInfo, " [LV]");
					if( ModelEx == 2 ){
						strcat(NameSysInfo, " (Roma)");
					}else if( ModelEx == 1 ){
						strcat(NameSysInfo, " (Sonora)");
					}else{
						strcat(NameSysInfo, " (Dublin)");
					}
					break;
				default:
					break;
				}
				break;
			case 0x24:
				strcpy(NameSysInfo, "Athlon 64 FX");
				strcat(NameSysInfo, ModelNumber);
				if( ModelEx > 0 ){
					strcat(NameSysInfo, " (San Diego)");
				}else if( Model == 7 ){
					strcat(NameSysInfo, " (SledgeHammer)");
				}else{
					strcat(NameSysInfo, " (SledgeHammer)");
				}
				break;
			case 0x26:
				strcpy(NameSysInfo, "Sempron");
				strcat(NameSysInfo, ModelNumber);
				if( ModelEx > 0 ){
					if(FlagAA64){
						strcat(NameSysInfo, " (Palermo)");
					}else if(FlagSSE3){
						strcat(NameSysInfo, " (Palermo)");
					}else{
						strcat(NameSysInfo, " (Victoria)");
					}
				}else{
					strcat(NameSysInfo, " (Paris)");
				}
				break;
			case 0x29:
			case 0x2C:
			case 0x2D:
			case 0x2E:
			case 0x2F:
			case 0x38:
				strcpy(NameSysInfo, "Dual Core Opteron");
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x29:	strcat(NameSysInfo, " SE");	break;
				case 0x2E:	strcat(NameSysInfo, " HE");	break;
				case 0x2F:	strcat(NameSysInfo, " EE");	break;
				default:	break;
				}			
				strcat(NameSysInfo, " (Denmark)");
				break;
			case 0x2A:
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x39:
				strcpy(NameSysInfo, "Dual Core Opteron");
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x2A:	strcat(NameSysInfo, " SE");	break;
				case 0x32:	strcat(NameSysInfo, " HE");	break;
				case 0x33:	strcat(NameSysInfo, " EE");	break;
				default:	break;
				}
				strcat(NameSysInfo, " (Italy)");
				break;
			case 0x2B:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x3A:
				strcpy(NameSysInfo, "Dual Core Opteron"); 
				strcat(NameSysInfo, ModelNumber);
				switch (BrandID){
				case 0x2B:	strcat(NameSysInfo, " SE");	break;
				case 0x36:	strcat(NameSysInfo, " HE");	break;
				case 0x37:	strcat(NameSysInfo, " EE");	break;
				default:	break;
				}		
				strcat(NameSysInfo, " (Egypt)");
				break;
			default:
				strcpy(NameSysInfo, "Hammer (K8)");
				break;
			}
		}
		strcpy(ProcessRule, r);
		return;
		break;

#ifndef _X86_64

	//////////////
	// Athlon K7
	//////////////
	case 6:
		FlagAMDMP =	( strstr(NameString,"MP") != NULL );
		if( Number >= 2 ){
			FlagAMDMP = TRUE;
		}

		switch( Model ){

		case 0xA:
			p = "Socket A";
			r = "0.13";		
			if( FlagAMDMobile ){
				FlagLH = LOVE_HAMMER_K7;
				if( CacheL2 == 512 ){
					n = "Mobile Athlon XP-M (Barton)";
				}else{
					n = "Mobile Athlon XP-M (Thorton)";
				}
				FlagClockModulation = TRUE;
				FlagMultiTable = MOBILE_ATHLON;
			}else{
				if( CacheL2 == 512 && FlagAMDMP ){
					n = "Athlon MP (Barton)";
				}else if( FlagSempron19 && CacheL2 == 512 && Multiplier == 12.0 ){
					n = "Sempron (Barton)"; // Sempron 3000+
					FlagK7Sempron = TRUE;
				}else if( FlagSempron19 && CacheL2 == 256 && Multiplier <= 12.0 ){
					n = "Sempron (Thorton)"; // ???
					FlagK7Sempron = TRUE;
				}else if( CacheL2 == 512 ){
					n = "Athlon XP (Barton)";
				}else if( CacheL2 == 256 ){
					n = "Athlon XP (Thorton)";
				}else{
					n = "Athlon XP/MP (Barton)";
				}
				FlagMultiTable = ATHLON;
			}
			break;
		case 8:
			p = "Socket A";
			r = "0.13";			
			if( FlagAMDMobile ){
				FlagLH = LOVE_HAMMER_K7;
				if( CacheL2 == 64 ){
					n = "Mobile Duron (Applebred)";
				}else{
					n = "Mobile Athlon XP-M (Thoroughbred)";
				}
				FlagClockModulation = TRUE;
				FlagMultiTable = MOBILE_ATHLON;
			}else{
				if( CacheL2 == 256 && FlagAMDMP ){
					n = "Athlon MP (Thoroughbred)";
				}else if( FlagSempron19 && CacheL2 == 256 && Multiplier <= 12.0 ){ // && SystemClock >= 166.6 * 0.95
					n = "Sempron (Thoroughbred)";
					FlagK7Sempron = TRUE;
				}else if( CacheL2 == 256 ){
					n = "Athlon XP (Thoroughbred)";
				}else if( CacheL2 == 64 ){
					n = "Duron (Applebred)";
				}else{
					n = "Athlon XP/MP (Thoroughbred)";
				}
				FlagMultiTable = ATHLON;
			}
			break;
		case 7:
			p = "Socket A";
			r = "0.18";			
			if( FlagAMDMobile ){
				FlagLH = LOVE_HAMMER_K7;
				n = "Mobile Duron (Morgan)";
				FlagClockModulation = TRUE;
				FlagMultiTable = MOBILE_ATHLON;
			}else{
				n = "Duron (Morgan)";
				FlagMultiTable = ATHLON;
			}
			break;
		case 6:
			p = "Socket A";
			r = "0.18";			
			if( FlagAMDMobile ){
				FlagLH = LOVE_HAMMER_K7;
				if( CacheL2 == 64 ){
					n = "Mobile Duron (Palomino)";
				}else{
					n = "Mobile Athlon 4 (Palomino)";
				}
				FlagClockModulation = TRUE;
				FlagMultiTable = MOBILE_ATHLON;
			}else{
				if( CacheL2 == 256 && FlagAMDMP ){
					n = "Athlon MP (Palomino)";
				}else if( CacheL2 == 256 ){
					n = "Athlon XP (Palomino)";
				}else if( CacheL2 == 64 ){
					n = "Duron (Morgan)";
				}else{
					n = "Athlon XP/MP (Palomino)";
				}
				FlagMultiTable = ATHLON;
			}
			break;
		case 4:
			n = "Athlon (Thunderbird)";
			p = "Slot A / Socket A";
			r = "0.18";
			FlagMultiTable = ATHLON;
			break;
		case 3:
			n = "Duron (Spitfire)";
			p = "Socket A";
			r = "0.18";
			FlagMultiTable = ATHLON;
			break;
		case 2:
			n = "Athlon (K75)";
			p = "Slot A";
			r = "0.18";
			FlagMultiTable = ATHLON;
			break;
		case 1:
		case 0:
			n = "Athlon (K7)";
			p = "Slot A";
			r = "0.25";
			FlagMultiTable = ATHLON;
			break;
		default:
			n = "Unknown CPU";
			break;
		}
		break;
	///////
	// K6
	///////
	case 5:
		switch( Model ){
		case 0xD:
		case 0xC:
			p = "Socket 7 / Mobile";
			r = "0.18";
			FlagMultiTable = K6;
			FlagLH = LOVE_HAMMER_K6;
			if( CacheL2 == 256 ){
				n = "K6-III+";
			}else{
				n = "K6-2+";
			}
			break;
		case 0xA:
			n = "Geode LX";
			p = "481-terminal PBGA";
			r = "0.13";
			FlagLH = LOVE_HAMMER_GEODE_LX;
			FlagMultiTable = K6;
			break;
		case 9:
			n = "K6-III (Sharptooth)";
			p = "Socket 7";
			r = "0.25";
			FlagMultiTable = K6;
			break;
		case 8:
			n = "K6-2 (K6-3D)";
			p = "Socket 7";
			r = "0.25";
			FlagMultiTable = K6;
			break;
		case 7:
			n = "K6";
			p = "Socket 7";
			r = "0.25";
			break;
		case 6:
			n = "K6";
			p = "Socket 7";
			r = "0.30";
			break;
		case 3:
		case 2:
		case 1:
			n = "K5";
			p = "Socket 5/7";
			r = "0.35";
			CacheL1I = 16;
			CacheL1D = 8;
			break;
		case 0:
			n = "K5";
			p = "Socket 5/7";
			r = "0.35/0.50";
			CacheL1I = 16;
			CacheL1D = 8;
			break;
		default:
			n = "Unknown CPU";
			break;
		}
		break;
	case 4:
		switch( Model ){
		case 0xF:
			n = "Am5x86";
			p = "Socket 3";
			CacheL1U = 16;
			break;
		case 9:
			n = "Am5x86";
			p = "Socket 3";
			CacheL1U = 16;
			break;
		case 8:
			n = "Am5x86";
			p = "Socket 3";
			CacheL1U = 16;
			break;
		case 7:
			n = "Am486DX2";
			p = "Socket 3";
			CacheL1U = 8;
			break;
		case 3:
			n = "Am486DX2";
			p = "Socket 3";
			CacheL1U=8;
			break;
		default:
			n = "Unknown CPU";
			break;
		}
		break;

#endif // _X86_64

	default:
		n = "Unknown CPU";
		break;
	}
	}

	strcpy(NameSysInfo,n);
	strcpy(PlatformName,p);
	strcpy(ProcessRule,r);
}

void CCpuInfo::SetCPUNameTMx86()
{
#ifndef _X86_64
	int F = Family,M = Model,S = Stepping;
	char *n,*p,*r;
	n = "";
//	p = "";
	r = "";
	p = "Mobile Module";
	FlagClockModulation = TRUE;
	FlagLongRun = TRUE;

	if( F == 0xF && M == 2 ){
		r = "0.13";
		if( strstr(NameString,"8800") != NULL ){
			n = "Efficeon TM8800";
			r = "0.09";
		}else if( strstr(NameString,"8600") != NULL ){
			n = "Efficeon TM8600 (Astro)";
		}else if( strstr(NameString,"8500") != NULL ){
			n = "Efficeon TM8500 (Astro)";
		}else if( strstr(NameString,"8300") != NULL ){
			n = "Efficeon TM8300 (Astro)";
		}else if( strstr(NameString,"8820") != NULL ){
			n = "Efficeon TM8820 (Astro)";
		}else if( strstr(NameString,"8620") != NULL ){
			n = "Efficeon TM8620 (Astro)";
		}else{
			n = "Efficeon (Astro)";
		}
	}else if( F == 5 && M == 4 ){
		if( strstr(NameString,"3120") != NULL ){
			n = "Crusoe TM3120";
			r = "0.22";
			FlagClockModulation = FALSE;
			FlagLongRun = FALSE;
		}else if( strstr(NameString,"3200") != NULL ){
			n = "Crusoe TM3200";
			r = "0.22";
			FlagClockModulation = FALSE;
			FlagLongRun = FALSE;
		}else if( strstr(NameString,"5400") != NULL ){
			n = "Crusoe TM5400";
			r = "0.18";
		}else if( strstr(NameString,"5600") != NULL ){
			n = "Crusoe TM5600";
			r = "0.18";
		}else if( strstr(NameString,"5500") != NULL ){
			n = "Crusoe TM5500";
			r = "0.13";
		}else if( strstr(NameString,"5800") != NULL ){
			n = "Crusoe TM5800";
			r = "0.13";
		}else if( strstr(NameString,"5700") != NULL ){
			n = "Crusoe TM5700";
			r = "0.13";
		}else if( strstr(NameString,"5900") != NULL ){
			n = "Crusoe TM5900";
			r = "0.13";
		}else{
			n = "Unknown CPU";
		}
	}else{
		n = "Unknown CPU";
	}

	strcpy(NameSysInfo,n);
	strcpy(PlatformName,p);
	strcpy(ProcessRule,r);
#endif // _X86_64
}


void CCpuInfo::SetCPUNameVIA()
{
#ifndef _X86_64
	int F = Family,M = Model,S = Stepping;
	char *n,*p,*r;
	n = "";
	p = "";
	r = "";

	BOOL VIA = FALSE;
	
	SetCoreRevisionVIA();

	if(F == 6 && (M == 0xA || M == 0xD)){
		n = "C7 (Esther)";
		p = "NanoBGA 479";
		r = "0.09";
		CacheL2 = 128;
		FlagMultiTable = ESTHER;
//		FlagLH = LONG_HAUL_LEVEL_2;
		VIA = TRUE;
	}else if(F == 6 && M == 9){
		n = "C3 (Nehemiah)";
		p = "Socket 370";
		r = "0.13";
		FlagMultiTable = NEHEMIAH;
		FlagLH = LONG_HAUL_LEVEL_2;
		VIA = TRUE;
	}else if(F == 6 && M == 8){
		n = "C3 (Ezra-T)";
		p = "Socket 370";
		r = "0.13";
		FlagMultiTable = EZRA_T;
		FlagLH = LONG_HAUL_LEVEL_2;
		VIA = TRUE;
	}else if(F == 6 && M == 7 && S >= 8){
		n = "C3 (Ezra)";
		p = "Socket 370";
		r = "0.13";
		FlagMultiTable = EZRA;
		FlagLH = LONG_HAUL_LEVEL_1;
		VIA = TRUE;
	}else if(F == 6 && M == 7 && S < 8){
		n = "C3 (Samuel 2)";
		p = "Socket 370";
		r = "0.15";
		FlagLH = LONG_HAUL_LEVEL_1;
		if( S == 0 ){
			FlagMultiTable = SAMUEL;
		}else{
			FlagMultiTable = SAMUEL2;
		}
		VIA = TRUE;
	}else if(F == 6 && M == 6){
		n = "Cyrix III (Samuel)";
		p = "Socket 370";
		r = "0.18";
		FlagMultiTable = SAMUEL; // == CYRIX3
		FlagLH = LONG_HAUL_LEVEL_1;
		VIA = TRUE;
	}else if(F == 6 && M == 5 && S == 1){
		n = "6x86MX (M II)";
		p = "Socket 7";
		r = "0.18/0.25";
		FlagMultiTable = CYRIX3;
		VIA = TRUE;
	}else if(F == 6 && M == 5){
		n = "Cyrix III (Joshua)";
		p = "Socket 370";
		r = "0.18";
		FlagMultiTable = CYRIX3;
		VIA = TRUE;
	}else if(F == 6 && M == 0 && S == 0){
		n = "6x86MX (M II)";
		p = "Socket 7";
		r = "0.35";
		CacheL1U = 64;
	}else if(F == 6 && M == 0){
		n = "6x86MX (M II)";
		p = "Socket 7";
		r = "0.30";
		CacheL1U = 64;
	}
	
	else if(F == 5 && M == 9 && FlagBrand == IDT ){
		n = "WinChip 3";
		p = "Socket 7";
		r = "0.25";
	}else if(F == 5 && M == 8 && FlagBrand == IDT ){
		n = "WinChip 2";
		p = "Socket 7";
		r = "0.35";
	}else if(F == 5 && M == 4 && FlagBrand == IDT ){
		n = "WinChip C6";
		p = "Socket 7";
		CacheL1I=32;
		r = "0.35";
		CacheL1D=32;
	}else if(F == 5 && M == 9 && FlagBrand == CYRIX ){
		n = "Geode";
	}else if(F == 5 && M == 4 && FlagBrand == CYRIX ){
		n = "MediaGX";
		CacheL1U=16;
	}else if(F == 0x5 && M == 0x2 && FlagBrand == CYRIX ){
		n = "6x86 (M I)";
		p = "Socket 5/7";
		r = "0.35/0.65";
		CacheL1U=16;
	}else{ 
		n = "Unknown CPU";
	}

	strcpy(NameSysInfo,n);
	strcpy(PlatformName,p);
	strcpy(ProcessRule,r);

	if( VIA ){
		strcpy(VendorName,"VIA");
	}
#endif // _X86_64
}

void CCpuInfo::SetCPUNameOthers()
{
#ifndef _X86_64

	int F = Family,M = Model,S = Stepping;
	char *n,*p,*r;
	n = "";
	p = "";
	r = "";

	switch( FlagBrand ){
	////////
	// SiS
	////////
	case SIS:
		if( F == 5 ){
			n = "SiS 55x";
		}else{
			n = "Unknown CPU";
		}
		break;
	////////
	// NSC
	////////
	case NSC:
		n = "Geode";
		break;
	///////////
	// NexGen
	///////////
	case NEXGEN:
		if( F == 5 && M == 0 ){
			n = "Nx586";
			r = "0.44/0.50";
			CacheL1I=16;
			CacheL1D=16;
		}else{
			n = "Unknown CPU";
		}
		break;
	/////////
	// Rise
	/////////
	case RISE:
		switch( Family ){
		case 5:
			switch( Model ){
			case 9:
				n = "mP6 (iDragon II)";
				p = "Socket 7";
				r = "0.18";
				CacheL1I=8;
				CacheL1D=8;
				break;
			case 8:
				n = "mP6 (iDragon)";
				p = "Socket 7";
				r = "0.25";
				CacheL1I=8;
				CacheL1D=8;
				break;
			case 2:	
				n = "mP6 (iDragon)";
				p = "Socket 7";
				r = "0.18";
				CacheL1I=8;
				CacheL1D=8;
				break;
			case 0:
				n = "mP6 (iDragon)";
				p = "Socket 7";
				r = "0.25";
				CacheL1I=8;
				CacheL1D=8;
				break;
			default:
				n = "Unknown CPU";
				break;
			}
			break;
		default:
			n = "Unknown CPU";
			break;
		}
		break;
	default:
        if( strcmp(VendorString,"Compaq FX!32") == 0 ){
			n = "Alpha";
		}
		if( strcmp(VendorString,"ConnectixCPU") == 0 ){
			n = "PowerPC";
		}
#ifdef _WIN32		
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		switch( si.wProcessorArchitecture )
		{
		case PROCESSOR_ARCHITECTURE_INTEL:
			switch( si.dwProcessorType )
			{
			case PROCESSOR_INTEL_386:
				n = "386";
				break;
			case PROCESSOR_INTEL_486:
				n = "486";
				break;
			case PROCESSOR_INTEL_PENTIUM:
				n= "Pentium";
				break;
			}
			break;
		case PROCESSOR_ARCHITECTURE_MIPS:
			switch(si.wProcessorLevel){
			case 0004:
				n = "MIPS R4000";
				break;
			default:
				n = "MIPS";
				break;
			}
			break;
		case PROCESSOR_ARCHITECTURE_ALPHA:
			switch(si.wProcessorLevel){
			case 21064:	n = "Alpha 21064";	break;
			case 21066:	n = "Alpha 21066";	break;
			case 21164:	n = "Alpha 21164";	break;
			default:	n = "Alpha";		break;
			}
			break;
		case PROCESSOR_ARCHITECTURE_PPC:
			switch(si.wProcessorLevel){
			case 1:		n = "PowerPC 601";	break;
			case 3:		n = "PowerPC 603";	break;
			case 4:		n = "PowerPC 604";	break;
			case 6:		n = "PowerPC 603+";	break;
			case 9:		n = "PowerPC 604+";	break;
			case 20:	n = "PowerPC 620";	break;
			default:	n = "PowerPC";		break;
			}
			break;
		default:
			n = "Unknown CPU";
			break;
		}
#endif // _WIN32
		break;
	}
	strcpy(NameSysInfo,n);
	strcpy(PlatformName,p);
	strcpy(ProcessRule,r);
#endif // _X86_64

}


//////////////////////////////////////////////////////////////////////
// Fill CPU Information
//////////////////////////////////////////////////////////////////////
int CCpuInfo::FillCpuInfo()
{
	DWORD EAX, EBX, ECX, EDX;

// VendorName
	char vendor[13];
	vendor[12] = '\0';
	cpuid(0x0, &EAX, &EBX, &ECX, &EDX);

	MaxCPUID = EAX;
	memcpy(vendor  , &EBX, 4);
	memcpy(vendor+4, &EDX, 4);
	memcpy(vendor+8, &ECX, 4);
	sprintf(VendorString,vendor);

	if( strcmp(VendorString,"GenuineIntel") == 0 ){
		FlagBrand = INTEL;
		sprintf(VendorName,"Intel");
	}else if( strcmp(VendorString,"AuthenticAMD") == 0 ){
		FlagBrand = AMD;
		sprintf(VendorName,"AMD");
	}else if( strcmp(VendorString,"GenuineTMx86") == 0 ){
		FlagBrand = TMx86;
		sprintf(VendorName,"Transmeta");
	}else if( strcmp(VendorString,"CyrixInstead") == 0 ){
		FlagBrand = CYRIX;
		sprintf(VendorName,"Cyrix");
	}else if( strcmp(VendorString,"CentaurHauls") == 0 ){
		FlagBrand = IDT;
		sprintf(VendorName,"IDT");
	}else if( strcmp(VendorString,"RiseRiseRise") == 0 ){
		FlagBrand = RISE;
		sprintf(VendorName,"Rise");
	}else if( strcmp(VendorString,"NexGenDriven") == 0 ){
		FlagBrand = NEXGEN;
		sprintf(VendorName,"NexGen");
	}else if( strcmp(VendorString,"UMC UMC UMC ") == 0 ){
		FlagBrand = UMC;
		sprintf(VendorName,"UMC");
	}else if( strcmp(VendorString,"Geode By NSC") == 0 ){
		FlagBrand = NSC;
		sprintf(VendorName,"NSC");
	}
	
	else if( strcmp(VendorString,"Compaq FX32!") == 0 ){
		FlagBrand = COMPAQ;
		sprintf(VendorName,"Compaq");
	}

// Extended CPUID
	cpuid(0x80000000, &EAX, &EBX, &ECX, &EDX);
	MaxCPUIDEx = EAX;// FeatureEx

	if( MaxCPUIDEx > 0x80000000 ){
		cpuid(0x80000001, &EAX, &EBX, &ECX, &EDX);
		VersionEx = EAX;
		MiscInfoEx = EBX;
		FeatureEx = EDX;
		FeatureExEcx = ECX;
	}

	FlagMMXEx	= (FeatureEx >> 22) & 0x1;
	
	if( FlagBrand == INTEL ){
		FlagIA32e = (FeatureEx >> 29) & 0x1;
	}else{
		FlagAA64 = (FeatureEx >> 29) & 0x1;
	}
	Flag3DNow	= (FeatureEx >> 31) & 0x1;
	Flag3DNowEx = (FeatureEx >> 30) & 0x1;
	FlagNX		= (FeatureEx >> 20) & 0x1;
	FlagAmdV	= (FeatureExEcx >> 2) & 0x1;
	FlagSSE4A   = (FeatureExEcx >> 6) & 0x1;
	FlagSSE5   = (FeatureExEcx >> 11) & 0x1;
	FlagAVX   = (FeatureEcx >> 28) & 0x1;

// Family / Model / Step
	cpuid(0x1, &EAX, &EBX, &ECX, &EDX);

	Version		= EAX;
	MiscInfo	= EBX;
	Feature		= EDX;
	FeatureEcx	= ECX;
	Family		= (Version>>8) & 0xF;
	Model		= (Version>>4) & 0xF;
	Stepping	= Version & 0xF;
	Type		= (Version>>12) & 0x3;
	if(Family == 0xF || (Family == 6 && ModelX > 0xE)){
		Apic	= (MiscInfo>>24) & 0xFF;
	}

	switch( Type )
	{
	case 0:	strcpy(TypeName,"Original OEM processor");	break;
	case 1:	strcpy(TypeName,"OverDrive processor");		break;
	case 2:	strcpy(TypeName,"Dual processor");			break;
	default:strcpy(TypeName,"Reserved");				break;
	}

	FlagMSR  = (Feature>> 5) & 0x1;
	FlagMMX  = (Feature>>23) & 0x1;
	FlagSSE	 = (Feature>>25) & 0x1;
	FlagSSE2 = (Feature>>26) & 0x1;
	FlagSSE3 = (FeatureEcx ) & 0x1;
	FlagSSSE3= (FeatureEcx>>9) & 0x1;
	FlagSSE41= (FeatureEcx >> 19) & 0x1;
	FlagSSE42= (FeatureEcx >> 20) & 0x1;
	FlagVT   = (FeatureEcx>>5) & 0x1;
	FlagHT	 = (Feature>>28) & 0x1;
	FlagIA64 = (Feature>>30) & 0x1;

	if(FlagSSE41 || FlagSSE42){
		FlagSSE4 = TRUE;
	}

	if(FlagHT && FlagBrand == INTEL && Family == 0xF){ // EnableHyperThreading
		HyperThreadNum = (EBX>>16) & 0xFF;
	}else if(FlagHT && FlagBrand == INTEL){ // Conroe , Merom
		PhysicalCoreNum = (EBX>>16) & 0xFF;
		HyperThreadNum = 0;
	}else{
		HyperThreadNum = 0;
	}

	if(FlagBrand == AMD && MaxCPUIDEx >= 0x80000008){
		cpuid(0x80000008, &EAX, &EBX, &ECX, &EDX);
		PhysicalCoreNum = (ECX & 0xFF) + 1;
	}

	if(FlagBrand == INTEL && MaxCPUID >= 0x00000004){
		cpuid(0x00000004, &EAX, &EBX, &ECX, &EDX);
		PhysicalCoreNum = ((EAX >> 26) & 0x3F) + 1;
	}

	if(PhysicalCoreNum == 2){
		FlagDualCore = TRUE;
	}

	// for Pentium D, Pentium XE
	if(PhysicalCoreNum >= 2 && FlagBrand == INTEL && Family == 0xF){
		HyperThreadNum = HyperThreadNum / PhysicalCoreNum;
	}
	if(HyperThreadNum <= 1){FlagHT = FALSE;}

	FamilyEx = (Version>>20) & 0xFF;
	ModelEx = (Version>>16) & 0xF;
	FamilyX = FamilyEx + Family;
	ModelX = ModelEx * 16 + Model;

	if(FlagBrand == INTEL)
	{
		ExFamilyX = FamilyX;
		ExModelX = ModelX;
		ExStepping = Stepping;
	}

	if(FlagBrand != INTEL || Family == 0xF){
		FamilyEx = (VersionEx>>20) & 0xFF;
		ModelEx = (VersionEx>>16) & 0xF;
		ExFamily   = (VersionEx>>8) & 0xF;
		ExFamilyX = FamilyEx + ExFamily;
		ExModel    = (VersionEx>>4) & 0xF;
		ExModelX = ModelEx * 16 + ExModel;
		ExStepping = VersionEx & 0xF;
	}

	// for BrandID
	if( FlagBrand == AMD && Family == 0xF && ModelEx < 4){// for Old Hammer
		BrandID = (((MiscInfoEx >> 8) & 0xF) * 4) + ((MiscInfoEx >> 6) & 0x3);
		BrandIDNN = MiscInfoEx & 0x3F;
	}else if(FlagBrand == AMD && Family == 0xF && ModelEx >= 4){// for New Hammer
		BrandID = ((MiscInfoEx >> 9) & 0x1F);
		BrandIDNN = (((MiscInfoEx >> 15) & 0x1) << 6) + (MiscInfoEx & 0x1F);
		PwrLmt = (((MiscInfoEx >> 6) & 0x7) << 1) + ((MiscInfoEx >> 14) & 0x1);
		PlatformID = SocketID = (VersionEx >> 4) & 0x3;
		if(PhysicalCoreNum == 4){ // Unknown
			CmpCap = 0x2;// or 0x3
		}else if(PhysicalCoreNum == 2){ //
			CmpCap = 0x1;
		}else{
			CmpCap = 0x0;
		}
	}

	if( FlagBrand == AMD && Family == 0xF ){// for Hammer or later
		if( BrandID == 0 ){
			BrandID = ((MiscInfo >> 5) & 0x7) << 2;		// bit 5-7  => 12bit Brand ID
			if( BrandID == 0 ){
				BrandID = ((MiscInfo >> 8) & 0x7) << 2;	// bit 8-10 => 12bit Brand ID
			}
			BrandIDNN = MiscInfo & 0x1F;
		}
	}else{
		BrandID	= MiscInfo & 0xFF;
	}


// FSBMode //
	if(FlagBrand == INTEL && Family == 0xF && FamilyEx == 0x0){
		FSBMultiplier = 4;
		strcpy(FSBMode,"QDR");
	}else if(FlagBrand == INTEL && Family == 0x6 && (Model == 0x9 || ModelX >= 0xC)){//Pentium M & Intel Core
		FSBMultiplier = 4;
		strcpy(FSBMode,"QDR");
	}else if(FlagBrand == IDT && Family == 0x6 && Model == 0xA){//VIA C7
		FSBMultiplier = 4;
		strcpy(FSBMode,"QDR");
	}else if(FlagBrand == AMD && Family == 0xF){
//		FSBMultiplier = 0;
		strcpy(FSBMode,"HT");
	}else if(FlagBrand == AMD && Family == 0x5 && Model == 0xA){//Geode LX
		FSBMultiplier = 0;
		strcpy(FSBMode,"---");
	}else if(FlagBrand == AMD && Family >= 0x6){
		FSBMultiplier = 2;
		strcpy(FSBMode,"DDR");
	}else{
		FSBMultiplier = 1;
		strcpy(FSBMode,"SDR");
	}

	sprintf(FMS,"%X%X%X",Family, Model, Stepping);
	FlagRDTSC = (Feature>>4) & 0x1;
	FlagRDMSR = (Feature>>5) & 0x1;

	// Processor Serial Number
	if( MaxCPUID >= 3 && ((Feature>>18) & 0x1) ){
		cpuid(0x1, &EAX, &EBX, &ECX, &EDX);
		cpuid(0x3, &EBX, &EBX, &ECX, &EDX); // EBX is Dummy
		sprintf(ProcessorSerial,
			"%04X-%04X-%04X-%04X-%04X-%04X",
			(EAX >> 16), (EAX & 0xFFFF),
			(EDX >> 16), (EDX & 0xFFFF),
			(ECX >> 16), (ECX & 0xFFFF)
			);
		FlagProcessorSerial = TRUE;
	}

// Feature Transmeta
	cpuid(0x80860000, &EAX, &EBX, &ECX, &EDX);
	if( EAX > 0x80860000 ){
		cpuid(0x80860001, &EAX, &EBX, &ECX, &EDX);
		FeatureTransmeta = EDX;
	}

// Feature VIA (Centaur)
	cpuid(0xC0000000, &EAX, &EBX, &ECX, &EDX);
	if( EAX > 0xC0000000 ){
		cpuid(0xC0000001, &EAX, &EBX, &ECX, &EDX);
		FeatureVia = EDX;
	}

// Feature PowerManagement
	cpuid(0x80000000, &EAX, &EBX, &ECX, &EDX);
	if( EAX >= 0x80000007 ){
		cpuid(0x80000007, &EAX, &EBX, &ECX, &EDX);
		FeaturePM = EDX;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Check Power mangement
//////////////////////////////////////////////////////////////////////
void CCpuInfo::CheckPowerMangement()
{
	if( FlagBrand == INTEL )
	{
		// for Debug
		// FlagLH = SPEED_STEP;

		if( (FeatureEcx & 0x80) // EIST
		|| ( Family == 0x6 && Model == 0xB && BrandID == 0x6 ) // Mobile Pentium III-M (Tualatin)
		|| ( Family == 0x6 && Model == 0x8 && CacheL2 == 256 && PlatformID % 2 && Multiplier > 5.0 ) // Mobile Pentium III (Coppermine) 
		){
			FlagSpeedStep = TRUE;
		}

		if((FeatureEcx & 0x80) /*EIST*/
			&& Family == 0xF && Model >= 3) // Prescott or later
		{
			FlagLH = SPEED_STEP_P4;
			FlagEIST = TRUE;
			FlagEistCorrect = TRUE;
		}

		if(Family == 0x6 && (Model == 0x9 || Model >= 0xC))
		{
			FlagLH = SPEED_STEP_PM;
			FlagEIST = TRUE;
		}

		if(Family == 0x6 && ModelX >= 0xE) // CoreDuo/Core2Duo 65nm
		{
			FlagLH = SPEED_STEP_CORE_MA;
			FlagEistCorrect = TRUE;
		}

		if(Family == 0x6 && ModelX >= 0x10) // Core2Duo 45nm
		{
			FlagLH = SPEED_STEP_PENRYN;
			FlagEIST = TRUE;
		}
	}
	
	if( FlagBrand == AMD )
	{
		FlagPowerNow = CheckAMDMobile();
		if(ExFamilyX == 0x10){FlagPowerNow = TRUE;}
		if(Family == 0xF){
			if(ModelEx >= 3){
				DWORD EAX, EBX, ECX, EDX;
				cpuid(0x80000007, &EAX, &EBX, &ECX, &EDX);
				FlagK8100MHzSteps = (EDX >> 6) & 0x1;
			}else if(ModelEx == 2 && PhysicalCoreNum == 2){
				FlagK8100MHzSteps = FALSE;
			}else{
				FlagK8100MHzSteps = TRUE;
			}
		}
	}

	if( FlagBrand == IDT )
	{
		if( FlagLH > 0){
			FlagLongHaul = TRUE;
		}
	}

	if( FlagBrand == TMx86 ){
		// Setup in CCpuInfo::SetCPUNameTMx86();
	}
}
//////////////////////////////////////////////////////////////////////
// Get Frequency
//////////////////////////////////////////////////////////////////////
/*
void WaitThread(void *thread)
{
	while(thread){
		Sleep(1000);
	}
}
*/

double CCpuInfo::GetClock(DWORD TimerType)
{
	if( TimerType == 9 ){
		return 0.0;
	}

#ifdef _WIN32

#ifdef HIGH_PRIORITY
	SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
#endif

	DWORD StartWT,EndWT;					// GetTickCount
	DWORD StartMMT,EndMMT;					// timeGetTime
	DWORD StartH,StartL,EndH,EndL;	// QPC
	DWORD TickCount;
	ULARGE_INTEGER lRdtscS,lRdtscE;
	LARGE_INTEGER lPre,lStart,lEnd,lFrequency;

	volatile int i=0;

	// with load
	TickCount = GetTickCount();
	if( FlagClockModulation && TimerType != 4 /* QPF + No Load */ ){
		while(GetTickCount() - TickCount < 1000){
			i++;
		}
	}
#ifndef _X86_64
	_asm{
		rdtsc
		mov StartH , edx
		mov StartL , eax
	}
#else
	ULONG64 ul64;
	ul64 = __rdtsc();
	StartH = (DWORD)(ul64 >> 32);
	StartL = (DWORD)ul64;
#endif
	QueryPerformanceCounter(&lPre);
	QueryPerformanceCounter(&lStart);
	StartMMT = timeGetTime();
	StartWT  = GetTickCount();
	
	TickCount = GetTickCount();
	DWORD SleepTime = 1000;
	switch ( TimerType ){
		case 0: // QPF + MMT (default)
			SleepTime = 500;
			break;
		case 1: // WT
		case 2: // MMT
			SleepTime = 1000;
			break;
		case 3:
		case 4:
			SleepTime = 100;
			break;
		default:
			SleepTime = 1000;
			break;
	}
	while(GetTickCount() - TickCount < SleepTime ){
		i++;
		Sleep(0);
	}

	/*
	BOOL status = TRUE;
	HANDLE hThread = (unsigned long *)_beginthread(WaitThread,0,&status);
	WaitForSingleObject(hThread, 1000);
	status = FALSE;
	*/

#ifndef _X86_64
	_asm{
		rdtsc
		mov EndH , edx
		mov EndL , eax
	}
#else
	ul64 = __rdtsc();
	EndH = (DWORD)(ul64 >> 32);
	EndL = (DWORD)ul64;
#endif

	EndMMT = timeGetTime();
	EndWT  = GetTickCount();
	QueryPerformanceCounter(&lEnd);
	QueryPerformanceFrequency(&lFrequency);
	lRdtscS.HighPart = StartH;
	lRdtscS.LowPart  = StartL;
	lRdtscE.HighPart = EndH;
	lRdtscE.LowPart  = EndL;

	double totalTimeQPC = (double)((lEnd.QuadPart - lStart.QuadPart) - (lStart.QuadPart - lPre.QuadPart) ) / (double)(lFrequency.QuadPart);
	double totalTimeMMT = (double)(EndMMT - StartMMT) / 1000.0;
	double totalTimeWT  = (double)(EndWT - StartWT) / 1000.0;

	double ClockQPC = (__int64)(lRdtscE.QuadPart - lRdtscS.QuadPart) / totalTimeQPC /1000000.0;
	double ClockMMT = (__int64)(lRdtscE.QuadPart - lRdtscS.QuadPart) / totalTimeMMT /1000000.0;
	double ClockWT  = (__int64)(lRdtscE.QuadPart - lRdtscS.QuadPart) / totalTimeWT /1000000.0;

#ifdef HIGH_PRIORITY
	SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
#endif
	switch ( TimerType )
	{
	case 0:
#ifndef _X86_64
		if( ModeX86_64 ){
			return ClockMMT;
		}
#endif
		if( ClockMMT * ERROR_RATE_MIN < ClockQPC && ClockQPC < ClockMMT * ERROR_RATE_MAX ){
			strcpy(MeasureMode,"QPC");
			return ClockQPC;
		}else{
			strcpy(MeasureMode,"MMT");
			return ClockMMT;
		}
		break;
	case 1: // Windows Timer
		strcpy(MeasureMode,"WT");
		return ClockWT;
		break;
	case 2: // Multimedia Timer
		strcpy(MeasureMode,"MMT");
		return ClockMMT;
		break;
	case 3: // QPC
	case 4: // QPC + No Load
		strcpy(MeasureMode,"QPC");
		return ClockQPC;
		break;
	default:
		strcpy(MeasureMode,"WT");
		return ClockWT;
		break;
	}
#else
	unsigned long long int start,finish;
	start = rdtsc();
	sleep(1);
	finish = rdtsc();
	return (double)(finish - start);
#endif

}


double CCpuInfo::GetOriginalClockByNameString()
{
	double clock;

	if( strstr(NameString,"1166") != 0 || strstr(NameString,"1.16") != 0 ){clock = 1166.66;}// x 7
	else if( strstr(NameString,"1833") != 0 || strstr(NameString,"1.83") != 0 ){clock = 1833.33;}// x11
	else if( strstr(NameString,"2166") != 0 || strstr(NameString,"2.16") != 0 ){clock = 2166.66;}// x13

	else if( strstr(NameString," 133") != 0){clock = 133.33;}
	else if( strstr(NameString," 166") != 0){clock = 166.66;}
//	else if( strstr(NameString," 200") != 0){clock = 200.00;}
	else if( strstr(NameString," 233") != 0){clock = 233.33;}
	else if( strstr(NameString," 266") != 0){clock = 266.66;}
//	else if( strstr(NameString," 300") != 0){clock = 300.00;}
	else if( strstr(NameString," 333") != 0){clock = 333.33;}
	else if( strstr(NameString," 366") != 0){clock = 366.66;}
//	else if( strstr(NameString," 400") != 0){clock = 400.00;}
	else if( strstr(NameString," 433") != 0){clock = 433.00;}
	else if( strstr(NameString," 466") != 0){clock = 466.66;}
//	else if( strstr(NameString," 500") != 0){clock = 500.00;}
	else if( strstr(NameString," 533") != 0){clock = 533.33;}
	else if( strstr(NameString," 566") != 0){clock = 566.66;}
//	else if( strstr(NameString," 600") != 0){clock = 600.0;}
	else if( strstr(NameString," 666") != 0){clock = 666.66;}
	else if( strstr(NameString," 667") != 0){clock = 666.66;}
	else if( strstr(NameString," 733") != 0){clock = 733.33;}
//	else if( strstr(NameString," 800") != 0){clock = 800.0;}
	else if( strstr(NameString," 866") != 0){clock = 866.66;}
	else if( strstr(NameString," 933") != 0){clock = 933.33;}
//	else if( strstr(NameString,"1000") != 0 || strstr(NameString,"1.00") != 0 ){clock = 1000.0;}
	else if( strstr(NameString,"1066") != 0 || strstr(NameString,"1.06") != 0 ){clock = 1066.66;}
	else if( strstr(NameString,"1133") != 0 || strstr(NameString,"1.13") != 0 ){clock = 1133.33;}
//	else if( strstr(NameString,"1200") != 0 || strstr(NameString,"1.20") != 0 ){clock = 1200.0;}
	else if( strstr(NameString,"1266") != 0 || strstr(NameString,"1.26") != 0 ){clock = 1266.66;}
	else if( strstr(NameString,"1333") != 0 || strstr(NameString,"1.33") != 0 ){clock = 1333.33;}
//	else if( strstr(NameString,"1400") != 0 || strstr(NameString,"1.40") != 0 ){clock = 1400.0;}
	else if( strstr(NameString,"1466") != 0 || strstr(NameString,"1.46") != 0 ){clock = 1466.66;}
	else if( strstr(NameString,"1533") != 0 || strstr(NameString,"1.53") != 0 ){clock = 1533.33;}
//	else if( strstr(NameString,"1600") != 0 || strstr(NameString,"1.60") != 0 ){clock = 1600.0;}
	else if( strstr(NameString,"1666") != 0 || strstr(NameString,"1.66") != 0 ){clock = 1666.66;}
	else if( strstr(NameString,"1667") != 0 || strstr(NameString,"1.67") != 0 ){clock = 1666.66;}
	else if( strstr(NameString,"1733") != 0 || strstr(NameString,"1.73") != 0 ){clock = 1733.33;}
//	else if( strstr(NameString,"1800") != 0 || strstr(NameString,"1.80") != 0 ){clock = 1800.00;}
	else if( strstr(NameString,"1866") != 0 || strstr(NameString,"1.86") != 0 ){clock = 1866.66;}
	else if( strstr(NameString,"1933") != 0 || strstr(NameString,"1.93") != 0 ){clock = 1933.33;}
//	else if( strstr(NameString,"2000") != 0 || strstr(NameString,"2.00") != 0 ){clock = 2000.00;}
	else if( strstr(NameString,"2066") != 0 || strstr(NameString,"2.06") != 0 ){clock = 2066.66;}
	else if( strstr(NameString,"2133") != 0 || strstr(NameString,"2.13") != 0 ){clock = 2133.33;}
//	else if( strstr(NameString,"2200") != 0 || strstr(NameString,"2.20") != 0 ){clock = 2200.0;}
	else if( strstr(NameString,"2266") != 0 || strstr(NameString,"2.26") != 0 ){clock = 2266.66;}
	else if( strstr(NameString,"2333") != 0 || strstr(NameString,"2.33") != 0 ){clock = 2333.33;}
//	else if( strstr(NameString,"2400") != 0 || strstr(NameString,"2.40") != 0 ){clock = 2400.0;}
	else if( strstr(NameString,"2466") != 0 || strstr(NameString,"2.46") != 0 ){clock = 2466.66;}
	else if( strstr(NameString,"2533") != 0 || strstr(NameString,"2.53") != 0 ){clock = 2533.33;}
//	else if( strstr(NameString,"2600") != 0 || strstr(NameString,"2.60") != 0 ){clock = 2600.0;}
	else if( strstr(NameString,"2666") != 0 || strstr(NameString,"2.66") != 0 ){clock = 2666.66;}
	else if( strstr(NameString,"2667") != 0 || strstr(NameString,"2.67") != 0 ){clock = 2666.66;}
	else if( strstr(NameString,"2733") != 0 || strstr(NameString,"2.73") != 0 ){clock = 2733.33;}
//	else if( strstr(NameString,"2800") != 0 || strstr(NameString,"2.80") != 0 ){clock = 2800.0;}
	else if( strstr(NameString,"2866") != 0 || strstr(NameString,"2.86") != 0 ){clock = 2866.66;}
	else if( strstr(NameString,"2933") != 0 || strstr(NameString,"2.93") != 0 ){clock = 2933.33;}
//	else if( strstr(NameString,"3000") != 0 || strstr(NameString,"3.00") != 0 ){clock = 3000.0}
	else if( strstr(NameString,"3066") != 0 || strstr(NameString,"3.06") != 0 ){clock = 3066.66;}
	else if( strstr(NameString,"3133") != 0 || strstr(NameString,"3.13") != 0 ){clock = 3133.33;}
//	else if( strstr(NameString,"3200") != 0 || strstr(NameString,"3.20") != 0 ){clock = 3200.0;}
	else if( strstr(NameString,"3266") != 0 || strstr(NameString,"3.26") != 0 ){clock = 3266.66;}
	else if( strstr(NameString,"3333") != 0 || strstr(NameString,"3.33") != 0 ){clock = 3333.33;}
//	else if( strstr(NameString,"3400") != 0 || strstr(NameString,"3.40") != 0 ){clock = 3400.0;}
	else if( strstr(NameString,"3466") != 0 || strstr(NameString,"3.46") != 0 ){clock = 3466.66;}
	else if( strstr(NameString,"3533") != 0 || strstr(NameString,"3.53") != 0 ){clock = 3533.33;}
//	else if( strstr(NameString,"3600") != 0 || strstr(NameString,"3.60") != 0 ){clock = 3600.0;}
	else if( strstr(NameString,"3666") != 0 || strstr(NameString,"3.66") != 0 ){clock = 3666.66;}
	else if( strstr(NameString,"3667") != 0 || strstr(NameString,"3.67") != 0 ){clock = 3666.66;}
	else if( strstr(NameString,"3733") != 0 || strstr(NameString,"3.73") != 0 ){clock = 3733.33;}
//	else if( strstr(NameString,"3800") != 0 || strstr(NameString,"3.80") != 0 ){clock = 3800.0;}
	else if( strstr(NameString,"3866") != 0 || strstr(NameString,"3.86") != 0 ){clock = 3866.66;}
	else if( strstr(NameString,"3933") != 0 || strstr(NameString,"3.93") != 0 ){clock = 3933.33;}
//	else if( strstr(NameString,"4000") != 0 || strstr(NameString,"4.00") != 0 ){clock = 4000.0}
	else if( strstr(NameString,"4066") != 0 || strstr(NameString,"4.06") != 0 ){clock = 4066.66;}
	else if( strstr(NameString,"4133") != 0 || strstr(NameString,"4.13") != 0 ){clock = 4133.33;}
//	else if( strstr(NameString,"4200") != 0 || strstr(NameString,"4.20") != 0 ){clock = 4200.0;}
	else if( strstr(NameString,"4266") != 0 || strstr(NameString,"4.26") != 0 ){clock = 4266.66;}
	else if( strstr(NameString,"4333") != 0 || strstr(NameString,"4.33") != 0 ){clock = 4333.33;}
//	else if( strstr(NameString,"4400") != 0 || strstr(NameString,"4.40") != 0 ){clock = 4400.0;}
	else if( strstr(NameString,"4466") != 0 || strstr(NameString,"4.46") != 0 ){clock = 4466.66;}
	else if( strstr(NameString,"4533") != 0 || strstr(NameString,"4.53") != 0 ){clock = 4533.33;}
//	else if( strstr(NameString,"4600") != 0 || strstr(NameString,"4.60") != 0 ){clock = 4600.0;}
	else if( strstr(NameString,"4666") != 0 || strstr(NameString,"4.66") != 0 ){clock = 4666.66;}
	else if( strstr(NameString,"4667") != 0 || strstr(NameString,"4.67") != 0 ){clock = 4666.66;}
	else if( strstr(NameString,"4733") != 0 || strstr(NameString,"4.73") != 0 ){clock = 4733.33;}
//	else if( strstr(NameString,"4800") != 0 || strstr(NameString,"4.80") != 0 ){clock = 4800.0;}
	else if( strstr(NameString,"4866") != 0 || strstr(NameString,"4.86") != 0 ){clock = 4866.66;}
	else if( strstr(NameString,"4933") != 0 || strstr(NameString,"4.93") != 0 ){clock = 4933.33;}
//	else if( strstr(NameString,"5000") != 0 || strstr(NameString,"5.00") != 0 ){clock = 5000.0}
	else if( strstr(NameString," 100") != 0){clock = 100.0;}
	else if( strstr(NameString," 200") != 0){clock = 200.0;}
	else if( strstr(NameString," 300") != 0){clock = 300.0;}
	else if( strstr(NameString," 400") != 0){clock = 400.0;}
	else if( strstr(NameString," 500") != 0){clock = 500.0;}
	else if( strstr(NameString," 600") != 0){clock = 600.0;}
	else if( strstr(NameString," 700") != 0){clock = 700.0;}
	else if( strstr(NameString," 800") != 0){clock = 800.0;}
	else if( strstr(NameString," 900") != 0){clock = 900.0;}
	else if( strstr(NameString,"1000") != 0 || strstr(NameString,"1.0") != 0 ){clock = 1000.0;}
	else if( strstr(NameString,"1100") != 0 || strstr(NameString,"1.1") != 0 ){clock = 1100.0;}
	else if( strstr(NameString,"1200") != 0 || strstr(NameString,"1.2") != 0 ){clock = 1200.0;}
	else if( strstr(NameString,"1300") != 0 || strstr(NameString,"1.3") != 0 ){clock = 1300.0;}
	else if( strstr(NameString,"1400") != 0 || strstr(NameString,"1.4") != 0 ){clock = 1400.0;}
	else if( strstr(NameString,"1500") != 0 || strstr(NameString,"1.5") != 0 ){clock = 1500.0;}
	else if( strstr(NameString,"1600") != 0 || strstr(NameString,"1.6") != 0 ){clock = 1600.0;}
	else if( strstr(NameString,"1700") != 0 || strstr(NameString,"1.7") != 0 ){clock = 1700.0;}
	else if( strstr(NameString,"1800") != 0 || strstr(NameString,"1.8") != 0 ){clock = 1800.0;}
	else if( strstr(NameString,"1900") != 0 || strstr(NameString,"1.9") != 0 ){clock = 1900.0;}
	else if( strstr(NameString,"2000") != 0 || strstr(NameString,"2.0") != 0 ){clock = 2000.0;}
	else if( strstr(NameString,"2100") != 0 || strstr(NameString,"2.1") != 0 ){clock = 2100.0;}
	else if( strstr(NameString,"2200") != 0 || strstr(NameString,"2.2") != 0 ){clock = 2200.0;}
	else if( strstr(NameString,"2300") != 0 || strstr(NameString,"2.3") != 0 ){clock = 2300.0;}
	else if( strstr(NameString,"2400") != 0 || strstr(NameString,"2.4") != 0 ){clock = 2400.0;}
	else if( strstr(NameString,"2500") != 0 || strstr(NameString,"2.5") != 0 ){clock = 2500.0;}
	else if( strstr(NameString,"2600") != 0 || strstr(NameString,"2.6") != 0 ){clock = 2600.0;}
	else if( strstr(NameString,"2700") != 0 || strstr(NameString,"2.7") != 0 ){clock = 2700.0;}
	else if( strstr(NameString,"2800") != 0 || strstr(NameString,"2.8") != 0 ){clock = 2800.0;}
	else if( strstr(NameString,"2900") != 0 || strstr(NameString,"2.9") != 0 ){clock = 2900.0;}
	else if( strstr(NameString,"3000") != 0 || strstr(NameString,"3.0") != 0 ){clock = 3000.0;}
	else if( strstr(NameString,"3100") != 0 || strstr(NameString,"3.1") != 0 ){clock = 3100.0;}
	else if( strstr(NameString,"3200") != 0 || strstr(NameString,"3.2") != 0 ){clock = 3200.0;}
	else if( strstr(NameString,"3300") != 0 || strstr(NameString,"3.3") != 0 ){clock = 3300.0;}
	else if( strstr(NameString,"3400") != 0 || strstr(NameString,"3.4") != 0 ){clock = 3400.0;}
	else if( strstr(NameString,"3500") != 0 || strstr(NameString,"3.5") != 0 ){clock = 3500.0;}
	else if( strstr(NameString,"3600") != 0 || strstr(NameString,"3.6") != 0 ){clock = 3600.0;}
	else if( strstr(NameString,"3700") != 0 || strstr(NameString,"3.7") != 0 ){clock = 3700.0;}
	else if( strstr(NameString,"3800") != 0 || strstr(NameString,"3.8") != 0 ){clock = 3800.0;}
	else if( strstr(NameString,"3900") != 0 || strstr(NameString,"3.9") != 0 ){clock = 3900.0;}
	else if( strstr(NameString,"4000") != 0 || strstr(NameString,"4.0") != 0 ){clock = 4000.0;}
	else if( strstr(NameString,"4100") != 0 || strstr(NameString,"4.1") != 0 ){clock = 4100.0;}
	else if( strstr(NameString,"4200") != 0 || strstr(NameString,"4.2") != 0 ){clock = 4200.0;}
	else if( strstr(NameString,"4300") != 0 || strstr(NameString,"4.3") != 0 ){clock = 4300.0;}
	else if( strstr(NameString,"4400") != 0 || strstr(NameString,"4.4") != 0 ){clock = 4400.0;}
	else if( strstr(NameString,"4500") != 0 || strstr(NameString,"4.5") != 0 ){clock = 4500.0;}
	else if( strstr(NameString,"4600") != 0 || strstr(NameString,"4.6") != 0 ){clock = 4600.0;}
	else if( strstr(NameString,"4700") != 0 || strstr(NameString,"4.7") != 0 ){clock = 4700.0;}
	else if( strstr(NameString,"4800") != 0 || strstr(NameString,"4.8") != 0 ){clock = 4800.0;}
	else if( strstr(NameString,"4900") != 0 || strstr(NameString,"4.9") != 0 ){clock = 4900.0;}
	else if( strstr(NameString,"5000") != 0 || strstr(NameString,"5.0") != 0 ){clock = 5000.0;}
	else{
		clock = -1.0;
	}
	return clock;
}

/*
double CCpuInfo::GetMultiplierByModelNumber(int bus)
{
	if( bus == 2 ){ // 100MHz
			 if( strstr(NameString,"705") != 0){MultiplierOri = 14.0;}
		else if( strstr(NameString,"715") != 0){MultiplierOri = 15.0;}
		else if( strstr(NameString,"725") != 0){MultiplierOri = 16.0;}
		else if( strstr(NameString,"735") != 0){MultiplierOri = 17.0;}
		else if( strstr(NameString,"745") != 0){MultiplierOri = 18.0;}
		else if( strstr(NameString,"755") != 0){MultiplierOri = 20.0;}
		else if( strstr(NameString,"765") != 0){MultiplierOri = 21.0;}
		else if( strstr(NameString,"775") != 0){MultiplierOri = 22.0;}
		else if( strstr(NameString,"785") != 0){MultiplierOri = 23.0;}
		else if( strstr(NameString,"795") != 0){MultiplierOri = 24.0;}

		// Low Voltage
		else if( strstr(NameString,"703") != 0){MultiplierOri = 12.5;}
		else if( strstr(NameString,"713") != 0){MultiplierOri = 13.0;}
		else if( strstr(NameString,"723") != 0){MultiplierOri = 13.5;}
		else if( strstr(NameString,"733") != 0){MultiplierOri = 14.0;}
		else if( strstr(NameString,"743") != 0){MultiplierOri = 14.5;}
		else if( strstr(NameString,"753") != 0){MultiplierOri = 15.0;}
		else if( strstr(NameString,"763") != 0){MultiplierOri = 15.5;}
		else if( strstr(NameString,"773") != 0){MultiplierOri = 16.0;}
		else if( strstr(NameString,"783") != 0){MultiplierOri = 16.5;}
		else if( strstr(NameString,"793") != 0){MultiplierOri = 17.0;}

		// Ultra Low Voltage
		else if( strstr(NameString,"708") != 0){MultiplierOri =  9.5;}
		else if( strstr(NameString,"718") != 0){MultiplierOri = 10.0;}
		else if( strstr(NameString,"728") != 0){MultiplierOri = 10.5;}
		else if( strstr(NameString,"738") != 0){MultiplierOri = 11.0;}
		else if( strstr(NameString,"748") != 0){MultiplierOri = 11.5;}
		else if( strstr(NameString,"758") != 0){MultiplierOri = 12.0;}
		else if( strstr(NameString,"768") != 0){MultiplierOri = 12.5;}
		else if( strstr(NameString,"778") != 0){MultiplierOri = 13.0;}
		else if( strstr(NameString,"788") != 0){MultiplierOri = 13.5;}
		else if( strstr(NameString,"798") != 0){MultiplierOri = 14.0;}
		else {MultiplierOri = Multiplier;}

		return MultiplierOri;
	}else if( bus == 1 ){// 133MHz
			 if( strstr(NameString,"700") != 0){MultiplierOri =  9.0;}
		else if( strstr(NameString,"710") != 0){MultiplierOri = 10.0;}
		else if( strstr(NameString,"720") != 0){MultiplierOri = 11.0;}
		else if( strstr(NameString,"730") != 0){MultiplierOri = 12.0;}
		else if( strstr(NameString,"740") != 0){MultiplierOri = 13.0;}
		else if( strstr(NameString,"750") != 0){MultiplierOri = 14.0;}
		else if( strstr(NameString,"760") != 0){MultiplierOri = 15.0;}
		else if( strstr(NameString,"770") != 0){MultiplierOri = 16.0;}
		else if( strstr(NameString,"780") != 0){MultiplierOri = 17.0;}
		else if( strstr(NameString,"790") != 0){MultiplierOri = 18.0;}
		else {MultiplierOri = Multiplier;}

		return MultiplierOri;
	}else{
		MultiplierOri = Multiplier;
		return MultiplierOri;
	}
}
*/

double CCpuInfo::GetMultiplierByNameString(int bus)
{

	if( bus == 2 ){ // 100MHz
			 if( strstr(NameString," 300") != 0){MultiplierOri = 3.0;}
		else if( strstr(NameString," 400") != 0){MultiplierOri = 4.0;}
		else if( strstr(NameString," 500") != 0){MultiplierOri = 5.0;}
		else if( strstr(NameString," 600") != 0){MultiplierOri = 6.0;}
		else if( strstr(NameString," 700") != 0){MultiplierOri = 7.0;}
		else if( strstr(NameString," 800") != 0){MultiplierOri = 8.0;}
		else if( strstr(NameString," 900") != 0){MultiplierOri = 9.0;}

		else if( strstr(NameString,"1.73") != 0 ){MultiplierOri = 13.0;}
		else if( strstr(NameString,"1.86") != 0 ){MultiplierOri = 14.0;}
		else if( strstr(NameString,"2.00") != 0 ){
			if(SystemClock >= 133.33 * 0.95){
				MultiplierOri = 15.0;
			}else{
				MultiplierOri = 20.0;
			}
		}
		else if( strstr(NameString,"2.13") != 0 ){MultiplierOri = 16.0;}
		else if( strstr(NameString,"2.26") != 0 ){MultiplierOri = 17.0;}
		else if( strstr(NameString,"2.40") != 0 ){
			if(SystemClock >= 133.33 * 0.95){
				MultiplierOri = 18.0;
			}else{
				MultiplierOri = 24.0;
			}
		}
		else if( strstr(NameString,"2.53") != 0 ){MultiplierOri = 19.0;}
		else if( strstr(NameString,"2.66") != 0 ){MultiplierOri = 20.0;}
		else if( strstr(NameString,"2.67") != 0 ){MultiplierOri = 20.0;}
		else if( strstr(NameString,"2.80") != 0 ){
			if(SystemClock >= 133.33 * 0.95){
				MultiplierOri = 21.0;
			}else{
				MultiplierOri = 28.0;
			}
		}
		else if( strstr(NameString,"1000") != 0 || strstr(NameString,"1.0") != 0 ){MultiplierOri = 10.0;}
		else if( strstr(NameString,"1100") != 0 || strstr(NameString,"1.1") != 0 ){MultiplierOri = 11.0;}
		else if( strstr(NameString,"1200") != 0 || strstr(NameString,"1.2") != 0 ){MultiplierOri = 12.0;}
		else if( strstr(NameString,"1300") != 0 || strstr(NameString,"1.3") != 0 ){MultiplierOri = 13.0;}
		else if( strstr(NameString,"1400") != 0 || strstr(NameString,"1.4") != 0 ){MultiplierOri = 14.0;}
		else if( strstr(NameString,"1500") != 0 || strstr(NameString,"1.5") != 0 ){MultiplierOri = 15.0;}
		else if( strstr(NameString,"1600") != 0 || strstr(NameString,"1.6") != 0 ){MultiplierOri = 16.0;}
		else if( strstr(NameString,"1700") != 0 || strstr(NameString,"1.7") != 0 ){MultiplierOri = 17.0;}
		else if( strstr(NameString,"1800") != 0 || strstr(NameString,"1.8") != 0 ){MultiplierOri = 18.0;}
		else if( strstr(NameString,"1900") != 0 || strstr(NameString,"1.9") != 0 ){MultiplierOri = 19.0;}
		else if( strstr(NameString,"2000") != 0 || strstr(NameString,"2.0") != 0 ){MultiplierOri = 20.0;}
		else if( strstr(NameString,"2100") != 0 || strstr(NameString,"2.1") != 0 ){MultiplierOri = 21.0;}
		else if( strstr(NameString,"2200") != 0 || strstr(NameString,"2.2") != 0 ){MultiplierOri = 22.0;}
		else if( strstr(NameString,"2300") != 0 || strstr(NameString,"2.3") != 0 ){MultiplierOri = 23.0;}
		else if( strstr(NameString,"2400") != 0 || strstr(NameString,"2.4") != 0 ){MultiplierOri = 24.0;}
		else if( strstr(NameString,"2500") != 0 || strstr(NameString,"2.5") != 0 ){MultiplierOri = 25.0;}
		else if( strstr(NameString,"2600") != 0 || strstr(NameString,"2.6") != 0 ){MultiplierOri = 26.0;}
		else if( strstr(NameString,"2700") != 0 || strstr(NameString,"2.7") != 0 ){MultiplierOri = 27.0;}
		else if( strstr(NameString,"2800") != 0 || strstr(NameString,"2.8") != 0 ){MultiplierOri = 28.0;}
		else if( strstr(NameString,"2900") != 0 || strstr(NameString,"2.9") != 0 ){MultiplierOri = 29.0;}
		else if( strstr(NameString,"3000") != 0 || strstr(NameString,"3.0") != 0 ){MultiplierOri = 30.0;}

		return MultiplierOri;
	}else if( bus == 1 ){// 133MHz
        	 if( strstr(NameString," 400") != 0){MultiplierOri = 3.0;}
		else if( strstr(NameString," 466") != 0){MultiplierOri = 3.5;}
		else if( strstr(NameString," 533") != 0){MultiplierOri = 4.0;}
		else if( strstr(NameString," 600") != 0){MultiplierOri = 4.5;}
		else if( strstr(NameString," 666") != 0 || strstr(NameString," 667") != 0){MultiplierOri = 5.0;}
		else if( strstr(NameString," 733") != 0){MultiplierOri = 5.5;}
		else if( strstr(NameString," 800") != 0){MultiplierOri = 6.0;}
		else if( strstr(NameString," 866") != 0){MultiplierOri = 6.5;}
		else if( strstr(NameString," 933") != 0){MultiplierOri = 7.0;}
		else if( strstr(NameString,"1000") != 0 || strstr(NameString,"1.00") != 0 ){MultiplierOri = 7.5;}
		else if( strstr(NameString,"1066") != 0 || strstr(NameString,"1.06") != 0 ){MultiplierOri = 8.0;}
		else if( strstr(NameString,"1133") != 0 || strstr(NameString,"1.13") != 0 ){MultiplierOri = 8.5;}
		else if( strstr(NameString,"1200") != 0 || strstr(NameString,"1.20") != 0 ){MultiplierOri = 9.0;}
		else if( strstr(NameString,"1266") != 0 || strstr(NameString,"1.26") != 0 ){MultiplierOri = 9.5;}
		else if( strstr(NameString,"1333") != 0 || strstr(NameString,"1.33") != 0 ){MultiplierOri = 10.0;}
		else if( strstr(NameString,"1400") != 0 || strstr(NameString,"1.40") != 0 ){MultiplierOri = 10.5;}
		else if( strstr(NameString,"1466") != 0 || strstr(NameString,"1.46") != 0 ){MultiplierOri = 11.0;}
		else if( strstr(NameString,"1533") != 0 || strstr(NameString,"1.53") != 0 ){MultiplierOri = 11.5;}
		else if( strstr(NameString,"1600") != 0 || strstr(NameString,"1.60") != 0 ){MultiplierOri = 12.0;}
		else if( strstr(NameString,"1666") != 0 || strstr(NameString,"1.66") != 0 ){MultiplierOri = 12.5;}
		else if( strstr(NameString,"1667") != 0 || strstr(NameString,"1.67") != 0 ){MultiplierOri = 12.5;}
		else if( strstr(NameString,"1733") != 0 || strstr(NameString,"1.73") != 0 ){MultiplierOri = 13.0;}
		else if( strstr(NameString,"1800") != 0 || strstr(NameString,"1.80") != 0 ){MultiplierOri = 13.5;}
		else if( strstr(NameString,"1866") != 0 || strstr(NameString,"1.86") != 0 ){MultiplierOri = 14.0;}
		else if( strstr(NameString,"1933") != 0 || strstr(NameString,"1.93") != 0 ){MultiplierOri = 14.5;}
		else if( strstr(NameString,"2000") != 0 || strstr(NameString,"2.00") != 0 ){MultiplierOri = 15.0;}
		else if( strstr(NameString,"2066") != 0 || strstr(NameString,"2.06") != 0 ){MultiplierOri = 15.5;}
		else if( strstr(NameString,"2133") != 0 || strstr(NameString,"2.13") != 0 ){MultiplierOri = 16.0;}
		else if( strstr(NameString,"2200") != 0 || strstr(NameString,"2.20") != 0 ){MultiplierOri = 16.5;}
		else if( strstr(NameString,"2266") != 0 || strstr(NameString,"2.26") != 0 ){MultiplierOri = 17.0;}
		else if( strstr(NameString,"2333") != 0 || strstr(NameString,"2.33") != 0 ){MultiplierOri = 17.5;}
		else if( strstr(NameString,"2400") != 0 || strstr(NameString,"2.40") != 0 ){MultiplierOri = 18.0;}
		else if( strstr(NameString,"2466") != 0 || strstr(NameString,"2.46") != 0 ){MultiplierOri = 18.5;}
		else if( strstr(NameString,"2533") != 0 || strstr(NameString,"2.53") != 0 ){MultiplierOri = 19.0;}
		else if( strstr(NameString,"2600") != 0 || strstr(NameString,"2.60") != 0 ){MultiplierOri = 19.5;}
		else if( strstr(NameString,"2666") != 0 || strstr(NameString,"2.66") != 0 ){MultiplierOri = 20.0;}
		else if( strstr(NameString,"2667") != 0 || strstr(NameString,"2.67") != 0 ){MultiplierOri = 20.0;}
		else if( strstr(NameString,"2733") != 0 || strstr(NameString,"2.73") != 0 ){MultiplierOri = 20.5;}
		else if( strstr(NameString,"2800") != 0 || strstr(NameString,"2.80") != 0 ){MultiplierOri = 21.0;}
		else if( strstr(NameString,"2866") != 0 || strstr(NameString,"2.86") != 0 ){MultiplierOri = 21.5;}
		else if( strstr(NameString,"2933") != 0 || strstr(NameString,"2.93") != 0 ){MultiplierOri = 22.0;}
		else if( strstr(NameString,"3000") != 0 || strstr(NameString,"3.00") != 0 ){MultiplierOri = 22.5;}
		return MultiplierOri;
	}else{
		MultiplierOri = Multiplier;
		return MultiplierOri;
	}
}

void CCpuInfo::SetCPUFSB()
{
	DWORD mul,bus;
	DWORD BusTable[10] = {    6666666, 13333333, 10000000, 16666666 , 20000000 , 23333333, 26666666, 0,
							 30000000, 33333333};

// for Debug
/*
	FlagBrand = AMD;
	FlagMultiTable = ATHLON;
	Family = 6;
	Model = 8;
//	FlagK7Sempron = TRUE;

	FlagBrand = INTEL;
	FlagMultiTable = Banias;
	MsrEAX1 = 0x43480000;MsrEDX1 = 0xD00086AB;
	MsrEAX2 = 0x00000000;MsrEDX2 = 0x00140000;
*/

	if( FlagBrand == CYRIX || FlagBrand == IDT ){
		bus = (MsrEAX1>>18)&0x3;
		mul = (MsrEAX1>>22)&0xF;
		switch ( FlagMultiTable )
		{
		case ESTHER:
			ClockOri = GetOriginalClockByNameString();
			if(ClockOri <= 1500.0){
				bus = 2; /* 100MHz */
				MultiplierOri = ClockOri / 100.0;
				Multiplier = Clock / 100.0;
				Multiplier = ((int)(Multiplier + 0.25) * 2) / 2;
			}else{
				bus = 1; /* 133MHz */
				MultiplierOri = ClockOri / 133.333333333333;
				Multiplier = Clock / 133.333333333333;
				Multiplier = ((int)(Multiplier + 0.25) * 2) / 2;
			}
			Multiplier = -1.0;
			break;
		case NEHEMIAH:
			if( (MsrEAX1>>27) & 0x1 ){mul += 16;}
			Multiplier = MultiplierOri = MultiTableNehemiah[mul] / 2.0;
			break;
		case EZRA_T:
			if( (MsrEAX1>>27) & 0x1 ){mul += 16;}
			Multiplier = MultiplierOri = MultiTableEzraT[mul] / 2.0;
			break;
		case EZRA:
			Multiplier = MultiplierOri = MultiTableEzra[mul] / 2.0;
			break;
		case SAMUEL2:
			Multiplier = MultiplierOri = MultiTableSamuel2[mul] / 2.0;
			break;
		case CYRIX3:
			Multiplier = MultiplierOri = MultiTableCyrix3[mul] / 2.0;
			break;

		default:
			if( (FlagBrand == IDT && Family == 5 && Model == 8
			  && 7 <= Stepping && Stepping <= 9) // WinChip 2A
			||	(FlagBrand == IDT && Family == 5 && Model == 9)// WinChip 3
			){
				Multiplier = MultiplierOri = ( ( (MsrEAX2 >> 23) & 0xF ) + 2.0) / ( (MsrEAX1 & 0x3) + 2.0 );
			}else if(FlagBrand == IDT && Model == 8){ // WinChip 2/B
				Multiplier = MultiplierOri = (MsrEAX1 & 0x3) + 2.0;
			}
			bus = 7;
			break;
		}
	}else if(FlagBrand == INTEL){
		bus = (MsrEAX1>>18)&0x3;
		mul = (MsrEAX1>>22)&0xF;

		switch ( FlagMultiTable )
		{
		case PENRYN:
			Multiplier = ((MsrEAX1>>22) & 0x1F) + ((MsrEAX1>>18) & 0x1) / 2.0;
			SystemClock = Clock / Multiplier;
			DWORD EAX, EDX;
			ReadMSR(0x00000198,&EAX,&EDX); 
			MultiplierOri = (double)((EDX >> 8) & 0x1F) + ((EDX >> 14) & 0x01) / 2.0; // Max Multiplier
			ClockOri = GetOriginalClockByNameString();
			// 
			if( ClockOri / MultiplierOri > 333 ){
				bus = 9; // 333MHz
			}else if( ClockOri / MultiplierOri > 266 ){
				bus = 6; // 266MHz
			}else if( ClockOri / MultiplierOri > 199 ){
				bus = 4; // 200MHz
			}
			break;
		case DOTHAN:
		case BANIAS:
			{
			Multiplier = (MsrEAX1>>22) & 0x1F;
			SystemClock = Clock / Multiplier;
			DWORD EAX, EDX;
			ReadMSR(0x00000198,&EAX,&EDX); 
			MultiplierOri = (double)((EDX >> 8) & 0x1F); // Max Multiplier
			ClockOri = GetOriginalClockByNameString();
			if(strstr(NameString, "Quad") && strstr(NameString, "2.93")){ // Core 2 Extreme (Quad)
				bus = 6; // 266MHz
				MultiplierOri = 11.0;
			}else if(strstr(NameString, "Quad") && strstr(NameString, "2.66")){ // Core 2 Extreme (Quad)
				bus = 6; // 266MHz
				MultiplierOri = 10.0;
			}else if(strstr(NameString, "Duo") && strstr(NameString, "2.93")){ // Core 2 Extreme (Duo)
				bus = 6; // 266MHz
				MultiplierOri = 11.0;
			}else if( ClockOri / MultiplierOri > 333 ){
				bus = 9; // 333MHz
			}else if( ClockOri / MultiplierOri > 266 ){
				bus = 6; // 266MHz
			}else if( ClockOri / MultiplierOri > 199 ){
				bus = 4; // 200MHz
			}else if( ClockOri / MultiplierOri > 166 ){
				bus = 3; // 166MHz
			}else if( ClockOri / MultiplierOri > 133 ){
				bus = 1; // 133MHz
			}else{
				bus = 2; // 100MHz
			}
			}
			break;
		case NORTHWOOD:
			MultiplierOri = (MsrEAX2)&0x1F;
			Multiplier    = (MsrEAX2>>24)&0x1F;

			switch( (MsrEAX2>>16) & 0x7 )
			{
			case 0:
				SystemClock = Clock / Multiplier;
				if ( Model >= 3 ){ // Pentium 4 XE
					bus = 6;	// 266MHz
				}else{
					bus = 2;	// 100MHz
				}
				break;
			case 1:	bus = 1;	break;	// 133MHz
			case 2:	bus = 4;	break;	// 200MHz
			case 3:	bus = 3;	break;	// 166MHz
			default:bus = 6;	break;	// 266MHz
			}
			break;
		case WILLAMETTE:
			mul = (MsrEAX1>>8)&0xF;
			bus = 2;
			Multiplier = MultiplierOri = MultiTableWillamette[mul] / 2.0;
			break;
		case TUALATIN:
			if( (MsrEAX1>>27)&0x1 ){mul += 16;}
			Multiplier = MultiTableTualatin[mul] / 2.0;
			MultiplierOri = GetMultiplierByNameString(bus);
			break;
		case COPPERMINE:
			if( ( MsrEAX1 >> 27 ) & 0x1 && ( ( Model == 8 && Stepping >= 3 ) || Model == 0xA ) ){
				mul += 16;
			}
			Multiplier = MultiplierOri = MultiTableCoppermine[mul] / 2.0;
			break;
		case P6:
			if(Model <= 2 || (Model == 0x3 && Type == 0x1) ){
				bus = 7;
			}
			Multiplier = MultiplierOri = MultiTableGeneric[mul] / 2.0;
			break;
		}		
	}else if( FlagBrand == AMD ){
		switch ( FlagMultiTable )
		{
		case K6:
			mul = MsrEAX1&0xF;
			if(Model >= 12){
				Multiplier = MultiplierOri = MultiTableK6M13[mul] / 2.0;
				LhCurrentFID = mul; // for LoveHammer
			}else if(Model == 0xA){ // Geode LX
				Multiplier = ((MsrEDX1 >> 1) & 0x1F) + 1.0;
				LhCurrentFID = (MsrEDX1 >> 1) & 0x1F; // for LoveHammer
			}else{
				Multiplier = MultiplierOri = MultiTableK6[mul] / 2.0;
			}
			if(Model >= 12){
				bus = 2;
			}else if(Model == 0xA){// Geode LX
				SystemClock = Clock / Multiplier; // 33MHz
			}else{
				bus = 7;
			}
			break;
		case ATHLON:
			mul = (MsrEAX1>>24) & 0xF;
			if( (MsrEAX1>>19)&0x1 ){mul += 16;}

			if(CacheL2 == 64){bus = 2;}
			Multiplier = MultiplierOri = MultiTableAthlon[mul] / 2.0;
			SystemClock = Clock / MultiplierOri;

			if(Family == 0x6 && Model == 0xA){ //Barton or Thorton
				if( FlagK7Sempron ){ // Sempron
					bus = 3; // 166MHz
				}else if( FlagAMDMP ){ // MP
					bus = 1; // 133MHz
				}else if( CacheL2 == 256 ){ // Thorton
					bus = 1; // 133MHz
				}else if( SystemClock >= 200.0 * 0.95 ){
					bus = 4; // 200MHz
				}else{
					bus = 3; // 166MHz
				}
			}else if(Family == 0x6 && Model == 0x8){// Thoroughbred
				if(CacheL2 == 64){ // Applebred
					bus = 1; // 133MHz
				}else if( FlagK7Sempron || (Stepping >= 1 && MultiplierOri >= 12.5 && SystemClock >= 166.66 * 0.95 ) ){
					bus = 3; // 166MHz
				}else{
					bus = 1; // 133MHz
				}
			}else if(Family == 0x6 && Model == 0x6){// Palomino
				bus = 1;
			}else{ // K7/K75
				bus = 2;
			}
			// Thunderbird
			if(Family == 0x6 && Model == 0x4){
				if(mul == 4){ Multiplier = MultiplierOri = 14.0; /* Thunderbird 1400MHz !? */	}
				if(mul == 3){ Multiplier = MultiplierOri = 13.0; /* Thunderbird 1300MHz */	}
				if( (int)(MultiplierOri*10) >= 110 || (int)(MultiplierOri*10) == 80 || (int)(MultiplierOri*10) == 95){
					bus = 2;//100
				}
				else if( (int)(MultiplierOri*10) == 105 ){bus = 1;} // 133
				else{
					SystemClock = Clock / MultiplierOri;
					if(SystemClock > (133.33 * 0.95) ){bus = 1;} // 95%
					else{bus = 2;}
				}
			}
			break;
		case MOBILE_ATHLON:
			Multiplier    = MultiTableMobileAthlon[ MsrEAX2 & 0x1F ] / 2.0;
			MultiplierOri = MultiTableMobileAthlon[ (MsrEAX2 >> 16) & 0x1F ] / 2.0;
			SystemClock = Clock / Multiplier;

			if(MultiTableMobileAthlon[ (MsrEAX2 >> 8) & 0x1F ] == 10){ // Startup Multiplier = x5.0
				bus = 2; // 100MHz
			}else if( MultiTableMobileAthlon[ (MsrEAX2 >> 8) & 0x1F ] == 12 ){ // Startup Multiplier = x6.0
				bus = 1; // 133MHz
			}else{ // for MOD Athlon XP-M
				if( Model == 8 // Thoroughbred
					&&
					(	MultiTableMobileAthlon[ (MsrEAX2 >> 16) & 0x1F ] == 24
					||	MultiTableMobileAthlon[ (MsrEAX2 >> 16) & 0x1F ] == 26
					||	MultiTableMobileAthlon[ (MsrEAX2 >> 16) & 0x1F ] == 28
					)
					&& ( LhMaxVID == 12 || LhMaxVID == 15 )
				){ 
					bus = 2; // 100MHz
				}else{
					bus = 1; // 133MHz
				}
			}
			if(Model <= 7){bus = 2;} // Palomino/Morgan = 100 MHz
			break;
		case ATHLON_64:
			Multiplier = MultiplierOri = ( MsrEAX1 >> 24 & 0x3F ) / 2.0 + 4.0;
			LhCurrentFID = ( MsrEAX1 >> 24 & 0x3F );
			LhMaxFID = ( MsrEAX1 >> 24 & 0x3F );
			bus = 4; // 200MHz
			break;
		case MOBILE_ATHLON_64:
			Multiplier = ( MsrEAX2 & 0x3F ) / 2.0 + 4.0;
			if( ( (MsrEAX2 >> 16) & 0x3F ) == 42 ){ // Athlon 64 FX
				MultiplierOri = ( (MsrEAX2 >> 8) & 0x3F ) / 2.0 + 4.0;
			}else{
				MultiplierOri = ( (MsrEAX2 >> 16) & 0x3F ) / 2.0 + 4.0;
			}

			LhCurrentFID = ( MsrEAX2 & 0x3F );
			LhMaxFID = ( MsrEAX1 >> 24 & 0x3F );
			bus = 4; // 200MHz
			break;
		case K10:
			Multiplier = (double)((MsrEAX1&0x3F) + 0x10) / (pow(2, ((MsrEAX1>>6)&0x7)));
			if(((MsrEDX1>>17)&0x3F) != 0){
				MultiplierOri = (double)((MsrEDX1>>17)&0x3F) / 2.0;
			}
			bus = 4; // 200MHz
			break;
		}
	}
	
	if(Multiplier < 0.0){
		Multiplier = MultiplierOri;
	}

	if(MultiplierOri < 0.5){
		MultiplierOri = -1.0;
	}else{
		SystemClock = Clock / Multiplier;
		SystemBus = SystemClock * FSBMultiplier;
		SystemClockOri = BusTable[bus] / 100000.0;
		SystemBusOri = SystemClockOri * FSBMultiplier;
		ClockOri = SystemClockOri * MultiplierOri;
	}

	if(FlagBrand == TMx86){
		ClockOri = TmNominalClock;
	}
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE hProcess,PBOOL Wow64Process);

BOOL IsWow64(VOID)
{
	BOOL bIsWow64 = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"),"IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
			bIsWow64 = FALSE;
		}
	}
	return bIsWow64;
}
//////////////////////////////////////////////////////////////////////
// Init Data
//////////////////////////////////////////////////////////////////////

void CCpuInfo::InitData()
{
#ifndef _X86_64
	/*
	ModeX86_64 = FALSE;
	typedef BOOL (*_IsWow64Process) (HANDLE, PBOOL);
	typedef void (*_GetNativeSystemInfo) (LPSYSTEM_INFO);

	HINSTANCE hLib = LoadLibrary("kernel32.dll");
	if(hLib != NULL){
		_IsWow64Process pIsWow64Process = (_IsWow64Process) GetProcAddress (hLib,"IsWow64Process");
		_GetNativeSystemInfo pGetNativeSystemInfo = (_GetNativeSystemInfo) GetProcAddress (hLib,"GetNativeSystemInfo");
		if( pIsWow64Process != NULL ){
			BOOL flag;
			if( pGetNativeSystemInfo != NULL ){
				SYSTEM_INFO si;
				pGetNativeSystemInfo(&si);
				if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ){
					ModeX86_64 = TRUE;
					gFlagStatus = 4; // DLLSTATUS_MODE_X86_64
				}
			if( pIsWow64Process(GetCurrentProcess(), &flag) ){
			}
			}
		}
	}

	FreeLibrary(hLib);
	*/
	if( handle == NULL ){
		ModeX86_64 = TRUE;
		gFlagStatus = 4; // DLLSTATUS_MODE_X86_64
	}else{
		ModeX86_64 = FALSE;
	}
#else if
	ModeX86_64 = FALSE;
#endif

	char* ptrEnd;
	char str[256];
	char path[MAX_PATH];
	char ini[MAX_PATH];
	::GetModuleFileName(NULL, path, MAX_PATH);
	::GetModuleFileName(NULL, ini, MAX_PATH);
	if ( (ptrEnd = strrchr(path, '\\')) != NULL ){
		*ptrEnd = '\0';
	}
	if ( (ptrEnd = strrchr(ini, '.')) != NULL ){
		*ptrEnd = '\0';
		strcat(ini, ".ini");
	}
	GetPrivateProfileString("Setting","CqDualWaitTime","1000",str,256,ini);
	DualWaitTime = abs( atoi(str) );

	FlagCPUID = 0;
	FlagRDTSC = 0;
	FlagRDMSR = 0;
	FlagBrand = 0;
	FlagAMDMP = 0;
	FlagHalfSpeedCache = 0;
	FlagMultiTable = 0;
	FlagAMDMobile = 0;
	FlagK7Sempron = 0;

	FlagClockModulation = 0;
	FlagProcessorSerial = 0;
	FlagSpeedStep = 0;
	FlagEIST = 0;
	FlagPowerNow = 0;
	FlagLongHaul = 0;
	FlagLongRun = 0;

	Clock = -1.0;
	Number = 1;
	Family = -1;
	Model = -1;
	Stepping = -1;
	FamilyEx = -1;
	ModelEx = -1;
	FamilyX = -1;
	ModelX = -1;
	ExFamily = -1;
	ExModel = -1;
	ExStepping = -1;
	ExFamilyX = -1;
	ExModelX = -1;
	Type = -1;
	CacheL1I = -1;
	CacheL1T = -1;
	CacheL1U = -1;
	CacheL1D = -1;
	CacheL2  = -1;
	CacheL3  = -1;
	BrandID = 0;
	BrandIDEx = 0;
	BrandIDNN = 0;
	PwrLmt = 0;
	SocketID = -1;
	CmpCap = 0;
	Feature = 0;
	FeatureEcx = 0;
	FeatureEx = 0;
	FeatureExEcx = 0;
	FeatureVia = 0;
	FeatureTransmeta = 0;
	CacheSpeed = -1.0;
	OverClock = -100.0;
	MiscInfo = 0;
	MiscInfoEx = 0;
	Version = 0;
	VersionEx = 0;
	PlatformID = -1;
	MicrocodeID = -1;

	Apic = -1;
	PhysicalCoreNum = 1;

	NameSysInfo[0] = '\0';
	FSBMode[0] = '\0';
	VendorString[0] = '\0';
	VendorName[0] = '\0';
	NameString[0] = '\0';
	PlatformName[0] = '\0';
	TypeName[0] = '\0';
	FMS[0] = '\0';
	CacheSpeedS[0] = '\0';
	Name[0] = '\0';
	CodeName[0] = '\0';
	FullName[0] = '\0';
	ProcessRule[0] = '\0';
	ProcessorSerial[0] = '\0';
	Architecture[0] = '\0'; 
	MeasureMode[0] = '\0'; 
	ModelNumber[0] = '\0';
	CoreRevision[0] = '\0';
	Logo[0] = '\0';

// Transmeta
	TmNameString[0] = '\0';
	TmClock = -1;
	TmNominalClock = -1;
	TmCurrentVoltage = -1;
	TmCurrentPerformance = -1;
	TmCurrentGateDelay = -1;
	TmHardwareVersion[0] = '\0';
	TmSoftwareVersion[0] = '\0';

	SystemClock = -1.0;
	SystemBus = -1.0;
	Multiplier = -1.0;
	MultiplierOri = -1.0;
	DieSize	= -1.0;
	Transister = -1.0;

	ClockOri = -1.0;
	SystemClockOri = -1.0;
	SystemBusOri = -1.0;

	FSBMultiplier = -1;
	MsrEAX1 = 0;
	MsrEAX2 = 0;
	MsrEDX1 = 0;
	MsrEDX2 = 0;

	FlagMMX = 0;
	FlagMMXEx = 0;
	FlagSSE = 0;
	FlagSSE2 = 0;
	FlagSSE3 = 0;
	FlagSSSE3 = 0;
	FlagSSE4 = 0;
	FlagSSE41 = 0;
	FlagSSE42 = 0;
	FlagSSE4A = 0;
	FlagSSE5 = 0;
	FlagAVX = 0;
	Flag3DNow = 0;
	Flag3DNowEx = 0;
	FlagHT = 0;
	FlagVT = 0;
	FlagAmdV = 0;
	FlagAA64 = 0;
	FlagIA64 = 0;
	FlagIA32e = 0;
	FlagDualCore = 0;

// CPU Number
#ifdef _WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	Number = si.dwNumberOfProcessors;

	if( IsWow64() ){
		strcpy(Architecture,"x64");
	}else{

		switch(si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_INTEL:
			strcpy(Architecture,"x86");
			break;
		case PROCESSOR_ARCHITECTURE_MIPS:
			strcpy(Architecture,"MIPS");
			break;
		case PROCESSOR_ARCHITECTURE_ALPHA:
			strcpy(Architecture,"Alpha");
			break;
		case PROCESSOR_ARCHITECTURE_PPC:
			strcpy(Architecture,"PowerPC");
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			strcpy(Architecture,"IA64");
			break;
		case PROCESSOR_ARCHITECTURE_AMD64:
			strcpy(Architecture,"x64");
			break;
		case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
			strcpy(Architecture,"WOW64");
			break;
		default:
			break;
		}
	}
#else
// for UNIX
	Number = 1; // (^_^;;
#endif

	L1DWaysS[0] = '\0';
	L1DLinesS[0] = '\0';
	L2WaysS[0] = '\0';
	L2LinesS[0] = '\0';
	L3WaysS[0] = '\0';
	L3LinesS[0] = '\0';
	L1ITUWaysS[0] = '\0';
	L1ITULinesS[0] = '\0';
	
	L1ITUWays = -1;
	L1ITULines = -1;
	L1DWays = -1;
	L1DLines = -1;
	L2Ways = -1;
	L2Lines = -1;
	L3Ways = -1;
	L3Lines = -1;

// Love Hammer & LongHaul
	FlagLH = 0;
	LhCurrentFID = 0;
	LhCurrentVID = 0;
	LhStartupFID = 0;
	LhStartupVID = 0;
	LhMaxFID = 0;
	LhMinVID = 0;
	LhMaxVID = 0;

	LhCurrentMultiplier[0] = '\0';
	LhStartupMultiplier[0] = '\0';
	LhMaxMultiplier[0] = '\0';
	LhCurrentVoltage[0] = '\0';
	LhStartupVoltage[0] = '\0';
	LhMinVoltage[0] = '\0';
	LhMaxVoltage[0] = '\0';

}

//////////////////////////////////////////////////////////////////////
// Check CPUID / RDTSC Enabled
//////////////////////////////////////////////////////////////////////
int CCpuInfo::CheckEnableCPUID()
{
#ifdef _WIN32
#ifndef _X86_64
	DWORD flag_1, flag_2;
	_asm{
			pushfd
			pop		eax
			mov		flag_1, eax
			xor		eax, 00200000h
			push	eax
			popfd
			pushfd
			pop		eax
			mov		flag_2, eax
		}
    if( flag_1 == flag_2){	// Disabled CPUID
		FlagCPUID = FALSE;
    }else{					// Enabled CPUID
		FlagCPUID = TRUE;
	}
#else
	FlagCPUID = TRUE;
#endif

#else
// for UNIX
	FlagCPUID = TRUE; // (^_^;
#endif
	return FlagCPUID;
}

//////////////////////////////////////////////////////////////////////
// Get Cache Information
//////////////////////////////////////////////////////////////////////
int CCpuInfo::FillCacheInfo()
{
	if( CacheL1U > 0 || CacheL1I > 0 || CacheL1D > 0 ){
		return -1;
	}
	if( FlagBrand == INTEL ){
		if( MaxCPUID >= 0x2 ){
			FillCacheInfoIntel();
		}
	}else{
		if( MaxCPUIDEx >= 0x80000005 ){
			FillCacheInfoAMD();
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Get Cache Information <INTEL>
//////////////////////////////////////////////////////////////////////
void CCpuInfo::FillCacheInfoIntel()
{
	DWORD EAX, EBX, ECX, EDX;
	cpuid(0x2, &EAX, &EBX, &ECX, &EDX);

	CacheInfoIntel( (EAX & 0xFF000000) >> 24 );
	CacheInfoIntel( (EAX & 0x00FF0000) >> 16 );
	CacheInfoIntel( (EAX & 0x0000FF00) >> 8  );
	CacheInfoIntel(  EAX & 0x000000FF );
	CacheInfoIntel( (EBX & 0xFF000000) >> 24 );
	CacheInfoIntel( (EBX & 0x00FF0000) >> 16 );
	CacheInfoIntel( (EBX & 0x0000FF00) >> 8  );
	CacheInfoIntel(  EBX & 0x000000FF );
	CacheInfoIntel( (ECX & 0xFF000000) >> 24 );
	CacheInfoIntel( (ECX & 0x00FF0000) >> 16 );
	CacheInfoIntel( (ECX & 0x0000FF00) >> 8  );
	CacheInfoIntel(  ECX & 0x000000FF );
	CacheInfoIntel( (EDX & 0xFF000000) >> 24 );
	CacheInfoIntel( (EDX & 0x00FF0000) >> 16 );
	CacheInfoIntel( (EDX & 0x0000FF00) >> 8  );
	CacheInfoIntel(  EDX & 0x000000FF );

}

//////////////////////////////////////////////////////////////////////
// Get Cache Information <INTEL>
//////////////////////////////////////////////////////////////////////
void CCpuInfo::CacheInfoIntel(int TEST)
{
	switch ( TEST )
	{
	case 0x06:CacheL1I=8;L1ITUWays=4;L1ITULines=32;			break;
	case 0x08:CacheL1I=16;L1ITUWays=4;L1ITULines=32;		break;

	case 0x0A:CacheL1D=8;L1DWays=2;L1DLines=32;				break;
	case 0x0C:CacheL1D=16;L1DWays=4;L1DLines=32;			break;

	case 0x10:CacheL1D=16;L1DWays=4;L1DLines=32;			break;	// IA-64
	case 0x15:CacheL1I=16;L1ITUWays=4;L1ITULines=32;		break;	// IA-64
	case 0x1A:CacheL2=96;L2Ways=6;L2Lines=64;				break;	// IA-64

	case 0x22:CacheL3=512;L3Ways=4;L3Lines=64;				break;
	case 0x23:CacheL3=1024;L3Ways=8;L3Lines=64;				break;
	case 0x25:CacheL3=2048;L3Ways=8;L3Lines=64;				break;
	case 0x29:CacheL3=4096;L3Ways=8;L3Lines=64;				break;

	case 0x2C:CacheL1D=32;L1DWays=8;L1DLines=64;			break;
	case 0x30:CacheL1I=32;L1ITUWays=8;L1ITULines=64;		break;

	case 0x39:CacheL2=128;L2Ways=4;L2Lines=64;				break;
	case 0x3A:CacheL2=192;L2Ways=6;L2Lines=64;				break;
	case 0x3B:CacheL2=128;L2Ways=2;L2Lines=64;				break;
	case 0x3C:CacheL2=256;L2Ways=4;L2Lines=64;				break;
	case 0x3D:CacheL2=384;L2Ways=6;L2Lines=64;				break;
	case 0x3E:CacheL2=512;L2Ways=4;L2Lines=64;				break;

	case 0x40:
		if(Family == 6){CacheL2=0;}
		else{CacheL3=0;}		
		break;

	case 0x41:CacheL2=128;L2Ways=4;L2Lines=32;				break;
	case 0x42:CacheL2=256;L2Ways=4;L2Lines=32;				break;
	case 0x43:CacheL2=512;L2Ways=4;L2Lines=32;				break;
	case 0x44:CacheL2=1024;L2Ways=4;L2Lines=32;				break;
	case 0x45:CacheL2=2048;L2Ways=4;L2Lines=32;				break;
	case 0x46:CacheL3=4096;L3Ways=4;L3Lines=64;				break;
	case 0x47:CacheL3=8192;L3Ways=8;L3Lines=64;				break;
	case 0x48:CacheL2=3072;L2Ways=12;L2Lines=64;			break;

	case 0x49:
		if(Family == 0xF){
			CacheL3=4096;L3Ways=16;L3Lines=64;
		}else{
			CacheL2=4096;L2Ways=16;L2Lines=64;
		}
		break;
	case 0x4A:CacheL3=6144;L3Ways=12;L3Lines=64;			break;
	case 0x4B:CacheL3=8192;L3Ways=16;L3Lines=64;			break;
	case 0x4C:CacheL3=12288;L3Ways=12;L3Lines=64;			break;
	case 0x4D:CacheL3=16384;L3Ways=16;L3Lines=64;			break;
	case 0x4E:CacheL2=6144;L2Ways=24;L2Lines=64;			break;

	case 0x60:CacheL1D=16;L1DWays=8;L1DLines=64;			break; // Prescott
	case 0x66:CacheL1D=8;L1DWays=4;L1DLines=64;				break;
	case 0x67:CacheL1D=16;L1DWays=4;L1DLines=64;			break;
	case 0x68:CacheL1D=32;L1DWays=4;L1DLines=64;			break;

	case 0x70:CacheL1T=12;L1ITUWays=8;						break;
	case 0x71:CacheL1T=16;L1ITUWays=8;						break;
	case 0x72:CacheL1T=32;L1ITUWays=8;						break;
	case 0x73:CacheL1T=64;L1ITUWays=8;						break;

	case 0x77:CacheL1I=16;L1ITUWays=8;L1ITULines=64;		break;	// IA-64

	case 0x78:CacheL2=1024;L2Ways=4;L2Lines=64;				break;
	case 0x79:CacheL2=128;L2Ways=8;L2Lines=64;				break;
	case 0x7A:CacheL2=256;L2Ways=8;L2Lines=64;				break;
	case 0x7B:CacheL2=512;L2Ways=8;L2Lines=64;				break;
	case 0x7C:CacheL2=1024;L2Ways=8;L2Lines=64;				break;
	case 0x7D:CacheL2=2048;L2Ways=8;L2Lines=64;				break;
	case 0x7E:CacheL2=256;L2Ways=8;L2Lines=128;				break;
	case 0x7F:CacheL2=512;L2Ways=2;L2Lines=64;				break;

	case 0x81:CacheL2=128;L2Ways=8;L2Lines=32;				break;
	case 0x82:CacheL2=256;L2Ways=8;L2Lines=32;				break;
	case 0x83:CacheL2=512;L2Ways=8;L2Lines=32;				break;
	case 0x84:CacheL2=1024;L2Ways=8;L2Lines=32;				break;
	case 0x85:CacheL2=2048;L2Ways=8;L2Lines=32;				break;

	case 0x86:CacheL2=512;L2Ways=8;L2Lines=64;				break;
	case 0x87:CacheL2=1024;L2Ways=8;L2Lines=64;				break;

	case 0x88:CacheL3=2048;L3Ways=4;L3Lines=64;				break;	// IA-64
	case 0x89:CacheL3=4096;L3Ways=4;L3Lines=64;				break;	// IA-64
	case 0x8A:CacheL3=8192;L3Ways=4;L3Lines=64;				break;	// IA-64

	case 0x8D:CacheL3=3096;L3Ways=12;L3Lines=128;			break;	// IA-64

	default :		;
	}
	sprintf(L1ITUWaysS,"%d",L1ITUWays);
	sprintf(L1ITULinesS,"%d",L1ITULines);
	sprintf(L1DWaysS,"%d",L1DWays);
	sprintf(L1DLinesS,"%d",L1DLines);
	sprintf(L2WaysS,"%d",L2Ways);
	sprintf(L2LinesS,"%d",L2Lines);
	sprintf(L3WaysS,"%d",L3Ways);
	sprintf(L3LinesS,"%d",L3Lines);
}

//////////////////////////////////////////////////////////////////////
// Get Cache Information <AMD>
//////////////////////////////////////////////////////////////////////
void CCpuInfo::FillCacheInfoAMD()
{
	DWORD EAX, EBX, ECX, EDX;
	cpuid(0x80000005, &EAX, &EBX, &ECX, &EDX);

	CacheL1I = EDX >> 24;
	CacheL1D = ECX >> 24;

	L1ITUWays = (EDX >> 16) & 0xFF;
	L1ITULines= EDX & 0xFF;
	L1DWays   = (ECX >> 16) & 0xFF;
	L1DLines  = ECX & 0xFF;

	sprintf(L1ITULinesS,"%d",L1ITULines);
	sprintf(L1DLinesS,"%d",L1DLines);

	if(L1ITUWays == 0xFF){
		sprintf(L1ITUWaysS,"Full");
	}else{
		sprintf(L1ITUWaysS,"%d",L1ITUWays);
	}
	if(L1DWays == 0xFF){
		sprintf(L1DWaysS,"Full");
	}else{
		sprintf(L1DWaysS,"%d",L1DWays);
	}

	if(FlagBrand == AMD && Family == 6 && Model == 3 && Stepping == 0){ // Duron Bug
		CacheL2 = 64;
	}else if(FlagBrand == AMD && Family == 6 && Model == 4 && Stepping <= 1){ // ThunderBird Bug
		CacheL2 = 256;
	}else if(MaxCPUIDEx >= 0x80000006){ // L2 Cache
		cpuid(0x80000006, &EAX, &EBX, &ECX, &EDX);
		CacheL2 = ECX >> 16;
		if(FlagBrand == IDT && ( Family == 6 && Model >= 7 ) ){
			CacheL2 = 64;// Nehemiah, Ezra-T, Ezra
		}
		L2Ways  = (ECX >> 12) & 0xF;
		L2Lines = ECX & 0xFF;
		sprintf(L2LinesS,"%d",L2Lines);
		switch(L2Ways){
			case 0x0:	sprintf(L2WaysS,"None");			break;
			case 0x1:	sprintf(L2WaysS,"Direct Mapped");	break;
			case 0x2:	sprintf(L2WaysS,"2");				break;
			case 0x4:	sprintf(L2WaysS,"4");				break;
			case 0x6:	sprintf(L2WaysS,"8");				break;
			case 0x8:	sprintf(L2WaysS,"16");				break;
			case 0xA:	sprintf(L2WaysS,"32");				break;
			case 0xB:	sprintf(L2WaysS,"48");				break;
			case 0xC:	sprintf(L2WaysS,"64");				break;
			case 0xD:	sprintf(L2WaysS,"96");				break;
			case 0xE:	sprintf(L2WaysS,"128");				break;
			case 0xF:	sprintf(L2WaysS,"Full");			break;
			default:	sprintf(L2WaysS,"None");			break;
		}
		CacheL3 = (EDX >> 18) * 512;
	}
}

//////////////////////////////////////////////////////////////////////
// Get NameString
//////////////////////////////////////////////////////////////////////
void CCpuInfo::FillNameString()
{
	DWORD EAX, EBX, ECX, EDX;
	char name[49];
	name[48]='\0';
	int i=0;
	for( int j = 2; j <= 4; j++){
		cpuid(0x80000000 + j, &EAX, &EBX, &ECX, &EDX);
		memcpy(name + 4 * (i++), &EAX, 4);
		memcpy(name + 4 * (i++) ,&EBX, 4);
		memcpy(name + 4 * (i++), &ECX, 4);
		memcpy(name + 4 * (i++), &EDX, 4);
	}

	char *p;
	p = name;
	while(*p == ' '){p++;}

	sprintf(NameString,p);
}

//////////////////////////////////////////////////////////////////////
// Get Transmeta Information
//////////////////////////////////////////////////////////////////////
void CCpuInfo::FillTMx86Ex()
{
#ifndef _X86_64
	DWORD EAX, EBX, ECX, EDX;
	DWORD Max8086;

	cpuid(0x80860000, &EAX, &EBX, &ECX, &EDX);
	Max8086 = EAX;

	// Get Nominal Clock //
	if(Max8086 >= 0x80860000){
		cpuid(0x80860000, &EAX, &EBX, &ECX, &EDX);
		TmNominalClock = ECX;
	}

	if(Max8086 >= 0x80860002){
		cpuid(0x80860001, &EAX, &EBX, &ECX, &EDX);
		TmNominalClock = ECX;
		if(EBX != 0x20000000 ){
			sprintf(TmHardwareVersion,
				"%d.%d-%d.%d",
				(EBX>>24)&0xFF,
				(EBX>>16)&0xFF,
				(EBX>> 8)&0xFF,
				(EBX    )&0xFF);
		}
		cpuid(0x80860002, &EAX, &EBX, &ECX, &EDX);
		sprintf(TmSoftwareVersion,
			"%d.%d.%d-%d-%d",
			(EBX>>24)&0xFF,
			(EBX>>16)&0xFF,
			(EBX>> 8)&0xFF,
			(EBX    )&0xFF,
			ECX);
	}

	char name[65];
	name[0]='\0';
	if(Max8086 >= 0x80860006){
		int i=0;
		for( int j = 3; j <= 6; j++){
			cpuid(0x80860000 + j, &EAX, &EBX, &ECX, &EDX);
			memcpy(name + 4 * (i++), &EAX, 4);
			memcpy(name + 4 * (i++) ,&EBX, 4);
			memcpy(name + 4 * (i++), &ECX, 4);
			memcpy(name + 4 * (i++), &EDX, 4);
		}
		char *p;
		p = name;
		while(*p == ' '){p++;}
		sprintf(TmNameString,name);
	}

	// Get Current Clock // 
	if(Max8086 >= 0x80860007){
		cpuid(0x80860007, &EAX, &EBX, &ECX, &EDX);
		TmClock = EAX;
		TmCurrentVoltage = EBX;
		TmCurrentPerformance = ECX;
		TmCurrentGateDelay = EDX;
	}
#endif
}



//////////////////////////////////////////////////////////////////////
// Set Socket for K8
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetSocketK8()
{
	// preliminary
	if(FamilyX == 0x10){
		if(SocketID == 0x0){
			wsprintf(PlatformName, "Socket S1"); // Socket S1g1
		}else if(SocketID == 0x1){
			wsprintf(PlatformName, "Socket F+");
		}else if(SocketID == 0x3){
			wsprintf(PlatformName, "Socket AM2+");
		}else{
			wsprintf(PlatformName, "");
		}
		return ;
	}

	if(ModelEx >= 4){
		if(SocketID == 0x0){
			wsprintf(PlatformName, "Socket S1"); // Socket S1g1
		}else if(SocketID == 0x1){
			wsprintf(PlatformName, "Socket F");
		}else if(SocketID == 0x3){
			wsprintf(PlatformName, "Socket AM2");
		}else{
			wsprintf(PlatformName, "");
		}
		return ;
	}

	switch(ExModelX)
	{
	case 0x5:
	case 0x15:
	case 0x25:
	case 0x21:
		wsprintf(PlatformName, "Socket 940");
		break;
	case 0x23:
	case 0x33:
	case 0x07:
	case 0x17:
	case 0x27:
	case 0x37:
	case 0x0B:
	case 0x1B:
	case 0x2B:
	case 0x3B:
	case 0x0F:
	case 0x1F:
	case 0x2F:
	case 0x3F:
		wsprintf(PlatformName, "Socket 939");
		break;
	case 0x04:
	case 0x14:
	case 0x24:
	case 0x08:
	case 0x18:
	case 0x28://
	case 0x0C:
	case 0x1C:
	case 0x2C://
	case 0x0E:
	case 0x1E:
	case 0x2E://
		wsprintf(PlatformName, "Socket 754");
		break;
	default:
		break;
	}
}

void CCpuInfo::SetCoreRevision(double D, double T, char* CR)
{
	DieSize = D;
	Transister = T;
	wsprintf(CoreRevision, CR);
}

//////////////////////////////////////////////////////////////////////
// Set Core Revision for AMD
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetCoreRevisionAMD()
{
	if(Family == 0xF){
		switch(VersionEx)
		{
		case 0x000000F40:
		case 0x000000F50:
			SetCoreRevision(193.0, 105.9, "SH-B0");
			break;
		case 0x00000F51:
			SetCoreRevision(193.0, 105.9, "SH-B3");
			break;
		case 0x00000F48:
		case 0x00000F58:
			SetCoreRevision(193.0, 105.9, "SH-C0");
			break;
		case 0x00000F4A://?
		case 0x00000F5A:
		case 0x00000FF4:
		case 0x00000F7A:
			SetCoreRevision(193.0, 105.9, "SH-CG");
			break;
		case 0x00000FC0:
		case 0x00000FE0:
		case 0x00000FF0:
			SetCoreRevision(144.0, 68.5, "DH-CG");
			break;
		case 0x00000F82:
		case 0x00000FB2:
			SetCoreRevision(144.0, 68.5, "CH-CG");
			break;
		case 0x00010F50:
		case 0x00010F40:
		case 0x00010F70:
			SetCoreRevision(84.0, 68.5, "SH-D0");
			break;
		case 0x00010FC0:
		case 0x00010FF0:
			SetCoreRevision(84.0, 68.5, "DH-D0");
			break;
		case 0x00010F80:
		case 0x00010FB0:
			SetCoreRevision(84.0, 68.5, "CH-D0");
			break;
		case 0x00020F10:
			SetCoreRevision(205.0, 233.0, "JH-E1");
			break;
		case 0x00020FF0:
		case 0x00020FC0:
			SetCoreRevision(84.0, 76.0, "DH-E3");
			break;
		case 0x00020F51:
		case 0x00020F71:
			SetCoreRevision(115.0, 114.0, "SH-E4");
			break;
		case 0x00020FB1:
			SetCoreRevision(147.0, 154.0, "BH-E4");
			break;
		case 0x00020F42:
			SetCoreRevision(115.0, 114.0, "BH-E5");
			break;
		case 0x00020FF2:
		case 0x00020FC2:
			SetCoreRevision(84.0, 76.0, "DH-E6");
			break;
		case 0x00020F12:
		case 0x00020F32:
			SetCoreRevision(205.0, 233.0, "JH-E6");
			break;
		case 0x00030F72:
			SetCoreRevision(115.0, 114.0, "E6");
			break;
		case 0x00040F12:
		case 0x00040F32:
			SetCoreRevision(220.0, 243.0, "JH-F2");
			break;
		case 0x00040F82:
			SetCoreRevision(147.0, 154.0, "BH-F2");
			break;
		case 0x00040FB2:
			SetCoreRevision(183.0, 153.8, "BH-F2");
			break;
		case 0x00040FF2:
			SetCoreRevision(81.0, 103.0, "DH-F2");
			break;
		case 0x00050FF2:
			SetCoreRevision(-1.0, -1.0, "DH-F2");
			break;
		case 0x00040F13:
		case 0x00040F33:
		case 0x000C0F13:
			SetCoreRevision(-1.0, -1.0, "JH-F3");
			break;
		case 0x00040FC2:
			SetCoreRevision(-1.0, -1.0, "DH-F2");
			break;
		case 0x00050FF3:
			SetCoreRevision(-1.0, -1.0, "DH-F3");
			break;
		case 0x00060FB1:
		case 0x00060F81:
			SetCoreRevision(-1.0, -1.0, "BH-G1");
			break;
		case 0x00060FB2:
		case 0x00060F82:
			SetCoreRevision(-1.0, -1.0, "BH-G2");
			break;
		case 0x00070FF1:
			SetCoreRevision(-1.0, -1.0, "DH-G1");
			break;
		case 0x00060FF2:
		case 0x00060FC2:
		case 0x00070FC2:
			SetCoreRevision(-1.0, -1.0, "DH-G2");
			break;
		default:
			wsprintf(CoreRevision, "");
			break;
		}
	}else{
		switch(Version & 0xFFF)
		{
		case 0x561:	SetCoreRevision(162.0, 8.8, "B");	break;
		case 0x562:	SetCoreRevision(162.0, 8.8, "C");	break;
		case 0x570:	SetCoreRevision( 68.0, 8.8, "A");	break;
		case 0x580:	SetCoreRevision( 81.0, 9.3, "A");	break;
		case 0x58C:	SetCoreRevision( 81.0, 9.3, "C");	break;
		case 0x590:	SetCoreRevision(118.0,21.3, "A");	break;
		case 0x591:	SetCoreRevision(118.0,21.3, "B");	break;
		
		case 0x611:	SetCoreRevision(184.0, 22.0, "C1");	break;
		case 0x612:	SetCoreRevision(184.0, 22.0, "C2");	break;
		case 0x621:	SetCoreRevision(102.0, 22.0, "A1");	break;
		case 0x622:	SetCoreRevision(102.0, 22.0, "A2");	break;
		case 0x630:	SetCoreRevision(100.0, 25.0, "A0");	break;
		case 0x631:	SetCoreRevision(100.0, 25.0, "A2");	break;
		case 0x642:	SetCoreRevision(120.0, 37.0, "A4");	break;
		case 0x644:	SetCoreRevision(120.0, 37.0, "A9");	break;
		case 0x660:	SetCoreRevision(130.0, 37.5, "A0");	break;
		case 0x661:	SetCoreRevision(130.0, 37.5, "A2");	break;
		case 0x662:	SetCoreRevision(130.0, 37.5, "A5");	break;
		case 0x670:	SetCoreRevision(106.0, 25.2, "A0");	break;
		case 0x671:	SetCoreRevision(106.0, 25.2, "A1");	break;
		case 0x680:	SetCoreRevision( 81.0, 37.5, "A0");	break;
		case 0x681:	SetCoreRevision( 85.0, 37.5, "B1");	break;
		case 0x6A0:	SetCoreRevision(101.0, 54.3, "A2");	break;
		default:
			wsprintf(CoreRevision, "");
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Set Core Revision for VIA
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetCoreRevisionVIA()
{
	switch(Version & 0xFFF)
	{
	case 0x520:	SetCoreRevision(-1.0, 3.0, "");		break; /*394.0mm^2 or 225.0mm^2*/
	case 0x530:	SetCoreRevision(169.0, 3.0, "");	break;
	case 0x600:	SetCoreRevision(197.0, 6.0, "");	break;
	case 0x601:	SetCoreRevision(156.0, 6.0, "");	break;
	case 0x651:	SetCoreRevision(-1.0, 6.0, "");		break; /*65.0mm^2 or 88.0mm^2*/
	case 0x660:	SetCoreRevision(75.0, 11.3, "S1");	break;
	case 0x670:	
	case 0x671:	
	case 0x672:	SetCoreRevision(52.0, 15.2, "S2");	break;
	case 0x678:	SetCoreRevision(52.0, 15.4, "E");	break;
	case 0x680:
	case 0x689:	SetCoreRevision(47.0, 15.5, "ET");	break;
	case 0x691:	
	case 0x693:	SetCoreRevision(47.0, 20.5, "N");	break;
	case 0x694:
	case 0x695:
	case 0x696:	
	case 0x697:	SetCoreRevision(52.0, 20.5, "N");	break;
	case 0x698:	SetCoreRevision(47.0, 20.4, "A");	break;
	default:
		wsprintf(CoreRevision, "");
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// Set Core Revision for Intel
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetCoreRevisionIntel()
{
	switch(Version & 0xFFF)
	{
	case 0x517:	SetCoreRevision(294.0, 3.1, "D1");	break;
	case 0x525:	SetCoreRevision( -1.0, 3.2, "C2");	break; /*148.0 or 91.0*/
	case 0x52C:	SetCoreRevision( 81.0, 3.3, "C2");	break;
	case 0x51A:	SetCoreRevision( 91.0, 3.3, "tA0");	break;
	case 0x543:	SetCoreRevision(128.0, 4.5, "B1");	break;
	case 0x544:	SetCoreRevision(140.0, 4.5, "A3");	break;
	case 0x581:	SetCoreRevision( 90.0, 4.5, "A0");	break;
	case 0x582:	SetCoreRevision( 90.0, 4.5, "B2");	break;

	case 0x611:	SetCoreRevision(306.0,  5.5, "B0");	break;
	case 0x612:	SetCoreRevision(306.0,  5.5, "C0");	break;
	case 0x616:	SetCoreRevision(195.0,  5.5, "sA0");break;
	case 0x617:	SetCoreRevision(195.0,  5.5, "sA1");break;
	case 0x619:	SetCoreRevision(195.0,  5.5, "sB1");break;
	case 0x632:	SetCoreRevision( -1.0, -1.0, "TdB0");break;
	case 0x633:	SetCoreRevision(203.0,  7.5, "C0");	break;
	case 0x634:	SetCoreRevision(203.0,  7.5, "C1");	break;
	case 0x650:	SetCoreRevision(131.0,  7.5, "A0");	break;
	case 0x651:	SetCoreRevision(131.0,  7.5, "A1");	break;
	case 0x652:	SetCoreRevision( -1.0,  7.5, "B0");	break;/*118.0 or131.0*/
	case 0x653:	SetCoreRevision(118.0,  7.5, "B1");	break;
	case 0x660:	SetCoreRevision(154.0, 19.0, "mA0");break;
	case 0x665:	SetCoreRevision(154.0, 19.0, "mB0");break;
	case 0x66A:	SetCoreRevision( -1.0, -1.0, "A0");	break;/*180.0/27.4 or 154.0/18.9*/
	case 0x672:	SetCoreRevision(123.0,  9.5, "B0");	break;
	case 0x673:	SetCoreRevision(123.0,  9.5, "C0");	break;
	case 0x681:	SetCoreRevision(106.0, 28.0, "A2");	break;
	case 0x683:	SetCoreRevision(105.0, 28.0, "B0");	break;
	case 0x686:	SetCoreRevision( 90.0, 28.0, "C0");	break;
	case 0x68A:	SetCoreRevision( 95.0, 28.0, "D0");	break;
	case 0x695:	SetCoreRevision( 83.0, 77.0, "B1");	break;
	case 0x6A0:	SetCoreRevision(375.0,140.0, "A0");	break;
	case 0x6A1:	SetCoreRevision(375.0,140.0, "A1");	break;
	case 0x6B1:	SetCoreRevision( 80.0, 44.0, "A1");	break;
	case 0x6B4:	SetCoreRevision( 80.0, 44.0, "B1");	break;
	case 0x6D6:	SetCoreRevision( 87.0,144.0, "B1");	break;
	case 0x6D8:	SetCoreRevision( 87.0,144.0, "C0");	break;
	case 0x6E4:	SetCoreRevision( 90.0,151.0, "C0");	break;
	case 0x6E8:	SetCoreRevision( 90.0,151.0, "C0");	break;

	case 0xF07:	SetCoreRevision(217.0, 42.0, "B2");	break;
	case 0xF0A:	SetCoreRevision(217.0, 42.0, "C1");	break;
	case 0xF11:	SetCoreRevision( -1.0, 76.0, "C0");	break;
	case 0xF12:	SetCoreRevision(217.0, 42.0, "D0");	break;
	case 0xF13:	SetCoreRevision(217.0, 42.0, "E0");	break;
	case 0xF22:
		if(CacheL3 == 1024){
			SetCoreRevision( -1.0,110.0, "A0");
		}else if(CacheL3 == 2048){
			SetCoreRevision(237.0,169.0, "A0");
		}		
		break;
	case 0xF24:	SetCoreRevision(146.0, 55.0, "B0");	break;
	case 0xF25:
		if(CacheL3 == 1024){
			SetCoreRevision( -1.0,108.0, "B1");
		}else if(CacheL3 == 2048){
			SetCoreRevision(237.0,169.0, "M0");
		}else{
			SetCoreRevision(131.0, 55.0, "M0");
		}
		break;
	case 0xF26:
		if(CacheL3 == 2048){
			SetCoreRevision(237.0,169.0, "C0");
		}else{
			SetCoreRevision( -1.0, -1.0, "C0");
		}		
		break;
	case 0xF27:	SetCoreRevision(131.0, 55.0, "C1");	break;
	case 0xF29:	SetCoreRevision(131.0, 55.0, "D1");	break;
	case 0xF33:	SetCoreRevision( 81.0,125.0, "C0");	break;
	case 0xF34:	SetCoreRevision( 81.0,125.0, "D0");	break;
	case 0xF41:	SetCoreRevision( 81.0,125.0, "E0");	break;
	case 0xF43:	SetCoreRevision(135.0,169.0, "N0");	break;
	case 0xF44:	SetCoreRevision(206.0,230.0, "A0");	break;
	case 0xF47:	SetCoreRevision(206.0,230.0, "B0");	break;
	case 0xF49:	SetCoreRevision( 81.0,125.0, "G1");	break;
	case 0xF4A:	SetCoreRevision(135.0,169.0, "R0");	break;
	case 0xF62:	SetCoreRevision(280.0, -1.0, "B1");	break;
	case 0xF64:	SetCoreRevision( -1.0, -1.0, "C1");	break;

	default:
		wsprintf(CoreRevision, "");
		break;
	}
}
