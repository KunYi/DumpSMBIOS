/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2002- hiyohiyo All rights reserved.
/*---------------------------------------------------------------------------*/

#ifndef __INST_DRV_H__
#define __INST_DRV_H__

//When you build CpuInfoAMD64.sys, please define _AMD64.
#define _AMD64

#define DRIVER_NAME			"CrystalCpuInfo"
#define DRIVER_NAME_FULL	"\\\\.\\"DRIVER_NAME

#ifdef _AMD64
#define DRIVER_FILE_NAME	"CpuInfoX64.sys"
#else
#define DRIVER_FILE_NAME	"CpuInfo.sys"
#endif

BOOL LoadDriver(char *FileName, char *DriverName);
BOOL UnloadDriver(char *DriverName);

#endif // __INST_DRV_H__
