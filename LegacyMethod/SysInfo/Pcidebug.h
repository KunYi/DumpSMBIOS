////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#include "pcifunc.h"
#include "pciioctl.h"

#define VXD_DRIVER_USE (1<<0)
#define SYS_DRIVER_USE (1<<1)
#define PCIBIOS_READY (1<<2)
#define LIB_IO_FUNC_USE (1<<3)

extern HANDLE handle;
extern drivertype;
