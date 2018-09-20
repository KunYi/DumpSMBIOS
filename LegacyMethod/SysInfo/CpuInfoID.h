/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#ifndef __CPU_INFO_ID_H__
#define __CPU_INFO_ID_H__

////////////
// Error Rate
//////////////////////////////////////////
#define ERROR_RATE_MAX	1.005 
#define ERROR_RATE_MIN	0.995 

////////////
// URL
//////////////////////////////////////////

#define URL_JAPANESE	"http://crystalmark.info/"
#define URL_ENGLISH		"http://crystalmark.info/?lang=en"

////////////
// Version Information
//////////////////////////////////////////

#ifdef _X86_64
#define		CRYSTAL_CPUID_PRODUCT		"CrystalCPUID Pure x64 Edition"	// Max 32
#else if
#define		CRYSTAL_CPUID_PRODUCT		"CrystalCPUID"			// Max 32
#endif
#define		CRYSTAL_CPUID_VERSION		"4.15.0"				// Max 16
#define		CRYSTAL_CPUID_STATUS		""						// Alpha or Beta
#define		CRYSTAL_CPUID_DATE			"2008/7/6"				// Max 10
#define		CRYSTAL_CPUID_AUTHOR		"hiyohiyo"				// Max 24

// If you modify CrystalCPUID. Please write your name this section.
//#define		CRYSTAL_CPUID_MODIFIED		"YOUR NAME"		// Max 24
#define		CRYSTAL_CPUID_MODIFIED		""						// Max 24

////////////
// CpuInfo.dll Status
/////////////////////////////////////////

#define		SI_VERSION					0x0001
#define		SI_DATE						0x0002
#define		SI_AUTHOR					0x0003

#define		SI_STATUS					0x0010
#define		SI_MEASURE_TIME				0x0020

////////////
// CPU
//////////////////////////////////////////

#define		CPU_BASE					0x1000

// String //
#define		CPU_NAME_SYS_INFO			0x1001	// SysInfo.dll Set CPU Name 
#define		CPU_FSB_MODE				0x1002	// ""=SDR "DDR" "QDR"
#define		CPU_VENDOR_STRING			0x1003	// OriginalVendorString
#define		CPU_VENDOR_NAME				0x1004	// Vendor Name
#define		CPU_NAME_STRING				0x1005	// CPUID EAX=0x8000:0003-0005
#define		CPU_PLATFORM_NAME			0x1006	// Soket/Slot Info
#define		CPU_TYPE_NAME				0x1007	// OEM Multi OverDrive
#define		CPU_FMS						0x1008	// Family Model Stepping
#define		CPU_CACHE_SPEED_STR			0x1009	// L2 Cache Speed info
#define		CPU_NAME					0x100A	// CPU Name
#define		CPU_CODE_NAME				0x100B	// Code Name
#define		CPU_FULL_NAME				0x100C	// Vendor Name + CPU Name
#define		CPU_PROCESS_RULE			0x100D	// Process Rule
#define		CPU_PROCESSOR_SERIAL		0x100E	// Intel Processor Serial Number
#define		CPU_ARCHITECTURE			0x100F	// Architecture
#define		CPU_MEASURE_MODE			0x1010	// Measure Mode
#define		CPU_MODEL_NUMBER			0x1011	// Model Number
#define		CPU_CORE_REVISION			0x1012	// Core Revision
#define		CPU_LOGO					0x1013	// Logo

// dobule //
#define		CPU_CLOCK					0x1020
#define		CPU_SYSTEM_CLOCK			0x1021
#define		CPU_SYSTEM_BUS				0x1022
#define		CPU_MULTIPLIER				0x1023
#define		CPU_CLOCK_ORI				0x1024
#define		CPU_SYSTEM_CLOCK_ORI		0x1025
#define		CPU_SYSTEM_BUS_ORI			0x1026
#define		CPU_CACHE_SPEED				0x1027	// L2 Cache Speed info
#define		CPU_OVER_CLOCK				0x1028
#define		CPU_MULTIPLIER_ORI			0x1029
#define		CPU_CLOCK_UPDATE			0x102A
#define		CPU_MULTIPLIER_UPDATE		0x102B
#define		CPU_CLOCK_UPDATE_WT			0x102C
#define		CPU_CLOCK_UPDATE_MMT		0x102D
#define		CPU_CLOCK_UPDATE_QPC		0x102E
#define		CPU_CLOCK_UPDATE_NOLOAD		0x102F	// No load (QPF)
#define		CPU_DIE_SIZE				0x10C0
#define		CPU_TRANSISTER				0x10C1
#define		CPU_K8_HYPER_TRANSPORT		0x10C2

