////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#include <windows.h>
#include <winioctl.h>
#include <stddef.h>
#include <assert.h>
#include <conio.h>
#include "pcidebug.h"

//
// I/OãÛä‘ÇÃì«Ç›èoÇµ
//
//
//

ULONG	WINAPI _IoReadLong(ULONG address)
{
	ULONG	data = 0;
#ifndef _X86_64
	BOOL IoctlResult;
	ULONG	ReturnedLength;

	if(drivertype & LIB_IO_FUNC_USE) return _inpd((USHORT)address);

    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_READ_PORT_ULONG,
                        &address,     
                        sizeof(ULONG),
                        &data,
                        sizeof(data),
                        &ReturnedLength,
                        NULL
                        );
    assert(IoctlResult);
	assert(ReturnedLength == sizeof(data));
#endif
	return data;
}

USHORT	WINAPI _IoReadShort(ULONG address)
{
	USHORT	data = 0;
#ifndef _X86_64
	BOOL IoctlResult;
	ULONG	ReturnedLength;

	if(drivertype & LIB_IO_FUNC_USE) return _inpw((USHORT)address);

    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_READ_PORT_USHORT,
                        &address,     
                        sizeof(ULONG),
                        &data,
                        sizeof(data),
                        &ReturnedLength,
                        NULL
                        );
    assert(IoctlResult);
	assert(ReturnedLength == sizeof(data));
#endif
	return data;
}

UCHAR	WINAPI _IoReadChar(ULONG address)
{
	UCHAR	data=0;
#ifndef _X86_64
	BOOL IoctlResult;
	ULONG	ReturnedLength;

	if(drivertype & LIB_IO_FUNC_USE) return _inp((USHORT)address);
    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_READ_PORT_UCHAR,
                        &address,
                        sizeof(ULONG),
                        &data,
                        sizeof(data),
                        &ReturnedLength,
                        NULL
                        );
    assert(IoctlResult);
	assert(ReturnedLength == sizeof(data));
#endif
	return data;
}

//
// I/OãÛä‘ÇÃèëÇ´çûÇ›
//
//
//

void WINAPI _IoWriteLong(ULONG address, ULONG data)
{
#ifndef _X86_64
	BOOL IoctlResult;
	ULONG	ReturnedLength;
    ULONG   DataLength;
    PCIDEBUG_WRITE_INPUT InputBuffer;    // Input buffer for DeviceIoControl

	if(drivertype & LIB_IO_FUNC_USE) {
		_outpd((USHORT)address, data);
		return;
	}

	InputBuffer.LongData = data;
	InputBuffer.PortNumber = address;
    DataLength = offsetof(PCIDEBUG_WRITE_INPUT, CharData) +
                         sizeof(InputBuffer.LongData);

    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_WRITE_PORT_ULONG,
                        &InputBuffer,     
                        DataLength,
                        NULL,
                        0,
                        &ReturnedLength,
                        NULL
                        );

    assert(IoctlResult);
#endif
}

void WINAPI _IoWriteShort(ULONG address, USHORT data)
{
#ifndef _X86_64
	BOOL IoctlResult;
	ULONG	ReturnedLength;
    ULONG   DataLength;
    PCIDEBUG_WRITE_INPUT InputBuffer;    // Input buffer for DeviceIoControl

	if(drivertype & LIB_IO_FUNC_USE) {
		_outpw((USHORT)address, data);
		return;
	}

	InputBuffer.ShortData = data;
	InputBuffer.PortNumber = address;
    DataLength = offsetof(PCIDEBUG_WRITE_INPUT, CharData) +
                         sizeof(InputBuffer.ShortData);

    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_WRITE_PORT_USHORT,
                        &InputBuffer,     
                        DataLength,
                        NULL,
                        0,
                        &ReturnedLength,
                        NULL
                        );

    assert(IoctlResult);
#endif
}

void WINAPI _IoWriteChar(ULONG address, UCHAR data)
{
#ifndef _X86_64
	BOOL IoctlResult;
	ULONG	ReturnedLength;
    ULONG   DataLength;
    PCIDEBUG_WRITE_INPUT InputBuffer;    // Input buffer for DeviceIoControl

	if(drivertype & LIB_IO_FUNC_USE) {
		_outp((USHORT)address, data);
		return;
	}

	InputBuffer.CharData = data;
	InputBuffer.PortNumber = address;
    DataLength = offsetof(PCIDEBUG_WRITE_INPUT, CharData) +
                         sizeof(InputBuffer.CharData);

    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_WRITE_PORT_UCHAR,
                        &InputBuffer,     
                        DataLength,
                        NULL,
                        0,
                        &ReturnedLength,
                        NULL
                        );

    assert(IoctlResult);
#endif
}

