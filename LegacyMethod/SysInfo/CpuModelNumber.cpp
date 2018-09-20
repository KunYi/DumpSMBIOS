/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "CpuInfo.h"
/*
//////////////////////////////////////////////////////////////////////
// Set Model Number for Pentium 4/D / Celeron D (P4 core)
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetModelNumberP4()
{

}

//////////////////////////////////////////////////////////////////////
// Set Model Number for Pentium M / Celeron M (P6 core)
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetModelNumberP6()
{

}

//////////////////////////////////////////////////////////////////////
// Set Model Number for Core/Core 2
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetModelNumberCore2()
{

	if(PhysicalCoreNum == 4){
		if(ClockOri == 2400.00){
			wsprintf(ModelNumber, " Q6600");
		}else if(ClockOri == 2666.66){
			wsprintf(ModelNumber, " Q6700");			
		}
	}else if(PhysicalCoreNum == 2){

	}else{

	}

}
*/
//////////////////////////////////////////////////////////////////////
// Set Model Number for K8
//////////////////////////////////////////////////////////////////////
void CCpuInfo::SetModelNumberK8()
{
	if(ModelEx >=4){
		if(SocketID == 0x0){ // Socket S1g1
			switch(BrandID)
			{
			case 0x2:
				if(PhysicalCoreNum == 2){
					wsprintf(ModelNumber, " TL-%d", 29 + BrandIDNN);
				}else{
					wsprintf(ModelNumber, " MK-%d", 29 + BrandIDNN);
				}
				break;
			case 0x3:
				break;
			default:
				break;
			}
		}else if(SocketID == 0x1){ // Socket F
			switch(BrandID)
			{
			case 0x1:
				wsprintf(ModelNumber, " 22%d", -1 + BrandIDNN);
				if(PwrLmt == 0x6){
					strcat(ModelNumber, " HE");
				}else if(PwrLmt == 0xC){
					strcat(ModelNumber, " SE");
				}
				break;
			case 0x4:
				wsprintf(ModelNumber, " 82%d", -1 + BrandIDNN);
				if(PwrLmt == 0x6){
					strcat(ModelNumber, " HE");
				}else if(PwrLmt == 0xC){
					strcat(ModelNumber, " SE");
				}
				break;
			default:
				break;
			}
		}else{ //  if(SocketID == 0x3) Socket AM2
			switch(BrandID)
			{
			case 0x1:
				wsprintf(ModelNumber, " 12%d", -1 + BrandIDNN);
				if(PwrLmt == 0x6){ // undefined
					strcat(ModelNumber, " HE");
				}else if(PwrLmt == 0xC){
					strcat(ModelNumber, " SE");
				}
				break;
			case 0x4: // Athlon 64, Athlon 64 X2
				if(BrandIDNN < 9){ // < Athlon 64 X2 3400+
					BrandIDNN += 32; // bit15 = 1
				}
				wsprintf(ModelNumber, " %d00+", 15 + CmpCap * 10 + BrandIDNN);
				break;
			case 0x6: // Sempron
				wsprintf(ModelNumber, " %d00+", 15 + CmpCap * 10 + BrandIDNN);
				break;
			case 0x5: // Athlon 64 FX
				wsprintf(ModelNumber, "-%d", 57 + BrandIDNN);
				break;
			default:
				strcpy(NameSysInfo, "Engineering Sample");
				break;
			}
		}
		return ;
	}

	switch(BrandID)
	{
	case 0:
		break;
	case 0x6:
		if(MultiplierOri == 14.0){
			wsprintf(ModelNumber, "-62");
		}else if(MultiplierOri == 13.0){
			wsprintf(ModelNumber, "-60");
		}else{
			wsprintf(ModelNumber, "");
		}
		break;

	/* Type XX */
	case 0x4:
	case 0x5:
	case 0x8:
	case 0x9:
	case 0x1D:
	case 0x1E:
	case 0x20:
		wsprintf(ModelNumber, " %d00+", BrandIDNN + 22);
		break;
	case 0xA:
	case 0xB:
		wsprintf(ModelNumber, "-%d", BrandIDNN + 22);
		break;

	/* Type YY */
	case 0xC:
	case 0xD:
	case 0xE:
	case 0xF:
		wsprintf(ModelNumber, " 1%d", 2 * BrandIDNN + 38);
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		wsprintf(ModelNumber, " 2%d", 2 * BrandIDNN + 38);
		break;
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		wsprintf(ModelNumber, " 8%d", 2 * BrandIDNN + 38);
		break;
	
	/* Type EE */
	case 0x18:
		wsprintf(ModelNumber, " %d00+", BrandIDNN + 9);
		break;

	/* Type TT */
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x26:
		wsprintf(ModelNumber, " %d00+", BrandIDNN + 24);
		break;

	/* Type ZZ */
	case 0x24:
		wsprintf(ModelNumber, "-%d", BrandIDNN + 24);
		break;

	/* Type RR */
	case 0x29:
	case 0x2C:
	case 0x2D:
	case 0x2E:
	case 0x2F:
	case 0x38:
		wsprintf(ModelNumber, " 1%d", 5 * BrandIDNN + 45);
		break;
	case 0x2A:
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x39:
		wsprintf(ModelNumber, " 2%d", 5 * BrandIDNN + 45);
		break;
	case 0x2B:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x3A:
		wsprintf(ModelNumber, " 8%d", 5 * BrandIDNN + 45);
		break;
	default:
		wsprintf(ModelNumber, "");
		break;
	}
}