// integer //
#define		CPU_NUMBER					0x1030
#define		CPU_VIRTUAL_NUMBER			0x1030
#define		CPU_FAMILY					0x1031
#define		CPU_MODEL					0x1032
#define		CPU_STEPPING				0x1033
#define		CPU_FAMILY_EX				0x1034
#define		CPU_MODEL_EX				0x1035
#define		CPU_BRAND_ID				0x1036
#define		CPU_FEATURE					0x1037	// CPUID EAX=0x00000001  EDX
#define		CPU_FEATURE_EX				0x1038	// CPUID EAX=0x80000001  EDX			
#define		CPU_CACHE_L1I				0x1039	
#define		CPU_CACHE_L1T				0x103A	// NetBurst micro-ops
#define		CPU_CACHE_L1D				0x103B
#define		CPU_CACHE_L2				0x103C
#define		CPU_CACHE_L3				0x103D
#define		CPU_HYPER_THREAD_NUM		0x103E
#define		CPU_TYPE					0x103F	// OEM Multi OverDrive
#define		CPU_RDMSR_EAX				0x1040
#define		CPU_RDMSR_EAX_1				0x1040
#define		CPU_RDMSR_EAX_2				0x1041
#define		CPU_FSB_MULTIPLIER			0x1042
#define		CPU_CACHE_L1U				0x1043	
#define		CPU_VERSION					0x1044	// CPUID EAX=0x00000001  EAX
#define		CPU_VERSION_EX				0x1045	// CPUID EAX=0x80000001  EAX
#define		CPU_FLAG_BRAND				0x1046
#define		CPU_APIC					0x1047
#define		CPU_RDMSR_EDX_1				0x1048
#define		CPU_RDMSR_EDX_2				0x1049
#define		CPU_PHYSICAL_NUMBER			0x104A
#define		CPU_FEATURE_ECX				0x104B	// CPUID EAX=0x00000001  ECX
#define		CPU_VENDOR_ID				0x104C
#define		CPU_PLATFORM_ID				0x104D
#define		CPU_MICROCODE_ID			0x104E
#define		CPU_TYPE_ID					0x104F
#define		CPU_PHYSICAL_CORE_NUM		0x10B0
#define		CPU_FAMILY_X				0x10B1
#define		CPU_MODEL_X					0x10B2
#define		CPU_EX_FAMILY_X				0x10B3
#define		CPU_EX_MODEL_X				0x10B4
#define		CPU_CACHE_L2_NUMBER			0x10B5

#define		CPU_EX_FAMILY				0x10A0
#define		CPU_EX_MODEL				0x10A1
#define		CPU_EX_STEPPING				0x10A2
#define		CPU_FEATURE_EX_ECX			0x10A3	// CPUID EAX=0x80000001  ECX			

// flag //
#define		CPU_FLAG_MMX				0x1050
#define		CPU_FLAG_MMX_EX				0x1051
#define		CPU_FLAG_SSE				0x1052
#define		CPU_FLAG_SSE2				0x1053
#define		CPU_FLAG_3DNOW				0x1054
#define		CPU_FLAG_3DNOW_EX			0x1055
#define		CPU_FLAG_HYPER_THREAD		0x1056
#define		CPU_FLAG_HTT				0x1056
#define		CPU_FLAG_AA64				0x1057
#define		CPU_FLAG_IA64				0x1058
#define		CPU_FLAG_SSE3				0x1059
#define		CPU_FLAG_PROCESSOR_SERIAL	0x105A
#define		CPU_FLAG_SPEED_STEP			0x105B
#define		CPU_FLAG_POWER_NOW			0x105C
#define		CPU_FLAG_LONG_HAUL			0x105D
#define		CPU_FLAG_LONG_RUN			0x105E
#define		CPU_FLAG_IA32E				0x105F
#define		CPU_FLAG_SSE4				0x10D0
#define		CPU_FLAG_AMD_V				0x10D1
#define		CPU_FLAG_SSSE3				0x10D2
#define		CPU_FLAG_SSE41				0x10D3
#define		CPU_FLAG_SSE42				0x10D4
#define		CPU_FLAG_SSE4A				0x10D5
#define		CPU_FLAG_SSE5				0x10D6
#define		CPU_FLAG_AVX				0x10D7

#define		CPU_FLAG_NX					0x1090
#define		CPU_FLAG_MSR				0x1091
#define		CPU_FLAG_DUAL_CORE			0x1092
#define		CPU_FLAG_EIST				0x1093
#define		CPU_FLAG_VT					0x1094
#define		CPU_FLAG_EIST_CORRECT		0x1095
#define		CPU_FLAG_K8_100MHZ_STEPS	0x1096		
#define		CPU_FLAG_K8_UNDER_1100V		0x1097

// for Transmeta // 
#define		CPU_TM_NAME_STRING			0x1060
#define		CPU_TM_CLOCK				0x1061
#define		CPU_TM_NOMINAL_CLOCK		0x1062
#define		CPU_TM_CURRENT_VOLTAGE		0x1063
#define		CPU_TM_CURRENT_PERFORMANCE	0x1064
#define		CPU_TM_CURRENT_GATE_DELAY	0x1065
#define		CPU_TM_HARDWARE_VERSION		0x1066
#define		CPU_TM_SOFTWARE_VERSION		0x1067
#define		CPU_TM_UPDATE				0x1068

