////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <stddef.h>
#include <assert.h>

#include "Pcidebug.h"
#include "driverload.h"

int GetFileInternalName(const char *file,char *name);

#include "resource.h"

void InitializeDll();
void DeinitializeDll();

int initialize(HMODULE ghInst);
int initialize_vxd(HMODULE ghInst);
void close(void);

extern void init_isr(void);

char DRIVERID[MAX_PATH];		//"CrystalSysInfo"
char DRIVERFILENAME[MAX_PATH];	//"SysInfo.sys" or SysInfoX64.sys
char DRIVERNAME[MAX_PATH];		//"CrystalSysInfo"
#define DRIVERFILENAME95 "SysInfo.vxd"

HINSTANCE g_hModule = NULL;
HANDLE handle = NULL;
int drivertype = 0;

// DLLの状態を保存する変数
static DLLSTATUS status = DLLSTATUS_OTHERERROR;
//char DRIVERID[MAX_PATH];

//
// OSの種類を調べる。
//
// 戻り値　　　　0: 取得できなかった
//　　　　　　　　　　　　1: windows 3.1
//　　　　　　　　　　　　2: windows 95
//　　　　　　　　　　　　3: windows NT
//　　　　　　　　　　　　4: windows NT x64
//　　　　　　　　　　　　-1:その他
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


//--------------------------

int oscheck()
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(os);
	if(GetVersionEx(&os) == 0){
		assert(0);
		return 0;
	}
	switch(os.dwPlatformId){
	case VER_PLATFORM_WIN32s:
		return 1;
	case VER_PLATFORM_WIN32_WINDOWS:
		return 2;
	case VER_PLATFORM_WIN32_NT:
#ifdef _X86_64
		return 4;
#else
		return IsWow64() ? 4 : 3;	//Windowx XP 64Bit Edition or Windows NT
#endif
	default:
		return -1;
	}
}

void InitializeDll()
{
	char str[MAX_PATH];
	char name[64];
	int ostype, res, i;
	OSVERSIONINFO os;

	drivertype = 0;
	ostype = oscheck();
	if(!(ostype >= 2 && ostype <= 4)) return ;
	GetModuleFileName(NULL,str,MAX_PATH);
	GetFileInternalName(str,name);

	os.dwOSVersionInfoSize = sizeof(os);
	if(GetVersionEx(&os) == 0){
		assert(0);
		return ;
	}

	switch(ostype)
	{
	case 4:// NT x64
	case 3:// NT
		strcpy(DRIVERID,"CrystalSysInfo");
		if(ostype==4){
			strcpy(DRIVERFILENAME, "SysInfoX64.sys");
		}else{
			if(os.dwMajorVersion==4){
				strcpy(DRIVERFILENAME, "SysInfoNT4.sys");
			}else{
				strcpy(DRIVERFILENAME, "SysInfo.sys");
			}
		}
//		strcpy(DRIVERFILENAME, (ostype==4) ? "SysInfoX64.sys" : "SysInfo.sys");
		
		strcpy(DRIVERNAME,"\\\\.\\CrystalSysInfo");
		drivertype |= SYS_DRIVER_USE;
		status = DLLSTATUS_NOERROR;
		break;
	case 2:// 95 98の場合
		drivertype |= VXD_DRIVER_USE;
		drivertype |= PCIBIOS_READY;
		drivertype |= LIB_IO_FUNC_USE;
		status = DLLSTATUS_NOERROR;
		break;
	default:// NT 95/98以外の場合
		status = DLLSTATUS_NOTSUPPORTEDPLATFORM;
		break;
	}
	if(status == DLLSTATUS_NOTSUPPORTEDPLATFORM) return ;

	init_isr();
	if(drivertype & SYS_DRIVER_USE) {
		for(i=0;i<16;i++){
			res = initialize(g_hModule);
			if(res == 0){break;}
			Sleep(100);
		}
	} else if(drivertype & VXD_DRIVER_USE) {
		res = initialize_vxd(g_hModule);
	} else {
		res = 3;
	}
	switch ( res ){
	case 0:
		status = DLLSTATUS_NOERROR;
		break;
	case 2:
		handle = NULL;
		status = DLLSTATUS_DRIVERNOTLOADED;
		break;
	default:
		handle = NULL;
		status = DLLSTATUS_OTHERERROR;
		break;
	}
}

void DeinitializeDll()
{
	close();
	status = DLLSTATUS_OTHERERROR;
}


///////////////////////////////////////////////////////////
//
// DLLがLOADされたときにデバイスドライバーを登録、開始する
// DLLがUNLOADされたときにデバイスドライバーを停止取り外す
//
///////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD ul_reason_for_call, 
					  LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
	case DLL_PROCESS_ATTACH:
		g_hModule = hModule;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

