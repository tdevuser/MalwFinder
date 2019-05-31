#pragma once
#include "Configsvc.h"
#include "MalwFindErrcode.h"
#include "MalwFind_Comm.h"

namespace APP
{
	class CConfigSvcDriver :  public CConfigSvc
	{
	public:
		CConfigSvcDriver(void);
		virtual ~CConfigSvcDriver(void);
	public:
		virtual void   SetSvcStatus(DWORD dwState, DWORD dwAccept);
		virtual ULONG  RunService(void);
		virtual void   StopService(void);
		virtual BOOL   ExecuteServiceFunc(void);  // MainServer Call Func
	public:
		CString GetDriverDeviceName(void);
		void    SetDriverDeviceName(const TCHAR* pszDriverDeviceName, ULONG ulDriverDeviceNameLen );
	public:
		BOOL  Driver_Open(void);
		void  Dirver_Close(void);
		
		//드라이버 버전얻기
		FLT_STATUS __fastcall Driver_GetVersion();

		//로그 취합시작/중지
		// 0: 중지
		// 1: 시작
		FLT_STATUS __fastcall Driver_LogStart(ULONG ulLogStart);

		// 로그 패치 폴링방식
		FLT_STATUS __fastcall Driver_GetLogFetch( char* pBuffer, ULONG  ulOutBufferSize );

		
		// GlobalControl 제어플래그 세팅
		FLT_STATUS __fastcall Driver_SetGlobalCtrl(BOOL bGlobalCtrl);

		//설치폴더 정보설정
		FLT_STATUS __fastcall Driver_SetSetupDir( char* pczSetupDir, char* pHideFolder, char* pczSysDir );

		//File 예외폴더 설정
		FLT_STATUS __fastcall Driver_F_SetExceptDir( char* pczExceptDir );
		//File 예외폴더 설정
		FLT_STATUS __fastcall Driver_F_ClrExceptDir();
		//File 예외프로세스폴더 설정
		FLT_STATUS __fastcall Driver_F_SetExceptProc( char* pczExceptProc );
		//File 예외프로세스폴더 해제
		FLT_STATUS __fastcall Driver_F_ClrExceptProc();

		//File FDD 제어설정
		FLT_STATUS __fastcall Driver_F_SetFDDCtrl( NODE_POLICY* pPolicy );
		//File 이동저장장치 제어설정
		FLT_STATUS __fastcall Driver_F_SetUSBStorCtrl( NODE_POLICY* pPolicy );

		//File 확장자 제어설정
		FLT_STATUS __fastcall Driver_F_SetFileExtCtrl( BOOL bFileExtDeny, NODE_POLICY* pPolicy, char* pczFileExt );
		//File 확장자 제어해제 
		FLT_STATUS __fastcall Driver_F_ClrFileExtCtrl();

		//공유폴더 제어설정
		FLT_STATUS __fastcall Driver_SF_SetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
		//공유폴더 제어해제
		FLT_STATUS __fastcall Driver_SF_ClrSFolderCtrl();		

		//공유폴더 예외폴더 설정
		FLT_STATUS __fastcall Driver_SF_SetExceptDir( char* pczExceptDir );
		//공유폴더 예외폴더 해제
		FLT_STATUS __fastcall Driver_SF_ClrExceptDir();
		
		//공유폴더 예외프로세스폴더 설정
		FLT_STATUS __fastcall Driver_SF_SetExceptProc( char* pczExceptDir );
		//공유폴더 예외프로세스폴더 해제
		FLT_STATUS __fastcall Driver_SF_ClrExceptProc();

		//프로세스 콜백
		//프로세스 살고 죽는것 가지고 오기
		FLT_STATUS __fastcall Driver_P_GetProcessStat( char* pczOutBuffer, int nMaxOutBuffer );

		//프로세스 예외폴더 설정 
		FLT_STATUS __fastcall Driver_P_SetExceptDir( char* pczExceptDir );
		//프로세스 예외폴더 해제
		FLT_STATUS __fastcall Driver_P_ClrExceptDir();

		//프로세스 제어설정 
		FLT_STATUS __fastcall Driver_P_SetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl );
		//프로세스 제어해제
		FLT_STATUS __fastcall Driver_P_ClrProcCtrl();


	
		
		// Control Send
		FLT_STATUS __fastcall FltPolicySend( ULONG  ulControlCode, 
			                                 void*  pSendBuf,    
											 ULONG  ulSendLength, 
											 void*  pReceiveBuf, 
											 PULONG_PTR  pReceiveLength );

	protected:		
		HANDLE                 m_hFsDrv;
		SERVICE_STATUS_HANDLE  m_hService;
	private:
		TCHAR* m_pszDriverDeviceName;

	};


};


extern APP::CConfigSvcDriver  g_SvcDriver;
