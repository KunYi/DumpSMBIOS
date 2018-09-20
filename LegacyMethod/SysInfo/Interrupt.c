////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#include <windows.h>
#include <winbase.h>
#include <winioctl.h>
#include <stddef.h>
#include <assert.h>
#include <process.h>
#include <stdio.h>

#include "pcidebug.h"
#define N_ISR	16
#define UNUSEDTABLE -1

static struct {
	int		level;
	HANDLE	isrthread;
	DWORD	stop_flag; // 割り込み監視スレットを終了させるフラグ
	void ( WINAPI *isr)(void);
} isrtable[N_ISR];

#define EVENTNAME	"ISREVENT"
#define STOPEVENTNAME "STOPISRTHREAD"


void enable_isr(int index)
{
	BOOL IoctlResult;
	DWORD ReturnedLength;
	assert(index < N_ISR && 0 <= index);
	assert(isrtable[index].level != UNUSEDTABLE);

	IoctlResult = DeviceIoControl(
                            handle,            
                            IOCTL_PCI_ENABLEISR,
                            &index,
                            sizeof(index),
                            NULL,
                            0,
                            &ReturnedLength,
                            NULL
                            );
}

//
//
// windows NT用　割り込み監視スレット
//
//

DWORD WINAPI isr_thread(LPVOID param)
{
	HANDLE event[2];
	void (WINAPI *isr)(void);
	int index;
	char buf[256];

	index = (int)param;
	assert(isrtable[index].level != UNUSEDTABLE);
	isr = isrtable[index].isr;
	sprintf(buf,"%s%d", STOPEVENTNAME, index);
	event[0] = CreateEvent(NULL, FALSE, FALSE, buf);
	if(event[0] == NULL) return 0;
	sprintf(buf,"%s%d", EVENTNAME, index);
	event[1] = OpenEvent(SYNCHRONIZE, FALSE, buf);
	if(event[1] == NULL) {
		CloseHandle(event[0]);
		return 0;
	}
	while(1){
		DWORD ret;
		enable_isr(index);
		ret = WaitForMultipleObjects(2, event, FALSE, INFINITE);
		if(ret == WAIT_FAILED) break; 
		if(ret == WAIT_OBJECT_0) break;
		isr();
	}
	CloseHandle(event[0]);
	CloseHandle(event[1]);
	return 0;
}

/*

  return 0 : time out
			1 : interrupt issu
			-1: ioctl error
*/

int wait_interrupt(int index)
{
	BOOL IoctlResult;
	DWORD ReturnedLength;
	DWORD ret;
	assert(index>=0 && index < N_ISR);
	assert(isrtable[index].level != UNUSEDTABLE);
	IoctlResult = DeviceIoControl(
                            handle,
                            IOCTL_WAIT_INTERRUPT,
                            &index,
                            sizeof(index),
                            &ret,
                            sizeof(ret),
                            &ReturnedLength,
                            NULL
                            );

	if (!IoctlResult)                            // Did the IOCTL succeed?
	{
		return -1;
	}
	return ret;
}

//
//
// windows 95/98用　割り込み監視スレット
//
//

DWORD WINAPI isr_thread_vxd(LPVOID param)
{
	void (WINAPI *isr)(void);
	int index;
	index = (int)param;
	assert(index>=0 && index < N_ISR);
	assert(isrtable[index].level != UNUSEDTABLE);
	isr = isrtable[index].isr;
	while(1){
		enable_isr(index);
		while(1){
			if(isrtable[index].stop_flag) return 0;
			if(wait_interrupt(index)) break;
		}
		isr();
	}
	return 0;
}

//
// 指定した割り込み番号に対応するisrtableのindex値をを探す
//　無い場合は-1を返す
//
int find_index(int level)
{
	int i;
	for(i=0;i<N_ISR;i++){
		if(isrtable[i].level == level) {
			return i;
		}
	}
	return -1;
}

void free_isrtable(int i)
{
	assert(i >=0 && i < N_ISR);
	isrtable[i].level = UNUSEDTABLE;
	isrtable[i].isr = NULL;
	isrtable[i].isrthread = NULL;
	isrtable[i].stop_flag = 0;
}

void init_isr()
{
	int i;
	for(i=0;i<N_ISR;i++){
		free_isrtable(i);
	}
}

