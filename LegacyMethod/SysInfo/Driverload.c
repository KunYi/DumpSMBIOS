////////////////////////////////////////////////////////
//  pcidebug.dll
//                        Aug 20 1999 kashiwano masahiro
//
////////////////////////////////////////////////////////

#include <windows.h>
/*
デバイスドライバドライバーの登録、開始をする
戻り値
	TRUE	正常終了
	FALSE	ドライバー登録、開始失敗
			デバイスドライバを制御できる権限がないと失敗する。

引数
	filename	ドライバーのファイル名
	drivername	ドライバーの名前。ドライバーを特定できる名前。
				UnloadDriverの引数にも使う

*/
BOOL LoadDriver(char *filename, char *drivername)
{
    SC_HANDLE	hSCManager;
    SC_HANDLE	hService;
    SERVICE_STATUS	serviceStatus;
    BOOL	ret;

	// サービスコントロールマネージャを開く
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager)
		return FALSE;

	// 既にドライバーが存在するか確認するためにドライバーを開く
	hService = OpenService( hSCManager,
							drivername,
							SERVICE_ALL_ACCESS);
	if(hService){
		// 既に動作している場合は停止させて削除する
		// 通常はドライバーが存在するときはLoadDriverを呼び出さないので普段はありえない
		// ドライバの異常が考えられる
		ret = ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);
		ret = DeleteService(hService);
		CloseServiceHandle(hService);
	}
	// ドライバーを登録する
	hService = CreateService(hSCManager,
		drivername,
		drivername,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER, // カーネルドライバ
		SERVICE_DEMAND_START,  // 後でStartService()によって開始する
		SERVICE_ERROR_NORMAL,
		filename,            // ドライバーファイルのパス
		NULL,NULL,NULL,NULL,NULL);

	ret = FALSE;
    if(hService) {

		// ドライバーを開始する
		ret = StartService(hService, 0, NULL);

		// ハンドルを閉じる
		CloseServiceHandle(hService);
	}
	// サービスコントロールマネージャを閉じる
	CloseServiceHandle(hSCManager);

	return ret;
}

/*
ドライバーを停止する
戻り値
	TRUE	正常終了
	FALSE	失敗

引数
	drivername	ドライバーの名前。
*/
BOOL UnloadDriver(char *drivername)
{
	SC_HANDLE	hSCManager;
	SC_HANDLE	hService;
	SERVICE_STATUS  serviceStatus;
	BOOL	ret;

	// サービスコントロールマネージャを開く
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager)
		return FALSE;

    // ドライバーのサービスを開く
    hService = OpenService(hSCManager, drivername, SERVICE_ALL_ACCESS);
	ret = FALSE;
	if(hService) {
	    // ドライバーを停止させる 
	    ret = ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);

	    // ドライバーの登録を消す
	    if(ret == TRUE)
	        ret = DeleteService(hService);

	    // ハンドルを閉じる
	    CloseServiceHandle(hService);
	}
	// サービスコントロールマネージャを閉じる
    CloseServiceHandle(hSCManager);

    return ret;
}
