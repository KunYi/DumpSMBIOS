# Microsoft Developer Studio Project File - Name="SysInfo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SysInfo - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "SysInfo.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "SysInfo.mak" CFG="SysInfo - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "SysInfo - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "SysInfo - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "SysInfo - Win32 Release64" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SysInfo - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "_NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Od /D "_WIN32" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /YX /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /i "version.lib wbemuuid.lib" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib advapi32.lib odbc32.lib odbccp32.lib wbemuuid.lib version.lib /nologo /dll /machine:I386 /out:"../CrystalExec/SysInfo.dll"
# ADD LINK32 user32.lib ole32.lib advapi32.lib oleaut32.lib odbc32.lib odbccp32.lib wbemuuid.lib version.lib /nologo /dll /machine:I386 /out:"..\CrystalExec\SysInfo.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "SysInfo - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /YX /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib advapi32.lib odbc32.lib odbccp32.lib wbemuuid.lib version.lib /nologo /dll /debug /machine:I386 /out:"../CrystalExec/SysInfo.dll" /pdbtype:sept
# ADD LINK32 user32.lib ole32.lib advapi32.lib oleaut32.lib odbc32.lib odbccp32.lib wbemuuid.lib version.lib /nologo /dll /debug /machine:I386 /out:"..\CrystalExec\SysInfo.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "SysInfo - Win32 Release64"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SysInfo___Win32_Release64"
# PROP BASE Intermediate_Dir "SysInfo___Win32_Release64"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release64"
# PROP Intermediate_Dir "Release64"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "_WIN32" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /YX /c
# ADD CPP /nologo /MD /W3 /O1 /D "_X86_64" /D "_WIN32" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /FR /GS- /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /i "version.lib wbemuuid.lib" /d "NDEBUG"
# ADD RSC /l 0x411 /i "version.lib wbemuuid.lib" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib ole32.lib advapi32.lib oleaut32.lib odbc32.lib odbccp32.lib wbemuuid.lib version.lib /nologo /dll /machine:I386 /out:"..\CrystalExec\SysInfo.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 cpuid.obj user32.lib ole32.lib advapi32.lib oleaut32.lib wbemuuid.lib version.lib /nologo /dll /machine:IX86 /out:"../CrystalExec/SysInfoX64.dll" /machine:AMD64
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "SysInfo - Win32 Release"
# Name "SysInfo - Win32 Debug"
# Name "SysInfo - Win32 Release64"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CpuInfo.cpp
DEP_CPP_CPUIN=\
	"..\common\ExecAndWait.h"\
	"..\common\IsNT.h"\
	".\CpuInfo.h"\
	".\CpuInfoID.h"\
	".\instdrv.h"\
	".\msrnt.h"\
	".\MultiplierTable.h"\
	
# End Source File
# Begin Source File

SOURCE=.\CpuModelNumber.cpp
DEP_CPP_CPUMO=\
	".\CpuInfo.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DmiInfo.cpp
DEP_CPP_DMIIN=\
	".\CpuInfoID.h"\
	".\DmiInfo.h"\
	".\ItemID.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\getdxver.cpp
DEP_CPP_GETDX=\
	".\dxdiag.h"\
	
# End Source File
# Begin Source File

SOURCE=.\msr9x.cpp
DEP_CPP_MSR9X=\
	".\msr9x.h"\
	

!IF  "$(CFG)" == "SysInfo - Win32 Release"

!ELSEIF  "$(CFG)" == "SysInfo - Win32 Debug"

!ELSEIF  "$(CFG)" == "SysInfo - Win32 Release64"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\msrnt.cpp
DEP_CPP_MSRNT=\
	".\msrnt.h"\
	
# End Source File
# Begin Source File

SOURCE=.\PciInfo.cpp
DEP_CPP_PCIIN=\
	".\CpuInfoID.h"\
	".\ItemID.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\PciInfo.h"\
	
# End Source File
# Begin Source File

SOURCE=.\SysInfo.cpp
DEP_CPP_SYSIN=\
	".\CpuInfo.h"\
	".\CpuInfoID.h"\
	".\DmiInfo.h"\
	".\ISysInfo.h"\
	".\ItemID.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\PciInfo.h"\
	".\Port32.h"\
	".\SysInfo.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CpuInfo.h
# End Source File
# Begin Source File

SOURCE=.\CpuInfoID.h
# End Source File
# Begin Source File

SOURCE=.\DmiInfo.h
# End Source File
# Begin Source File

SOURCE=.\ISysInfo.h
# End Source File
# Begin Source File

SOURCE=.\ItemID.h
# End Source File
# Begin Source File

SOURCE=.\msr9x.h
# End Source File
# Begin Source File

SOURCE=.\msrnt.h
# End Source File
# Begin Source File

SOURCE=.\MultiplierTable.h
# End Source File
# Begin Source File

SOURCE=.\PciInfo.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource1.h
# End Source File
# Begin Source File

SOURCE=.\SysInfo.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\SysInfo.rc
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\ExecAndWait.cpp
DEP_CPP_EXECA=\
	"..\common\ExecAndWait.h"\
	
# End Source File
# Begin Source File

SOURCE=..\common\ExecAndWait.h
# End Source File
# Begin Source File

SOURCE=..\common\IsNT.cpp
DEP_CPP_ISNT_=\
	"..\common\IsNT.h"\
	
# End Source File
# Begin Source File

SOURCE=..\common\IsNT.h
# End Source File
# End Group
# Begin Group "PciDebug"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\Driverload.c
# End Source File
# Begin Source File

SOURCE=.\Driverload.h
# End Source File
# Begin Source File

SOURCE=.\Interrupt.c
DEP_CPP_INTER=\
	".\Pcidebug.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\Pciioctl.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Iofunc.c
DEP_CPP_IOFUN=\
	".\Pcidebug.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\Pciioctl.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Memfunc.c
DEP_CPP_MEMFU=\
	".\Pcidebug.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\Pciioctl.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Pcidebug.h
# End Source File
# Begin Source File

SOURCE=.\Pcidef.h
# End Source File
# Begin Source File

SOURCE=.\Pcidll.c
DEP_CPP_PCIDL=\
	".\Driverload.h"\
	".\Pcidebug.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\Pciioctl.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Pcifunc.c
DEP_CPP_PCIFU=\
	".\Pcidebug.h"\
	".\Pcidef.h"\
	".\Pcifunc.h"\
	".\Pciioctl.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Pcifunc.h
# End Source File
# Begin Source File

SOURCE=.\Pciioctl.h
# End Source File
# Begin Source File

SOURCE=.\Port32.h
# End Source File
# End Group
# End Target
# End Project