BOOL Opendriver(void)
{
	DWORD	dwStatus;

	handle = CreateFile(
		DRIVERNAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

	dwStatus = GetLastError();

    if (handle == INVALID_HANDLE_VALUE)
	{
		//ドライバーをオープンできなかった場合
        return FALSE;
    }
	return TRUE;
}


DWORD GetRefCount(HANDLE hHandle)
{
	DWORD dwRefCount;
	ULONG ReturnedLength, IoctlResult;


	dwRefCount = (DWORD)-1;

	if(hHandle)
	{
		IoctlResult = DeviceIoControl(
                            hHandle,
                            IOCTL_GET_REFCONT,
                            NULL,
                            0,
                            &dwRefCount,
                            sizeof(dwRefCount),
                            &ReturnedLength,
                            NULL
                            );
	}
	if(!IoctlResult) dwRefCount = (DWORD)-1;

	return dwRefCount;
}

void close()
{
	DWORD dwRefCount;
	
	dwRefCount = GetRefCount(handle);

	if(handle) CloseHandle(handle);
	if(drivertype & SYS_DRIVER_USE)
	{
		if(dwRefCount==1)
			UnloadDriver(DRIVERID);
	}
	handle = NULL;
}


/*
	NTドライバの登録とドライバーのOpen

	引数
	ghInst	このモジュールのhandle

	戻り値
	0		正常終了
	1		テンポラリファイルが作成できない
	2		ドライバーがloadできない
	3		その他のエラー
*/
int initialize(HMODULE ghInst)
{

	BOOL	ret;
	char tmp[MAX_PATH];
	char path[MAX_PATH];
	char *ptrEnd;

	//ドライバーをオープンしてみる
	if(Opendriver()) {
		status = DLLSTATUS_NOERROR;
		return 0;
	}
	//オープンできない場合はドライバーの登録、開始をする。

    // ドライバーの登録、開始
	//GetCurrentDirectory(MAX_PATH, tmp);
	GetModuleFileName(NULL,tmp, MAX_PATH);
	if ( (ptrEnd = strrchr(tmp, '\\')) != NULL ) {*ptrEnd = '\0';}
	sprintf(path,"%s\\%s",tmp,DRIVERFILENAME);
	ret = LoadDriver(path, DRIVERID);
	if(ret == FALSE){
		/*
		GetWindowsDirectory(tmp, MAX_PATH);
		strcat( tmp, "\\system32\\drivers\\"DRIVERFILENAME);
		ret = LoadDriver(tmp, DRIVERID);
		if(ret == FALSE){
			return 2;
		}
		*/
		return 2;
	}
	// ドライバーをオープンする
	if(Opendriver()) {
		status = DLLSTATUS_NOERROR;
		return 0;
	}
	// ドライバーをオープンできない場合
	return 3;
}


/*
	95/98ドライバの登録とドライバーのOpen

	引数
	ghInst	このモジュールのhandle

	戻り値
	0		正常終了
	1		テンポラリファイルが作成できない
	3		その他のエラー
*/
int initialize_vxd(HMODULE ghInst)
{
    handle = CreateFile("\\\\.\\" DRIVERFILENAME95, 0, 0, NULL, 0, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if(handle == INVALID_HANDLE_VALUE) return 2;

	return 0; // 0k
}

/*
DLLSTATUS getdllstatus(void)
戻り値
	DLLSTATUS_NOERROR				エラーなし。正常動作中
	DLLSTATUS_DRIVERNOTLOADED		ドライバーをロードできない
	DLLSTATUS_NOTSUPPORTEDPLATFORM	サポートされてない環境
	DLLSTATUS_OTHERERROR			その他エラー
機能
	DLLの状態を返す
*/

DLLSTATUS WINAPI getdllstatus(void)
{
	return status;
}


int GetFileInternalName(const char *file,char *name)
{
	VS_FIXEDFILEINFO vffi;
	ULONG reserved = 0;	
	UINT size;
	char *buf = NULL, *vbuf;
	char str[256];
	int  Locale = 0;

	size = GetFileVersionInfoSize((char*)file,&reserved);
	str[0] = '\0';
	vbuf = (char*)malloc(size);

	if (GetFileVersionInfo((char*)file, 0, size, vbuf))
	{
		VerQueryValue(vbuf,TEXT("\\"),(void**)&buf,&size);
		CopyMemory( &vffi, buf, sizeof(VS_FIXEDFILEINFO));

		VerQueryValue(vbuf, "\\VarFileInfo\\Translation", (void**)&buf, &size);
		CopyMemory(&Locale, buf, sizeof(int));
		wsprintf(str,
					"\\StringFileInfo\\%04X%04X\\%s", 
					LOWORD(Locale), HIWORD(Locale),
					"InternalName"
				);
		VerQueryValue(vbuf,str,(void**)&buf,&size);

		strcpy(str,buf);
		if(name != NULL){
			strcpy(name,buf);
		}
	}
	if(vbuf) free(vbuf);

	return 0;
}



