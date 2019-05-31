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
		
		//����̹� �������
		FLT_STATUS __fastcall Driver_GetVersion();

		//�α� ���ս���/����
		// 0: ����
		// 1: ����
		FLT_STATUS __fastcall Driver_LogStart(ULONG ulLogStart);

		// �α� ��ġ �������
		FLT_STATUS __fastcall Driver_GetLogFetch( char* pBuffer, ULONG  ulOutBufferSize );

		
		// GlobalControl �����÷��� ����
		FLT_STATUS __fastcall Driver_SetGlobalCtrl(BOOL bGlobalCtrl);

		//��ġ���� ��������
		FLT_STATUS __fastcall Driver_SetSetupDir( char* pczSetupDir, char* pHideFolder, char* pczSysDir );

		//File �������� ����
		FLT_STATUS __fastcall Driver_F_SetExceptDir( char* pczExceptDir );
		//File �������� ����
		FLT_STATUS __fastcall Driver_F_ClrExceptDir();
		//File �������μ������� ����
		FLT_STATUS __fastcall Driver_F_SetExceptProc( char* pczExceptProc );
		//File �������μ������� ����
		FLT_STATUS __fastcall Driver_F_ClrExceptProc();

		//File FDD �����
		FLT_STATUS __fastcall Driver_F_SetFDDCtrl( NODE_POLICY* pPolicy );
		//File �̵�������ġ �����
		FLT_STATUS __fastcall Driver_F_SetUSBStorCtrl( NODE_POLICY* pPolicy );

		//File Ȯ���� �����
		FLT_STATUS __fastcall Driver_F_SetFileExtCtrl( BOOL bFileExtDeny, NODE_POLICY* pPolicy, char* pczFileExt );
		//File Ȯ���� �������� 
		FLT_STATUS __fastcall Driver_F_ClrFileExtCtrl();

		//�������� �����
		FLT_STATUS __fastcall Driver_SF_SetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
		//�������� ��������
		FLT_STATUS __fastcall Driver_SF_ClrSFolderCtrl();		

		//�������� �������� ����
		FLT_STATUS __fastcall Driver_SF_SetExceptDir( char* pczExceptDir );
		//�������� �������� ����
		FLT_STATUS __fastcall Driver_SF_ClrExceptDir();
		
		//�������� �������μ������� ����
		FLT_STATUS __fastcall Driver_SF_SetExceptProc( char* pczExceptDir );
		//�������� �������μ������� ����
		FLT_STATUS __fastcall Driver_SF_ClrExceptProc();

		//���μ��� �ݹ�
		//���μ��� ��� �״°� ������ ����
		FLT_STATUS __fastcall Driver_P_GetProcessStat( char* pczOutBuffer, int nMaxOutBuffer );

		//���μ��� �������� ���� 
		FLT_STATUS __fastcall Driver_P_SetExceptDir( char* pczExceptDir );
		//���μ��� �������� ����
		FLT_STATUS __fastcall Driver_P_ClrExceptDir();

		//���μ��� ����� 
		FLT_STATUS __fastcall Driver_P_SetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl );
		//���μ��� ��������
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