int setup_isr(
	INTERFACE_TYPE type,
	ULONG busnumber,
	ULONG level,
	KINTERRUPT_MODE		InterruptMode,
	char *eventname,
	int index)
{
	BOOL IoctlResult;
	DWORD ReturnedLength;
	PCIDEBUG_ISRINFO isrinfo;

	isrinfo.type = type;
	isrinfo.busnumber = busnumber;
	isrinfo.ilevel = level;
	isrinfo.ivector = level;
	isrinfo.InterruptMode = InterruptMode;
	isrinfo.ShareVector = TRUE;
	isrinfo.index = index;
	strcpy(isrinfo.eventname, eventname);
	IoctlResult = DeviceIoControl(
                            handle,
                            IOCTL_PCI_SETUPISR,
                            &isrinfo,
                            sizeof(isrinfo),
                            NULL,
                            0,
                            &ReturnedLength,
                            NULL
                            );

	if (!IoctlResult)                            // Did the IOCTL succeed?
	{
		return 1;
	}
	return 0;
}

/*
	戻り値	0	正常終了
			1	エラー 
*/

int WINAPI _hookIRQ_NT(
					 	INTERFACE_TYPE type,
						ULONG busnumber,
						ULONG level,
						KINTERRUPT_MODE		InterruptMode,
						void (WINAPI *isr)(void) )
{
//	DWORD ThreadId; 
	HANDLE stop_event;
	int index;
	char buf[256];

	//割り込み監視スレッドが動作中なら、停止する
	index = find_index(level);
	if(index >= 0 && isrtable[index].isrthread){

		setup_isr(type, busnumber, level, InterruptMode, "", index); // unsetup

		// Windows 95/98の場合はstop_flagを１にすることでスレットを停止させる
		if(drivertype & VXD_DRIVER_USE) {
			isrtable[index].stop_flag = 1;
		} else {

		// Windows NTの場合はstop_eventをシグナル状態にすることでスレットを停止させる
			sprintf(buf,"%s%d", STOPEVENTNAME, index);
			stop_event = OpenEvent(EVENT_MODIFY_STATE, FALSE, buf);
			assert(stop_event != NULL);
			if(stop_event != NULL) {
				SetEvent( stop_event );
				CloseHandle( stop_event );
			}
		}
		while(1){ // スレットが停止するまで待つ
			DWORD ret;
			Sleep(10);
			if(GetExitCodeThread(isrtable[index].isrthread, &ret) == 0){
				// fail
				assert(0);
				break;
			}
			if(ret != STILL_ACTIVE) break;
		}
		free_isrtable(index);
	}

	if(isr == NULL) return 0; //割り込み解除のみ。
	index = find_index(UNUSEDTABLE);
	if(index <0) return 1; // 空いているテーブルが無い

	assert(index >= 0 && index < N_ISR);

	isrtable[index].level = level;
	isrtable[index].isr = isr;
	// 割り込みを登録する
	sprintf(buf,"%s%d", EVENTNAME, index);
	if(setup_isr(type, busnumber, level, InterruptMode, buf, index)){
		return 1;
	}

	if(drivertype & VXD_DRIVER_USE) { // 95/98の場合
		isrtable[index].stop_flag = 0;
//		isrtable[index].isrthread = (HANDLE)_beginthreadex(NULL, 0, isr_thread_vxd, (void*)index, 0, &ThreadId);
		assert(isrtable[index].isrthread != NULL);
		if(isrtable[index].isrthread == NULL) return 1;
		return 0;	
	}
//	isrtable[index].isrthread = (HANDLE)_beginthreadex(NULL, 0, isr_thread, (void*)index, 0, &ThreadId); 
	assert(isrtable[index].isrthread != NULL);
	if(isrtable[index].isrthread == NULL) return 1;

	// threadが開始されるまで待つ。
	sprintf(buf,"%s%d", STOPEVENTNAME, index);
	while(1){
		Sleep(10);
		stop_event = OpenEvent(EVENT_ALL_ACCESS , FALSE, buf);
		if(stop_event) break; 
	}
	CloseHandle( stop_event );
	return 0;
}

int WINAPI _hookIRQ(ULONG level, void (WINAPI *isr)(void))
{
	return _hookIRQ_NT(PCIBus, 0, level, LevelSensitive, isr);
}

int WINAPI _freeIRQ(ULONG level)
{
	return _hookIRQ_NT(PCIBus, 0, level, LevelSensitive, NULL);
}
