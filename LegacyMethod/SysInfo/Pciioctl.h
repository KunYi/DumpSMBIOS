////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#ifndef PCIIOCTL
#define PCIIOCTL

//
//	pcidebug.sys
//	ioctl関連定義ファイル
//


//エラーコード

#define PCI_ERR_BUSNOTEXIST	(0xE0000001L)  
#define PCI_ERR_NODEVICE	(0xE0000002L)
#define PCI_ERR_CONFWRITE	(0xE0000003L)
#define PCI_ERR_CONFREAD	(0xE0000004L)
#define PCI_ERR_EVENTCREATE	(0xE0000005L)


// Device type           -- in the "User Defined" range."

#define PCI_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.

#define IOCTL_PCI_READ_PORT_UCHAR \
    CTL_CODE( PCI_TYPE, 0x900, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_PCI_READ_PORT_USHORT \
    CTL_CODE( PCI_TYPE, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_PCI_READ_PORT_ULONG \
    CTL_CODE( PCI_TYPE, 0x902, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_PCI_WRITE_PORT_UCHAR \
    CTL_CODE(PCI_TYPE,  0x910, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_WRITE_PORT_USHORT \
    CTL_CODE(PCI_TYPE,  0x911, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_WRITE_PORT_ULONG \
    CTL_CODE(PCI_TYPE,  0x912, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_READ_MEM \
    CTL_CODE( PCI_TYPE, 0x920, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_PCI_WRITE_MEM \
    CTL_CODE(PCI_TYPE,  0x930, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_READ_CONF \
    CTL_CODE( PCI_TYPE, 0x940, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_PCI_WRITE_CONF \
    CTL_CODE(PCI_TYPE,  0x950, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_ENABLEISR \
    CTL_CODE(PCI_TYPE,  0x960, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_SETUPISR \
    CTL_CODE(PCI_TYPE,  0x961, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//#define IOCTL_PCI_MAXPCIBUSNO \
//    CTL_CODE(PCI_TYPE,  0x962, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_PCIBIOSSTATUS \
    CTL_CODE(PCI_TYPE,  0x963, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_WAIT_INTERRUPT \
    CTL_CODE(PCI_TYPE,  0x964, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_PCI_COPY_MEM \
    CTL_CODE(PCI_TYPE,  0x970, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PCI_FILL_MEM \
    CTL_CODE(PCI_TYPE,  0x980, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_READ_MSR \
	CTL_CODE(PCI_TYPE, 0x981, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_WRITE_MSR \
	CTL_CODE(PCI_TYPE, 0x982, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HLT \
	CTL_CODE(PCI_TYPE, 0x983, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_REFCONT \
	CTL_CODE(PCI_TYPE, 0x999, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct  _PCIDEBUG_PCIBIOSSTATUS {
    CHAR	returncode;
	CHAR	accessway;
	CHAR	version_measure;
	CHAR	version_minor;
	CHAR	maxbusnumber;
	CHAR	cf;
	CHAR	reserved[2];
	ULONG	sig;
}   PCIDEBUG_PCIBIOSSTATUS;

typedef struct  _PCIDEBUG_WRITE_INPUT {
    ULONG   PortNumber;     // Port # to write to
    union   {               // Data to be output to port
        ULONG   LongData;
        USHORT  ShortData;
        UCHAR   CharData;
    };
}   PCIDEBUG_WRITE_INPUT;

typedef struct  _PCIDEBUG_MEMREAD_INPUT {
    ULONG   address;     
    ULONG   unitsize;
	ULONG	count;
}   PCIDEBUG_MEMREAD_INPUT;

typedef struct  _PCIDEBUG_MEMWRITE_INPUT {
    ULONG   address;     
    ULONG   unitsize;
	ULONG	count;
    char	data[1];
}   PCIDEBUG_MEMWRITE_INPUT;

typedef struct  _PCIDEBUG_MEMCOPY_INPUT {
    ULONG   src_address;     
    ULONG   dest_address;
	ULONG	unitsize;
	ULONG	count;
}   PCIDEBUG_MEMCOPY_INPUT;

typedef struct  _PCIDEBUG_MEMFILL_INPUT {
    ULONG   address;     
    ULONG   data;
	ULONG	unitsize;
	ULONG	count;
}   PCIDEBUG_MEMFILL_INPUT;

typedef struct  _PCIDEBUG_CONFREAD_INPUT {
    ULONG   pci_address;     
    ULONG   pci_offset;     
}   PCIDEBUG_CONFREAD_INPUT;

typedef struct  _PCIDEBUG_CONFWRITE_INPUT {
    ULONG   pci_address;     
    ULONG   pci_offset;     
    char	data[1];
}   PCIDEBUG_CONFWRITE_INPUT;


#define EVENTNAMEMAXLEN	100

typedef struct  _PCIDEBUG_ISRINFO {
	INTERFACE_TYPE type;
	ULONG busnumber;
	ULONG ilevel;
	ULONG ivector;
	KINTERRUPT_MODE		InterruptMode;
	BOOLEAN				ShareVector;
	char reserved[3];
	int index;
	char eventname[EVENTNAMEMAXLEN];
}   PCIDEBUG_ISRINFO;

#endif
