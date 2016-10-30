#pragma once

class CMalwFManager
{
public:
	CMalwFManager(void);
	virtual ~CMalwFManager(void);
public:	
	int   GetShiftIndex( char cVol );
	ULONG GetVolumeTypeDrives( ULONG ulDrvType, char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetFdd_Drives  ( char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetUsb_Drives  ( char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetCdrom_Drives( char* pOutBuffer, int nMaxOutBuffer );	
	ULONG GetFixed_Drives( char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetNetwork_Drives( char* pOutBuffer, int nMaxOutBuffer );	
	void GetHomeDirectory( char* pczOutBuffer, int nMaxPath );	
	void SetInitParameter( char*  pDrvSymName,
						   char*  pDrvSvcName,
                           char*  pDrvFullPath,
					       ULONG  ulStartType,
					       ULONG  ulServiceType );
public:
	HANDLE m_hLogFetchEvent;
	static DWORD WINAPI ThreadProc_LogFetch( PVOID pParam );
public:	
	BOOL Driver_Install(void);   // 드라이버 설치
	BOOL Driver_UnInstall(void); // 드라이버 삭제	
	BOOL Driver_Start(void);     // 드라이버 구동
	BOOL Driver_Stop(void);      // 드라이버 내림

	//드라이버 버전
	BOOL Driver_GetVersion(void);
	// 로그 취합시작/정지 
	BOOL Driver_LogStart( ULONG ulLogStart );
	// 로그 Fetch
	BOOL Driver_GetLogFetch( char* pOutBuffer, ULONG  ulOutBufferSize );
	//전역플래그 설정
	BOOL Driver_SetGlobalCtrl(BOOL bGlobalCtrl);
	//설치폴더설정하기
	BOOL Driver_SetupDir(char* pSetupDir, char* pSysDir);


	//File 예외폴더 설정
	BOOL DriverSetExcept_FileDir(char* pczExceptDir);
	//File 예외폴더 해제
	BOOL DriverClrExcept_FileDir(void);
	//File 예외프로세스 설정 
	BOOL DriverSetExcept_FileProc(char* pczExceptProc);
	//File 예외프로세스 해제
	BOOL DriverClrExcept_FileProc(void);


	//File FDD 제어설정 
	BOOL DriverSetFDDVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
	//File 이동저장장치 제어설정
	BOOL DriverSetUSBVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
	// TRUE: 확장자제어 차단리스트 FALSE:확장자제어 차단 예외리스트
	//File 확장자 제어설정
	BOOL DriverSetFileExtCtrl( BOOL bFileExtDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczFileExt );
	//File 확장자 제어해제
	BOOL DriverClrFileExtCtrl(void);
	
	
	//공유폴더 제어설정
	BOOL  DriverSetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
	//공유폴더 제어해제
	BOOL  DriverClrSFolderCtrl(void);
	//공유폴더 예외폴더 설정
	BOOL  DriverSetExcept_SFDir(char* pczExceptDir);
	//공유폴더 예외폴더 해제
	BOOL  DriverClrExcept_SFDir(void);
	//공유폴더 예외프로세스 설정 
	BOOL  DriverSetExcept_SFProc( char* pczExceptProc );
	//공유폴더 예외프로세스 해제
	BOOL  DriverClrExcept_SFProc(void);

	

	//프로세스 살고 죽는것 가지고 오기
	BOOL  DriverGetProcStat(char* pczOutBuffer, int nMaxOutBuffer);
	//Process 예외폴더 설정
	BOOL  DriverSetExcept_ProcDir( char* pczExceptDir );
	//Process 예외폴더 해제
	BOOL  DriverClrExcept_ProcDir(void);
	//Process 제어 설정
	BOOL  DriverSetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl );
	//Process 제어 해제
	BOOL  DriverClrProcCtrl(void);

private:
	ULONG  m_ulDrvStartType;
	ULONG  m_ulDrvServiceType;
	char   m_czDrvSymName [MAX_PATH];
	char   m_czDrvSvcName [MAX_PATH];
	char   m_czDrvFullPath[MAX_PATH];
public:
	char* GetDrvSymName(void) { return m_czDrvSymName;  }
	char* GetDrvSvcName(void) { return m_czDrvSvcName;  }
	char* GetDrvFullPath(void){ return m_czDrvFullPath; }

};


extern CMalwFManager g_MalwFMgr;
