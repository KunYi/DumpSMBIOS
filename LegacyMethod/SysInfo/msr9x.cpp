/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2002-2005 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "msr9x.h"

//////////////////////////////////////////////////////////////////////
// HLT
//////////////////////////////////////////////////////////////////////
__declspec(naked) void Ring0HLT()
{
#ifndef _X86_64
	_asm
	{
		hlt
		retf
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// RDMSR
//////////////////////////////////////////////////////////////////////
__declspec(naked) void Ring0ReadMSR()
{
#ifndef _X86_64
	_asm
	{
		rdmsr
		mov	[ebx],eax
		mov	[edi],edx
		retf
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// WRMSR
//////////////////////////////////////////////////////////////////////
__declspec(naked) void Ring0WriteMSR()
{
#ifndef _X86_64
	_asm
	{
		mov	eax,[ebx]
		mov	edx,[edi]
		wrmsr
		retf
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// This function makes it possible to call ring 0 code from a ring 3
// application.
//////////////////////////////////////////////////////////////////////
int CallRing0(PVOID pvRing0FuncAddr,ULONG ulECX,ULONG* pulEAX,ULONG* pulEDX)
{
#ifndef _X86_64
	GDT_DESCRIPTOR *pGDTDescriptor;
	GDTR gdtr;
	_asm sgdt [gdtr]
	
	// Skip the null descriptor
	pGDTDescriptor = (GDT_DESCRIPTOR *)(gdtr.dwGDTBase + 8);
	// Search for a free GDT descriptor
	for (WORD wGDTIndex = 1; wGDTIndex < (gdtr.wGDTLimit / 8); wGDTIndex++)
	{
		if (pGDTDescriptor->Type == 0     &&
			pGDTDescriptor->System == 0   &&
			pGDTDescriptor->DPL == 0      &&
			pGDTDescriptor->Present == 0)
		{
			// Found one !
			// Now we need to transform this descriptor into a callgate.
			// Note that we're using selector 0x28 since it corresponds
			// to a ring 0 segment which spans the entire linear address
			// space of the processor (0-4GB).
			CALLGATE_DESCRIPTOR *pCallgate;
			pCallgate =	(CALLGATE_DESCRIPTOR *) pGDTDescriptor;
			pCallgate->Offset_0_15 = LOWORD(pvRing0FuncAddr);
			pCallgate->Selector = 0x28;
			pCallgate->ParamCount =	0;
			pCallgate->Unused = 0;
			pCallgate->Type = 0xc;
			pCallgate->System = 0;
			pCallgate->DPL = 3;
			pCallgate->Present = 1;
			pCallgate->Offset_16_31 = HIWORD(pvRing0FuncAddr);
			// Prepare the far call parameters
			WORD CallgateAddr[3];
			CallgateAddr[0] = 0x0;
			CallgateAddr[1] = 0x0;
			CallgateAddr[2] = (wGDTIndex << 3) | 3;
			// Please fasten your seat belts!
			// We're about to make a hyperspace jump into RING 0.
			//   MessageBox(NULL,"CallGate","START",MB_OK);
			_asm mov ebx,pulEAX
			_asm mov edi,pulEDX
			_asm mov ecx,ulECX
			_asm call fword ptr [CallgateAddr]
			// We have made it !
			// Now free the GDT descriptor
			memset(pGDTDescriptor, 0, 8);
			// Our journey was successful. Seeya.
			return true;
		}
		// Advance to the next GDT descriptor
		pGDTDescriptor++; 
	}
	// Whoops, the GDT is full
#endif
	return false;
}