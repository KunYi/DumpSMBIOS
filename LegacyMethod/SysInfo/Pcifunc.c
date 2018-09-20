////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////


#include <windows.h>
#include <winioctl.h>
#include <stddef.h>
#include <assert.h>
#include "pcidebug.h"

ULONG WINAPI pciBiosStatus(PCIDEBUG_PCIBIOSSTATUS *buf)
{
	DWORD ReturnedLength;
	BOOL IoctlResult;
	IoctlResult = DeviceIoControl(
                            handle,
                            IOCTL_PCI_PCIBIOSSTATUS,
							NULL,
							0,
                            buf,
                            sizeof(PCIDEBUG_PCIBIOSSTATUS),
                            &ReturnedLength,
                            NULL
                            );

	if (!IoctlResult)                            // Did the IOCTL succeed?
	{
		return GetLastError();
	}

	assert((drivertype & VXD_DRIVER_USE) || ReturnedLength == sizeof(PCIDEBUG_PCIBIOSSTATUS));
        
	return 0;
}

//
//	コンフィグレーション空間の読み出し
//
//
//

ULONG WINAPI pciConfigRead(ULONG pci_address,ULONG reg_address, char *data , ULONG size)
{
	PCIDEBUG_CONFREAD_INPUT buf;
	DWORD ReturnedLength;
	BOOL IoctlResult;
	buf.pci_address = pci_address;
	buf.pci_offset = reg_address;
	IoctlResult = DeviceIoControl(
                            handle,
                            IOCTL_PCI_READ_CONF,
                            &buf,
                            sizeof(buf),
                            data,
                            size,
                            &ReturnedLength,
                            NULL
                            );

	if (!IoctlResult)                            // Did the IOCTL succeed?
	{
		return GetLastError();
	}

	assert((drivertype & VXD_DRIVER_USE) || ReturnedLength == size);
        
	return 0;
}

UCHAR	WINAPI _pciConfigReadChar(ULONG pci_address,ULONG reg_address)
{
	UCHAR ret;
	if(pciConfigRead(pci_address, reg_address, (char*)&ret , sizeof(ret) )) {
		// error
		return 0xffL;
	}
	return ret;
}

USHORT	WINAPI _pciConfigReadShort(ULONG pci_address,ULONG reg_address)
{
	USHORT ret;
	if(pciConfigRead(pci_address, reg_address, (char*)&ret , sizeof(ret) )) {
		// error
		return 0xffffL;
	}
	return ret;
}

ULONG	WINAPI _pciConfigReadLong(ULONG pci_address,ULONG reg_address)
{
	ULONG ret;
	if(pciConfigRead(pci_address, reg_address, (char*)&ret , sizeof(ret) )) {
		// error
		return 0xffffffffL;
	}
	return ret;
}

//
//	コンフィグレーション空間の書き込み
//
//
//

ULONG pciConfigWrite(ULONG pci_address,ULONG reg_address, char *data , int size)
{
	PCIDEBUG_CONFWRITE_INPUT	*w_input;
	DWORD ReturnedLength;
	int w_input_size;
	BOOL IoctlResult;
	w_input_size = offsetof(PCIDEBUG_CONFWRITE_INPUT, data) + size;
	w_input = (PCIDEBUG_CONFWRITE_INPUT*)malloc(w_input_size);
	if(w_input == NULL) return (-1);
	memcpy(w_input->data, data, size);
	w_input->pci_address = pci_address;
	w_input->pci_offset = reg_address;
	IoctlResult = DeviceIoControl(
                            handle,
                            IOCTL_PCI_WRITE_CONF,
                            w_input,
                            w_input_size,
                            NULL,
                            0,
                            &ReturnedLength,
                            NULL
                            );

	free(w_input);
	if (!IoctlResult)                            // Did the IOCTL succeed?
	{
		return GetLastError();
	}
	return 0;
}

void WINAPI _pciConfigWriteChar(ULONG pci_address,ULONG reg_address,UCHAR data)
{
	if(pciConfigWrite(pci_address, reg_address, (char*)&data , sizeof(data) )) {
		// error
		assert(0);
	}
	return ;
}

