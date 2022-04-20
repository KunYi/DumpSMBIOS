DumpSMBIOS
==========

Dump SMBIOS for Windows XP and later,  

On Windows Vista and later it uses [GetSystemFirmwareTable() API](https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable), otherwise - WMI.

provide a access memory method to from F Segment to read table, only for study

Build Environments 
===
Visual Studio 2019 with vc142
