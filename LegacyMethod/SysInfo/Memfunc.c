////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#include <windows.h>
#include <winioctl.h>
#include <stddef.h>
#include "pcidebug.h"

//extern "C" __declspec (dllexport) ULONG	_MemReadBlock(ULONG address, UCHAR* data, ULONG count, ULONG unitsize)
extern  __declspec (dllexport) ULONG	_MemReadBlock(ULONG address, UCHAR* data, ULONG count, ULONG unitsize)
{
	PCIDEBUG_MEMREAD_INPUT param;
	BOOL IoctlResult;
	ULONG	ReturnedLength;
	ULONG	size;
	param.address = address;
	param.unitsize = unitsize;
	param.count = count;
	size = param.unitsize * param.count;
    IoctlResult = DeviceIoControl(
                        handle,							// Handle to device
                        IOCTL_PCI_READ_MEM,				// IO Control code
                        &param,							// Buffer to driver
                        sizeof(PCIDEBUG_MEMREAD_INPUT),	// Length of buffer in bytes.
                        data,							// Buffer from driver.
                        size,							// Length of buffer in bytes.
                        &ReturnedLength,				// Bytes placed in outbuf.
                        NULL							// NULL means wait till I/O completes.
                        );

    if(IoctlResult && ReturnedLength == size){
		return param.count;
	} else {
		return 0;
	}
}

ULONG	WINAPI _MemReadBlockLong(ULONG address, UCHAR* data, ULONG count)
{
	return _MemReadBlock(address, data, count, 4);
}

ULONG	WINAPI _MemReadBlockShort(ULONG address, UCHAR* data, ULONG count)
{
	return _MemReadBlock(address, data, count, 2);
}

ULONG	WINAPI _MemReadBlockChar(ULONG address, UCHAR* data, ULONG count)
{
	return _MemReadBlock(address, data, count, 1);
}

ULONG	WINAPI _MemReadLong(ULONG address)
{
	ULONG data;
	_MemReadBlockLong(address, (unsigned char*)&data, 1);
	return data;
}

USHORT	WINAPI _MemReadShort(ULONG address)
{
	USHORT data;
	_MemReadBlockShort(address, (unsigned char*)&data, 1);
	return data;
}

UCHAR	WINAPI _MemReadChar(ULONG address)
{
	UCHAR data;
	_MemReadBlockChar(address, &data, 1);
	return data;
}

ULONG	_MemWriteBlock(ULONG address, UCHAR* data, ULONG count, ULONG unitsize)
{
	PCIDEBUG_MEMWRITE_INPUT *param;
	BOOL IoctlResult;
	ULONG	ReturnedLength;
	ULONG	size;
	size = offsetof(PCIDEBUG_MEMWRITE_INPUT,data) + count*unitsize;
	param = (PCIDEBUG_MEMWRITE_INPUT*)malloc(size);
	param->address = address;
	param->unitsize = unitsize;
	param->count = count;
	memcpy(&param->data, data, count*unitsize);
    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_WRITE_MEM,
                        param,     
                        size,
                        NULL,
                        0,
                        &ReturnedLength,
                        NULL
                        );
	free(param);

    if(IoctlResult){
		return param->count;
	} else {
		return 0;
	}
}

ULONG	WINAPI _MemWriteBlockLong(ULONG address, UCHAR* data, ULONG count)
{
	return _MemWriteBlock(address, data, count, 4);
}

ULONG	WINAPI _MemWriteBlockShort(ULONG address, UCHAR* data, ULONG count)
{
	return _MemWriteBlock(address, data, count, 2);
}

ULONG	WINAPI _MemWriteBlockChar(ULONG address, UCHAR* data, ULONG count)
{
	return _MemWriteBlock(address, data, count, 1);
}

void WINAPI _MemWriteLong(ULONG address, ULONG data)
{
	_MemWriteBlockLong(address, (unsigned char*)&data, 1);
}

void WINAPI _MemWriteShort(ULONG address, USHORT data)
{
	_MemWriteBlockShort(address, (unsigned char*)&data, 1);
}

void WINAPI _MemWriteChar(ULONG address, UCHAR data)
{
	_MemWriteBlockChar(address, &data, 1);
}

ULONG	_MemCopy(ULONG src_address, ULONG dest_address, ULONG count, ULONG unitsize)
{
	PCIDEBUG_MEMCOPY_INPUT param;
	BOOL IoctlResult;
	ULONG	ReturnedLength;
	param.src_address = src_address;
	param.dest_address = dest_address;
	param.unitsize = unitsize;
	param.count = count;
    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_COPY_MEM,
                        &param,     
                        sizeof(param),
                        NULL,
                        0,
                        &ReturnedLength,
                        NULL
                        );

    if(IoctlResult){
		return param.count;
	} else {
		return 0;
	}
}

// add copy and fill function  ver 1.1
ULONG WINAPI _MemCopyLong(ULONG src_address, ULONG dest_address, ULONG count)
{
	return _MemCopy(src_address, dest_address, count, 4);
}

ULONG WINAPI _MemCopyShort(ULONG src_address, ULONG dest_address, ULONG count)
{
	return _MemCopy(src_address, dest_address, count, 2);
}

ULONG WINAPI _MemCopyChar(ULONG src_address, ULONG dest_address, ULONG count)
{
	return _MemCopy(src_address, dest_address, count, 1);
}

ULONG	_MemFill(ULONG address, ULONG data, ULONG count, ULONG unitsize)
{
	PCIDEBUG_MEMFILL_INPUT param;
	BOOL IoctlResult;
	ULONG	ReturnedLength;
	param.address = address;
	param.data = data;
	param.unitsize = unitsize;
	param.count = count;
    IoctlResult = DeviceIoControl(
                        handle,
                        IOCTL_PCI_FILL_MEM,
                        &param,     
                        sizeof(param),
                        NULL,
                        0,
                        &ReturnedLength,
                        NULL
                        );

    if(IoctlResult){
		return param.count;
	} else {
		return 0;
	}
}

ULONG WINAPI _MemFillLong(ULONG  address, ULONG data, ULONG count)
{
	return _MemFill(address, data, count, 4);
}


ULONG WINAPI _MemFillShort(ULONG  address, USHORT data, ULONG count)
{
	return _MemFill(address, data, count, 2);
}

ULONG WINAPI _MemFillChar(ULONG  address, UCHAR data, ULONG count)
{
	return _MemFill(address, data, count, 1);
}