void WINAPI _pciConfigWriteShort(ULONG pci_address,ULONG reg_address,USHORT data)
{
	if(pciConfigWrite(pci_address, reg_address, (char*)&data , sizeof(data) )) {
		// error
		assert(0);
	}
	return;
}

void WINAPI _pciConfigWriteLong(ULONG pci_address,ULONG reg_address, ULONG data)
{
	if(pciConfigWrite(pci_address, reg_address, (char*)&data , sizeof(data) )) {
		// error
		assert(0);
	}
	return;
}


UCHAR	WINAPI _pciBusNumber(void)
{
	ULONG id;
	UCHAR bus;
	ULONG pci_address;
	ULONG	ret;

	if(drivertype & VXD_DRIVER_USE) { // for 95/98
		PCIDEBUG_PCIBIOSSTATUS buf;
		pciBiosStatus(&buf);
		if(buf.returncode == 0 || buf.cf ==0 && buf.sig == 0x20494350){
			return buf.maxbusnumber +1;
		} else {
			return 0;
		}
	} else {	// for NT
		for(bus = 0; bus < NumberOfBus; bus++){
			pci_address = pciBusDevFunc(bus, 0, 0);
			ret = pciConfigRead(pci_address, 0, (char*)&id , sizeof(id) );
			if(ret == PCI_ERR_BUSNOTEXIST) break;
		}
		return bus;
	}
}


ULONG	WINAPI _pciFindPciDevice(ULONG vender_id,ULONG device_id,ULONG index)
{
	ULONG bus, dev, func;
	ULONG count;
	ULONG pci_address;
	ULONG	id;
	ULONG	ret;	
	DWORD multifuncflag;
	BYTE type;
	count = 0;
	if(vender_id == 0xffff) return 0x83;

	for(bus = 0; bus < NumberOfBus; bus++){
		for(dev = 0; dev < NumberOfDevice; dev++){
			multifuncflag = 0;
			for(func = 0; func < NumberOfFunc; func++){
				if(multifuncflag == 0 && func > 0) break; 

				pci_address = pciBusDevFunc(bus,dev,func);
				ret = pciConfigRead(pci_address, 0, (char*)&id , sizeof(id) );
				if(ret == 0){
					if (func == 0) { // 多機能デバイスか調べる
						if(pciConfigRead(pci_address, 0x0e, (char*)&type, sizeof(type)) == 0) {
							if(type & 0x80) multifuncflag = 1;
						}
					}

					if (id == (vender_id | (device_id<<16))){
						if(count == index) return (pci_address<<16);
						count++;
						continue;
					}
				} else if(ret == PCI_ERR_BUSNOTEXIST) {
					return (0x86); 
				} else if(ret == PCI_ERR_NODEVICE && func == 0) break; 
			}
		}
	}
	return (0x86) ;
}

ULONG	WINAPI _pciFindPciClass(UCHAR baseclass, UCHAR subclass, UCHAR programif, ULONG index)
{
	ULONG bus, dev, func;
	ULONG count;
	ULONG pci_address;
	ULONG	conf[3];
	ULONG	ret;	
	DWORD multifuncflag;
	BYTE type;
	count = 0;

	for(bus = 0; bus < NumberOfBus; bus++){
		for(dev = 0; dev < NumberOfDevice; dev++){
			multifuncflag = 0;
			for(func = 0; func < NumberOfFunc; func++){
				if(multifuncflag == 0 && func > 0) break; 

				pci_address = pciBusDevFunc(bus,dev,func);
				ret = pciConfigRead(pci_address, 0, (char*)conf , sizeof(conf) );
				if(ret == 0){
					if (func == 0) { // 多機能デバイスか調べる
						if(pciConfigRead(pci_address, 0x0e, (char*)&type, sizeof(type)) == 0) {
							if(type & 0x80) multifuncflag = 1;
						}
					}
					if (( conf[2] & 0xffffff00L) == 
					   (((ULONG)baseclass<<24) |
						((ULONG)subclass<<16) |
						((ULONG)programif<<8))
						){
						if(count == index) return (pci_address<<16);
						count++;
						continue;
					}
				} else if(ret == PCI_ERR_BUSNOTEXIST) {
					return (0x86) ;
				} else if(ret == PCI_ERR_NODEVICE && func == 0) break; 
			}
		}
	}
	return (0x86) ;
}