#define		CPU_FEATURE_VIA             0x1070
#define		CPU_FEATURE_TRANSMETA       0x1071
#define		CPU_FEATURE_PM              0x1072

// for Cache
#define		CPU_CACHE_L1ITU_WAYS		0x1080
#define		CPU_CACHE_L1ITU_LINES		0x1081
#define		CPU_CACHE_L1D_WAYS			0x1082
#define		CPU_CACHE_L1D_LINES			0x1083
#define		CPU_CACHE_L2_WAYS			0x1084
#define		CPU_CACHE_L2_LINES			0x1085
#define		CPU_CACHE_L3_WAYS			0x1086
#define		CPU_CACHE_L3_LINES			0x1087

// Update CPU Information
#define		CPU_UPDATE_BASE				0x1100

// Tuning
#define		LH_BASE						0x1200
#define		LH_GET_CURRENT_STATUS		0x1200
#define		LH_GET_TYPE					0x1201
#define		LH_GET_TABLE_TYPE			0x1202
#define		LH_RESET_FVID_FLAG			0x1203

#define		LH_GET_CURRENT_FID			0x1211
#define		LH_GET_STARTUP_FID			0x1212
#define		LH_GET_MAX_FID				0x1213
#define		LH_GET_CURRENT_MULTIPLIER	0x1214
#define		LH_GET_STARTUP_MULTIPLIER	0x1215
#define		LH_GET_MAX_MULTIPLIER		0x1216
#define		LH_SET_FID					0x1217
#define		LH_SET_FVID					0x1218
#define		LH_SET_EIST_CORRECT			0x1219

#define		LH_GET_CURRENT_VID			0x1221
#define		LH_GET_STARTUP_VID			0x1222
#define		LH_GET_MAX_VID				0x1223
#define		LH_GET_CURRENT_VOLTAGE		0x1224
#define		LH_GET_STARTUP_VOLTAGE		0x1225
#define		LH_GET_MAX_VOLTAGE			0x1226
#define		LH_SET_VID					0x1227
#define		LH_SET_K7_DESKTOP			0x1228
#define		LH_SET_K8_LOW_VOLTAGE		0x1229
#define		LH_GET_MIN_VID				0x122A
#define		LH_GET_MIN_VOLTAGE			0x122B

#define		SET_VIA_VT_310DP_PIPELINE	0x1300

////////////
// CPU Vendor ID
//////////////////////////////////////////

#define		INTEL						1
#define		AMD							2
#define		TMx86						3
#define		CYRIX						4
#define		IDT							5
#define		SIS							6
#define		UMC							7
#define		RISE						8
#define		NEXGEN						9
#define		COMPAQ					   10
#define		NSC						   11

////////////
// System Bus & Multipler Table
//////////////////////////////////////////

#define		GENERAL					    0
#define		P6						    1
#define		COPPERMINE				    3
#define		TUALATIN				    4
#define		WILLAMETTE					5
#define		NORTHWOOD					6
#define		BANIAS						7
#define		DOTHAN						8
#define		PENRYN						9


#define		K6						   10
#define		ATHLON					   11
#define		MOBILE_ATHLON			   12
#define		ATHLON_64				   13
#define		MOBILE_ATHLON_64		   14
#define		GEODE_LX				   15
#define		K10						   16

#define		CYRIX3					   20 // CYRIX3 = SAMUEL
#define		SAMUEL					   20
#define		SAMUEL2					   21
#define		EZRA					   22
#define		EZRA_T					   23
#define		NEHEMIAH				   24
#define		ESTHER					   25

////////////
// LongHole (VIA C3 Multiplier Change)
//////////////////////////////////////////
#define		LONG_HAUL_LEVEL_1			1 // Samuel  & Samuel2 & Ezra
#define		LONG_HAUL_LEVEL_2			2 // Ezra-T & VIA C3 Nehemiah
#define		LONG_HAUL_LEVEL_3			3 // Ester

////////////
// Love Hammer (AMD K7/K8 Multiplier Change)
//////////////////////////////////////////
#define		LOVE_HAMMER_GEODE_LX		5
#define		LOVE_HAMMER_K6				6
#define		LOVE_HAMMER_K7				7
#define		LOVE_HAMMER_K8				8

////////////
// Speed Step
//////////////////////////////////////////
#define		SPEED_STEP_PM				86
#define		SPEED_STEP_P4				87
#define		SPEED_STEP_CORE_MA			88	// CoreDuo/Core2Duo 65nm
#define		SPEED_STEP_PENRYN			89	// Core2Duo 45nm

////////////
// Language
//////////////////////////////////////////

#define		JAPANESE				0x0001
#define		ENGLISH					0x0002

#endif // __CPU_INFO_ID_H__
