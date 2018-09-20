;; CPUID.ASM - by NujiNuji (MikoMiko)
;; $Id$


	.code
;; typedef unsigned int uint32;
;; typedef unsigned __int64 uint64;
;; assert(sizeof(uint32)*8 == 32);
;; assert(sizeof(uint64)*8 == 64);

;; void __fastcall cpuid(uint32 dwOP, uint32 *lpAX, uint32 *lpBX, uint32 *lpCX, uint32 *lpDX);
_cpuid	PROC
	PUSH	RBP
	MOV	RBP, RSP

	;; RSP+30H [lpDX]
	;; RSP+28H [ROOM FOR lpCX]
	;; RSP+20H [ROOM FOR lpBX]
	;; RSP+18H [ROOM FOR lpAX]
	;; RSP+14H [PADDING FOR dwOP]
	;; RSP+10H [ROOM FOR dwOP]
	;; RSP+08H [RETURN-ADDRESS]
	;; RSP+00H [RBP]
	;; RBP == RSP
	MOV	DWORD PTR [RBP+10H], ECX
	MOV	DWORD PTR [RBP+14H], 0
	MOV	QWORD PTR [RBP+18H], RDX
	MOV	QWORD PTR [RBP+20H], R8
	MOV	QWORD PTR [RBP+28H], R9

	;; RCX, RDX are volatile
	PUSH	RBX
	PUSH	RDI

	MOV	EAX, DWORD PTR [RBP+10H]
	MOV	ECX, 0

	CPUID

	;; EDI = lpAX;
	MOV	RDI, QWORD PTR [RBP+18H]
	;; *EDI = EAX;
	MOV	DWORD PTR [RDI], EAX

	;; *lpBX = EBX;
	MOV	RDI, QWORD PTR [RBP+20H]
	MOV	DWORD PTR [RDI], EBX

	;; *lpCX = ECX;
	MOV	RDI, QWORD PTR [RBP+28H]
	MOV	DWORD PTR [RDI], ECX

	;; *lpDX = EDX
	MOV	RDI, QWORD PTR [RBP+30H]
	MOV	DWORD PTR [RDI], EDX

	POP	RDI
	POP	RBX

	XOR	RAX, RAX
	POP	RBP
	RET
_cpuid	ENDP


	PUBLIC	_cpuid

	END
;; THE END OF FILE
